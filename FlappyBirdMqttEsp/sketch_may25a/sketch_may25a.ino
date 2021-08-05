#include <WiFi.h>
#include <PubSubClient.h>

#include <MFRC522.h>
#include <SPI.h>

#define SS_PIN    21
#define RST_PIN   22

#define SIZE_BUFFER     18
#define MAX_SIZE_BLOCK  16

const char* ssid = "BETOS-LANCHES";
const char* password =  "87794625";
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* mqttUser = "";
const char* mqttPassword = "";

WiFiClient espClient;
PubSubClient client(espClient);

//Authentication
MFRC522::MIFARE_Key key;

//Authentication status code
MFRC522::StatusCode status;

//Pins
MFRC522 mfrc522(SS_PIN, RST_PIN);

String UID;

byte readTAG[4];

char mensagem[30];
int ledPin = 2;

int tiltPin = 26;
int tiltStatus;
boolean isTilted = false;

String message;

void setup()
{
    SPI.begin();
    mfrc522.PCD_Init();

    pinMode(ledPin, OUTPUT);
    Serial.begin(115200);
    pinMode(tiltPin, INPUT);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Iniciando conexao com a rede WiFi..");
    }

    Serial.println("Conectado na rede WiFi!");

    client.setCallback(callback);
}

void loop()
{
    //Conecta com o broker, caso não esteja conectado.
    if(!client.connected()){
        reconectabroker();
    }

    tiltStatus = digitalRead(tiltPin);
    
    if(tiltStatus == HIGH){
        if(!isTilted){
            isTilted = true;

            sprintf(mensagem, "Flap");
            Serial.println(mensagem);

            client.publish("FlappyBird/Inputs", mensagem);
        }
    }else if(tiltStatus == LOW){
        if(isTilted){
            isTilted = false;

            sprintf(mensagem, "Flap");
            Serial.println(mensagem);

            client.publish("FlappyBird/Inputs", mensagem);
        }
    }

    if(mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        readCard();

        message = "";

        if (*(uint32_t*)readTAG == 0x99CF53A9){
            sprintf(mensagem, "Vermelho%s", message);
            Serial.println(mensagem);
        }

        else if (*(uint32_t*)readTAG == 0xA3F0ECB9){
            sprintf(mensagem, "Amarelo%s", message);
            Serial.println(mensagem);
        }

        client.publish("FlappyBird/Inputs", mensagem);
    }
  //Para checar a rotina de callback
  client.loop();
}

void reconectabroker()
{
  //Conexao ao broker MQTT
  client.setServer(mqttServer, mqttPort);
  while (!client.connected())
  {
    Serial.println("Conectando ao broker MQTT...");
    if (client.connect("ESPGhost")) //Se tivesse um usuario e senha no broker aqui teria que ser client.connect("ESPdoJoel",mqttUser,mqttPassword)
    {
      Serial.println("Conectado ao broker!");
      client.subscribe("FlappyBird/Inputs");
    }
    else
    {
      Serial.print("Falha na conexao ao broker - Estado: ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Chegou mensagem no topico: ");
  Serial.print(topic);
  Serial.print(". Mensagem: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }

  Serial.println();
}

void readCard(){
    for (int i = 0; i < 4; i++) {
        readTAG[i] = mfrc522.uid.uidByte[i];    
    }  

    mfrc522.PICC_HaltA();
}