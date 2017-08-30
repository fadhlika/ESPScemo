#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

WiFiManager wifiManager;
WiFiClient client;

const char* proxy_server = "167.205.22.103";
const unsigned int proxy_port = 8080;
const char* server = "128.199.227.5";
const unsigned int port = 80;

bool measuring = false;
int measurePin =  5;

int send(String, String);
bool scanMeasure();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();

int scanPeriod = 2000;
long scanLast;

void setup(){
    Serial.begin(115200);
    wifiManager.autoConnect("ESPScemo", "12345678");

    Serial.println("connected to wifi");
    send("debug", "Booted");

    pinMode(measurePin, OUTPUT);
    digitalWrite(measurePin, LOW);
}

void loop(){
    if (Serial.available() > 0){
        String data = Serial.readStringUntil('\n');
        send("data", data);
    }
    if(measuring){
        send("debug", "measuring");
        measuring = false;
    }
    if(millis() > scanLast + scanPeriod){
        measuring = scanMeasure();
        Serial.print("measure: ");
        Serial.println(measuring);
        scanLast = millis();
    }
}

int send(String path, String message){
    Serial.print(path);
    Serial.print("\t");
    Serial.println(message);

    int status;        

    if (client.connect(proxy_server, proxy_port)) {
        
        String req = "POST http://" + String(server) + "/" + path + " HTTP/1.1\r\n";
        req += "Proxy-Authorization: Basic ZmFkaGxpa2E6NzgzNzEzMTE=\r\n";
        req += "Content-Length: " + String(message.length()) +"\r\n";
        req += "\r\n";
        req += message;

        client.println(req);
        client.println();
    }

    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 1000) {
            Serial.println("»> Client Timeout !");
            client.stop();
            return -1;
        }
    }
  
    if(client.available() > 0){
        String line = client.readStringUntil('\r');
        String st = line.substring(9, 12);
        status = st.toInt();
        Serial.println(status);
    }

    client.stop();
    return status;
}

bool scanMeasure(){
    Serial.println("Scan measure");

    bool m = false;

    if (client.connect(proxy_server, proxy_port)) {
        
        String req = "GET http://" + String(server) + "/measure HTTP/1.1\r\n";
        req += "Proxy-Authorization: Basic ZmFkaGxpa2E6NzgzNzEzMTE=\r\n";

        client.println(req);
        client.println();
    }

    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 1000) {
            Serial.println("»> Client Timeout !");
            client.stop();
            return false;
        }
    }
  
    // Read all the lines of the reply from server and print them to Serial
    int i = 0;
    while(client.available() > 0){
        i++;
        String line = client.readStringUntil('\r');
        if(i == 11) {
            Serial.println(line);
            if(line.toInt() == 1) m = true;
            else m = false;
        }
    }
    client.stop();
    return m;
}