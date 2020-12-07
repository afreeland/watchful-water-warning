#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h> 
#include <WiFiClientSecureBearSSL.h>
#include <secret.h>

#define TOP_SENSOR 3

const char* ssid = WIFI_SSID; // write your SSID
const char* password = WIFI_PASS; // write your Password

void setup() {
  delay(10);

  pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED_BUILTIN pin as an output
  pinMode(TOP_SENSOR, INPUT); // Initialize the top float sensor as input
  
  WiFi.hostname("watchful-water-warning"); // An easily identifiable hostname for router identification
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
}

void blinkLED(int count, int delayTime = 200){
  for (int i = 0; i < count; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(delayTime);
    digitalWrite(LED_BUILTIN, LOW);
    delay(delayTime);
  }
  
}

// I dont necessarily care which float sensor hits, essential one is backup to prevent...backup.
void sendAlert(){
  if ((WiFi.status() == WL_CONNECTED)) {
    const char* host = AWS_URL;
    WiFiClientSecure client;
    client.setInsecure(); //the magic line, use with caution
    client.connect(host, 443);  
    HTTPClient https; 
    if (https.begin(client, host)) {
      https.addHeader("X-API-Key", AWS_API_KEY); // API Gateway requires an API Key
      https.GET(); // We can achieve what we want with a simple GET request
      delay(500); // Give it some space
      https.end();  // Free resources
    }
  }
}
// TODO debounce float input and identity an a way on 8266-01 to use pull up resistor without triggering a non-desirable board mode >.>
void loop() {
    // By default, we really dont want to spam our inbox.  Could setup a webserver for status or send messages for heartbeat.
    // However, I believe I will just have a physical light indicator as opposed to digital noise
    bool shouldSendAlert = false;
    int val = digitalRead(TOP_SENSOR);
    
    if(val == LOW){
      // Uh oh, if this code is being ran than it is because a float sensor is registering high water levels
      shouldSendAlert = true;
    }

    // OH NOOOO, something isn't write water levels are higher than we like =/
    // Lets send a message to our cloud function so we can get notified
    if(shouldSendAlert){
      digitalWrite(LED_BUILTIN, HIGH);
      // blinkLED(10, 40);
      sendAlert();
      // Since our application is so simple, we are fine with delaying every 5 seconds
      delay(5000);
    }else{
       digitalWrite(LED_BUILTIN, LOW);
    }

    
}