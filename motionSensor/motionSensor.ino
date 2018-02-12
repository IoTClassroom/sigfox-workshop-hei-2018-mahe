/*
 * Author: Louis Moreau: https://github.com/luisomoreau
 * Date: 2017/03/03
 * Update: 2018/02/12 by Matthieu Baralle
 * Description:
 * This arduino example will show you how to send a Sigfox message 
 * using the wisol module and the arduino UNO (https://yadom.fr/carte-breakout-sfm10r1.html)
*/

// include the SoftwareSerial library so you can use its functions:
#include <SoftwareSerial.h>

#define rxPin 10
#define txPin 11

int pirPin = 2; //input pin for PIR sensor
int pirState = LOW;
int valPIR = 0;



// set up a new serial port
SoftwareSerial Sigfox =  SoftwareSerial(rxPin, txPin);


//Set to 0 if you don't need to see the messages in the console
#define DEBUG 1 

//Message buffer
uint8_t msg[12];

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(pirPin, INPUT);
  if(DEBUG){
    Serial.begin(9600);
  }
  
  // open Wisol communication
  // define pin modes for tx, rx:
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  Sigfox.begin(9600);
  delay(100);
  getID();
  delay(100);
  getPAC();
}

// the loop function runs over and over again forever
void loop() {
  valPIR = digitalRead(pirPin);
  if (valPIR == HIGH){
    pirState = HIGH;
  } else {
    pirState = LOW;
  }
  int light = analogRead(A0);
  int humidity = analogRead(A1);
  Serial.print("light: ");
  Serial.println(light);
  Serial.print("Humidity: ");
  Serial.println(humidity);

  int lightPercentage = map(light, 0, 1023, 0, 100);
  int humidityPercentage = map(humidity, 0, 1023, 0, 100);
  Serial.print("light percentage: ");
  Serial.print(lightPercentage);
  Serial.println(" % ");
  Serial.print("Humidity percentage: ");
  Serial.print(humidityPercentage);
  Serial.println(" % ");
  if (pirState == HIGH){
    Serial.println("Detected motion");
  } else {
    Serial.println("No detected motion");
  }
    
  msg[0]=lightPercentage;
  msg[1]=humidityPercentage;
  if (pirState == HIGH){
    msg[2]=01;
  } else {
    msg[2]=00;
  }

  sendMessage(msg, 3);

  // In the ETSI zone, due to the reglementation, an object cannot emit more than 1% of the time hourly
  // So, 1 hour = 3600 sec
  // 1% of 3600 sec = 36 sec
  // A Sigfox message takes 6 seconds to emit 
  // 36 sec / 6 sec = 6 messages per hours -> 1 every 10 minutes
  delay(10*60*1000);
}

void blink(){
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);    
}

//Get Sigfox ID
String getID(){
  String id = "";
  char output;
  
  Sigfox.print("AT$I=10\r");
  while (!Sigfox.available()){
     blink();
  }
  
  while(Sigfox.available()){
    output = Sigfox.read();
    id += output;
    delay(10);
  }
  
  if(DEBUG){
    Serial.println("Sigfox Device ID: ");
    Serial.println(id);
  }
  
  return id;
}


//Get PAC number
String getPAC(){
  String pac = "";
  char output;
  
  Sigfox.print("AT$I=11\r");
  while (!Sigfox.available()){
     blink();
  }
  
  while(Sigfox.available()){
    output = Sigfox.read();
    pac += output;
    delay(10);
  }
  
  if(DEBUG){
    Serial.println("PAC number: ");
    Serial.println(pac);
  }
  
  return pac;
}


//Send Sigfox Message
void sendMessage(uint8_t msg[], int size){

  String status = "";
  char output;

  Sigfox.print("AT$SF=");
  for(int i= 0;i<size;i++){
    if (msg[i] < 16){
      Sigfox.write('0');
    }
    Sigfox.print(String(msg[i], HEX));
    if(DEBUG){
      if (msg[i] < 16){
        Serial.write('0');
      }
      Serial.print(String(msg[i], HEX));
    }
  }
  
  Sigfox.print("\r");

  while (!Sigfox.available()){
     blink();
  }
  while(Sigfox.available()){
    output = (char)Sigfox.read();
    status += output;
    delay(10);
  }
  if(DEBUG){
    Serial.println();
    Serial.print("Status \t");
    Serial.println(status);
  }
}

