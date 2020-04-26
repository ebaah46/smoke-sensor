
//#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
//#include <HTTPClient.h>
#include <SimpleTimer.h>
#include <ESP8266HTTPClient.h>


#define   mq2_pin A0 //Sensor pin

#define led D5
#define buzzer D0

#include "arduino_secrets.h" 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

char server[] = "http://smoke-sensor-api.herokuapp.com/api/data";    // name address for you server in the cloud (using DNS)
int port = 80;

// Time interval for all post messages
int INTERVAL = 3600000;
int threshold = 250;



SimpleTimer timer; // Create simple timer object

WiFiClient wifi;
int status = WL_IDLE_STATUS;

//Interval per readings definitions
int INTERVAL_READING = 50;
int INTERVAL_READING_TIME_SPACING = 500;

String contentType = "application/json";
int statusCode  = 0;
String response = " ";


HTTPClient client;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  pinMode(led,OUTPUT);

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    
    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Connected to wifi");
  client.begin(server);
  Serial.println("Connecting to server.......");
}

void loop() {
 
  //  Get mqSensor value
  int mqValue=0;
  // read mq sensor at intervals
  Serial.println("Taking sensor readings.");
  mqValue = mqIntervalReading();
  //  Push data to server if sensor records a dangerous value indicating smoke
  if(mqValue > threshold){
  digitalWrite(led,HIGH); // Light the LED indicating danger
  digitalWrite(buzzer,HIGH); // Sound the buzzer to draw attention
  postData(mqValue);  // Push data to server
  }
  else{
    Serial.println("No dangerous data recored\nProceeding");
   }
  delay(10000); // Wait for 10s before taking next reading
  // if the server's disconnected, stop the client:
  if (status != WL_CONNECTED) {
    Serial.println();
    Serial.println("Wireless Network Disconnected.");
    client.end();
   
  }
}

//Read MQ Values
int readMQ(){
  int analogSensor = analogRead(mq2_pin); // Read value value from sensor
  delay(100); // Wait for 0.1s 
  return analogSensor;
  }

//Read sensor values at different intervals
long mqIntervalReading(){
  long  value = 0;
  for(int i=0;i<INTERVAL_READING;i++){
      value += readMQ();
      delay(INTERVAL_READING_TIME_SPACING); 
    }
  value = value/INTERVAL_READING;
  return value;
}

//Send data to server
void postData(int mqValue){
  StaticJsonDocument<200> doc;
  char JsonMessageBuffer[200];
  doc["data"] = mqValue;
  serializeJsonPretty(doc, Serial);
  serializeJsonPretty(doc,JsonMessageBuffer);
  client.addHeader("Content-Type", "application/json",false,true);
//  client.addHeader("Content-Type", "application/x-www-form-urlencoded");
  statusCode = client.POST(JsonMessageBuffer);
  response = client.getString();
  while(statusCode != 201)
    {
        delay(2000);
        Serial.println("Data could not be posted.");
        Serial.println("StatusCode:");
        Serial.print(statusCode);
        Serial.println("Response received:");
        Serial.println(response);
        Serial.println("Reposting data again");
        statusCode = client.POST(JsonMessageBuffer);
            
    }
  Serial.println("Data successfully posted.");
  Serial.println("Response received:");
  Serial.println(response);
  client.end();
  
}
