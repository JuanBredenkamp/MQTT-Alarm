/*

        Project : Alarm
         Author : Juan Bredenkamp
        Created : 31 May 2019
    Description : Hassio MQTT Alarm Automation
        Version : v.2
MQTT Device ID  : alarm

  Arduino Pin                                  Board Pin

	Ground  ----------------------------------   Ground

		  D1  ----------------------------------   panic
		  D2  ----------------------------------   armdisarm
		  D5  ----------------------------------   externalled
      D6  ----------------------------------   alarmstatus
      D7  ----------------------------------   sirenstatus
  
  Pub
  
	     iotstatus		  ---- 	  Alarm IOT Online!
	    alarmstatus	    ---- 	  Alarm Status "alarmstatus off /on "
      sirenstatus	    ---- 	  Siren Status "sirenstatus off/ on"
	  alarmgoingoff     ----    Alarm going off "alarmgoingoff trigger"

  Sub
  
	     armdisarm		  ---- 	  Alarm Arm or Disarm "armdisarmtrigger"
	         panic		  ---- 	  Trigger Panic Alarm "panictrigger"
      externalled		  ---- 	  externalled On or Off "externalledtriggerhigh" or "externalledtriggerlow"

*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

char message_buff[100];

IPAddress ip(192, 168, 20, 61);
IPAddress gateway(192, 168, 20, 1);
IPAddress subnet(255, 255, 255, 0); 
IPAddress server(192, 168, 20, 60);

char ssid[] = "SSID"; // your SSID
char pass[] = "Password";  // your SSID Password

WiFiClient espClient;
PubSubClient mqttClient(espClient);

int panic = D1;
int armdisarm = D2;
int externalled = D5;
int alarmstatus = D6;
int sirenstatus = D7;

const long interval = 1000;
unsigned long previousMillis = 0;

const long systeminterval = 2000;
unsigned long systempreviousMillis = 0;

int ledState = LOW;

int alarmstatusState      = 0;     // current state of the button
int lastalarmstatusState  = 0;     // previous state of the button

int sirenstatusState      = 0;     // current state of the button
int lastsirenstatusState  = 0;     // previous state of the button

int counter = 0;

int builtinled = 2;

void setup() {
  
  Serial.begin(115200);
  pinMode(builtinled, OUTPUT);
  
  pinMode(externalled, OUTPUT);
  digitalWrite(externalled, LOW);
  
  pinMode(panic, OUTPUT);
  digitalWrite(panic, HIGH);
  
  pinMode(armdisarm, OUTPUT);
  digitalWrite(armdisarm, HIGH);
  
  pinMode(alarmstatus, INPUT);
  pinMode(sirenstatus, INPUT_PULLUP);
  
  WiFi.config(ip, gateway, subnet); 
  WiFi.begin(ssid, pass);
 
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
}

  Serial.println ( "" );
  Serial.print ( "Connected to " );
  Serial.println ( ssid );
  Serial.print ( "IP address: " );
  Serial.println ( WiFi.localIP() );
    
  delay(1500);
   
  mqttClient.setServer(server, 1883);
  mqttClient.setCallback(callback);
 
  delay(1500);
  
  alarmstatusState = digitalRead(alarmstatus);
  lastalarmstatusState = alarmstatusState;
  
  sirenstatusState = digitalRead(sirenstatus);
  lastsirenstatusState = sirenstatusState;
 
}

void loop() {
  
  if (!mqttClient.connected()) {
    reconnect();
  }
  
unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {

    previousMillis = currentMillis;


    if (ledState == LOW) {
       ledState = HIGH;
    }     
    else {
       ledState = LOW;
    }
    digitalWrite(builtinled, ledState);
  }

  mqttClient.loop();
    
  unsigned long systemcurrentMillis = millis();
   

  if (systemcurrentMillis - systempreviousMillis >= systeminterval) {
      systempreviousMillis = systemcurrentMillis;
      sensorcheck();

  }

}

void reconnect() {

  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("alarm", "Username", "Password")) {
      Serial.println("connected");
      
      mqttClient.publish("iotstatus","Alarm IOT Online!");
	  
      mqttClient.subscribe("armdisarm");
      mqttClient.subscribe("panic");
      mqttClient.subscribe("externalled");
	  
    }
    else {
      Serial.print("failed, rc=");
      Serial.println(" try again in 10 seconds");
      delay(10000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  int i = 0;

  for (i = 0; i < length; i++) {
    message_buff[i] = payload[i];
  }

  message_buff[i] = '\0';
  String msgString = String(message_buff);
  Serial.print(msgString);
  Serial.println();
  
  if (msgString == "panictrigger"){
	  digitalWrite(panic,LOW);
	  delay(500);
    digitalWrite(panic,HIGH);
    Serial.println("panictrigger");
  }
	
  if (msgString == "armdisarmtrigger"){
	  digitalWrite(armdisarm,LOW);
	  delay(500);
    digitalWrite(armdisarm,HIGH);
    Serial.println("armdisarmtrigger");
  }
	
  if (msgString == "externalledtriggerhigh"){
	  digitalWrite(externalled,HIGH);
    Serial.println("externalled on");
  }
	
  if (msgString == "externalledtriggerlow"){
	  digitalWrite(externalled,LOW);
    Serial.println("externalled off");
  }
 
}

void sensorcheck() {
	
int alarmstatusVal = digitalRead(alarmstatus);
int sirenstatusVal = digitalRead(sirenstatus);

  if (alarmstatusVal == HIGH) {
    mqttClient.publish("alarmstatus","on");
    Serial.println("alarmstatus on");
  }
  else {
    mqttClient.publish("alarmstatus","off");
    Serial.println("alarmstatus off");
  }
   
   delay(20);
      
  if (sirenstatusVal == HIGH) {
    mqttClient.publish("sirenstatus","off");
    Serial.println("sirenstatus off");
	  counter = 0;

  }
  else {
    mqttClient.publish("sirenstatus","on");
    Serial.println("sirenstatus on");
    alarmgoingofffunction();
    
}

}
	
void alarmgoingofffunction (){
  Serial.println("alarmgoing off");
	 counter++;
     Serial.println(counter);
	 if ( counter == 4 ){
		 mqttClient.publish("alarmgoingoff","trigger");
		 Serial.println("alarmgoingoff on");
	     counter = 0;
	 }
	 
 }
