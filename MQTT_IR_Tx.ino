#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <IRremoteESP8266.h>

IRsend irsend(4); //an IR led is connected to GPIO pin 4 (D2)

// Update these with values suitable for your network.

const char* ssid = "";
const char* password = "";
const char* mqtt_server = "10.0.0.200";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void sendIR(byte* payload, unsigned int length) {
  
  //1st byte IDs protocol
  int protocol = (char)payload[0] - '0';
  
  //2nd and 3rd byte IDs bit length
  int bitLength = (((char)payload[1] - '0')*10) + ((char)payload[2] - '0');

  //Everything after is the command to send
  unsigned long command = 0;
  char comstring[length - 3];
  
  for(int i = 3; i < length; i++){
    comstring[i-3] = (char)payload[i];
    Serial.print((char)payload[i]);
  }
 command = (unsigned long)strtol(comstring, NULL, 16);
 
  switch(protocol) {
    //NEC Case
    case 0: 
      irsend.sendNEC(command, bitLength);
    break;
    //Sony Case
    case 1: 
      irsend.sendSony(command, bitLength, 2);
    break;
    //Samsung Case
    case 2: 
      irsend.sendSAMSUNG(command, bitLength);
    break;
    //LG Case
    case 3: 
      irsend.sendLG(command, bitLength);
    break;

    default:
    break;
    
  }
  //Serial.println(command);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  sendIR(payload, length);

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      // Once connected, publish an announcement...
      client.publish("stat/IR_Remote", "ONLINE");
      // ... and resubscribe
      client.subscribe("cmnd/IR_Remote");
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  irsend.begin();
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 75, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("stat/IR_Remote", msg);
  }
}
