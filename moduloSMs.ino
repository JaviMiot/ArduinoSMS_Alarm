#include <SoftwareSerial.h>
#include <arduino-timer.h>
auto timer = timer_create_default();

#define sim800Tx 8
#define sim800Rx 7

#define power_key 9
#define input_signal 11

#define relay1  12
#define relay2  13

SoftwareSerial SIM800(sim800Rx, sim800Tx); //Seleccionamos los pines 7 como Rx y 8 como Tx

String phonesNumbers[] = {"+593983942677"} ;
int lentPhones = 1;
int countPhone = 0;
int inputValue = 0;
String comando = "";
boolean enviarMensaje = false;

//constantes de funcionamiento
const String ALARMA_MSG = "Alarma activada";
const String ENCENDER = "Encender";
const String APAGAR = "Apagar";

void setup()
{
 SIM800.begin(19200);
 Serial.begin(19200);
 configInputOutput();
 activePowerSim();
 delay(1000);
 initSim800();
 getSMS();
   //*configuro timer
  timer.every(50000,function_to_call );
}


void loop()
{
  timer.tick();
  inputValue = digitalRead(input_signal);
  delay(500);
  Serial.println(debugMsg("principal"));
  delay(40);
  setRelay();
}

bool function_to_call() {
  Serial.println(debugMsg("entra a verificar"));

  if(countPhone>lentPhones){
    Serial.println(debugMsg("reste count"));
    enviarMensaje = !enviarMensaje;
    countPhone = 0;
  }

  if(inputValue == HIGH && !enviarMensaje){
    call(phonesNumbers[countPhone]);
    countPhone = countPhone + 1;
  }

  if(inputValue == HIGH && enviarMensaje){
    sendSMS(phonesNumbers[countPhone]);
    countPhone = countPhone + 1;
  }

  return true;
}


void configInputOutput(){
  pinMode(power_key, OUTPUT);   
  pinMode(relay1, OUTPUT);   
  pinMode(relay2, OUTPUT);   
  pinMode(input_signal, INPUT_PULLUP); 
  digitalWrite(relay1, HIGH);
}

void activePowerSim(){
  digitalWrite(power_key,HIGH);
  delay(1000);
  digitalWrite(power_key,LOW);
  delay(15000);
}

void initSim800(){
  Serial.println("Initializing...");
  delay(1000);
  SIM800.println("AT"); //Once the handshake test is successful, it will back to OK
  updateSerial();
  SIM800.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
  updateSerial();
  SIM800.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
  updateSerial();
  SIM800.println("AT+CREG?"); //Check whether it has registered in the network
  updateSerial();
}

String debugMsg(String msg){
  return "debug --> " + msg; 
}

void sendSMS(String number){
  Serial.println(debugMsg("enviando sms al numero: " + number));
  SIM800.println("AT");
  updateSerial();
  SIM800.println("AT+CMGF=1");
  updateSerial();
  SIM800.println("AT+CMGS=\""+number+"\"");
  updateSerial();
  SIM800.print(ALARMA_MSG +"\r\n");
  updateSerial();
  SIM800.write(0x1A);  
  delay(1000);
}

void getSMS(){
  SIM800.println("AT");
  updateSerial();                               
  SIM800.println("AT+CMGF=1"); 
  updateSerial();   
  SIM800.println("AT+CNMI=1,2,0,0,0");  
  updateSerial();
}

void call(String number){
  Serial.println(debugMsg("llamando al numero: " + number));
  SIM800.print("ATD"+number+";\r");
  delay(10000);
  SIM800.print("ATH");
  Serial.println(debugMsg("finaliza llamada "));
}

void updateSerial(){
  delay(500);
  while (Serial.available()) 
  {
    Serial.write(Serial.read());
    SIM800.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(SIM800.available()) 
  {
    Serial.write(SIM800.read());//Forward what Software Serial received to Serial Port
  }
}

void setRelay(){
  while(SIM800.available()) 
  { 
    char recieved = SIM800.read();
    comando += recieved;

    if (recieved == '\n'){
      if(comando.indexOf(APAGAR) != -1){
        Serial.println(debugMsg("APAGAR"));
        digitalWrite(relay1, HIGH);
      }

      if(comando.indexOf(ENCENDER) != -1){
        Serial.println(debugMsg("ENCENDER"));
        digitalWrite(relay1, LOW);
      }
      
      comando = "";
    }
  }
}
