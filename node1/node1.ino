#include <easyMesh.h>
#include <SoftwareSerial.h>

#define   MESH_PREFIX     "Snapdanger"
#define   MESH_PASSWORD   "pumbicha"
#define   MESH_PORT       5555

#define VELMAX 115
#define ALCOOLMAX 70
#define OPCODE_BEBADO 0x40
#define OPCODE_VEL 0x20

easyMesh  mesh;
SoftwareSerial swSer(14, 12, false, 256);

int LED = D4;

//Setando valores para um bom condutor;
int sensorAlcool = 400;
int velocidade = 0;
int buttonState=0;
bool flag_alc = false;
bool flag_vel = false;
unsigned int startTime=0x7FFFFFFF;
void receivedCallback( uint32_t from, String &msg ) {

  // verifica se recebeu payload via wifi.
  // se recebeu, envia via  bluetooth o pacote recebido
  char btbuffer[13];

  msg.toCharArray(btbuffer, 13);
  btbuffer[12] = '\n';
  swSer.write((char*) btbuffer, 13);


}

void newConnectionCallback( bool adopt ) {

}

void send_payload(byte opcode, int flagSelect)
{
  swSer.write("ENCODE\n");


  if (flagSelect == 0)
    flag_alc = true;
  else
    flag_vel = true;

  while (swSer.available())
  {
    byte data_received[12];
    String package;
    for (int i = 0; i < 12; i++)
    {
      data_received[i] = swSer.read();
      if (i == 11)
      {

        if (flagSelect == 0)
          flag_alc = false;
        else
          flag_vel = false;
      }
    }

    //Opcode representando condutor bebado

    data_received[0] = data_received[0] | (byte) opcode;

    //broadcast data via wifi

    package = String((char*) data_received);
    if (millis()-startTime >= 600000) {
      Serial.println(package);
      startTime = millis();
    }
    mesh.sendBroadcast(package);

  }
}

void setup() {
  Serial.begin(115200);
  swSer.begin(9600); /* Define baud rate for serial communication */

  // pin to read mq3
  pinMode(A0, INPUT);

  // pin to read push button
  pinMode(4,INPUT);

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
//  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  mesh.setReceiveCallback( &receivedCallback );
  mesh.setNewConnectionCallback( &newConnectionCallback );
}


void loop() {
  mesh.update();
  delay(1000);

//  Serial.println(s);
//  if (Serial.available())
//  {
//    String data = Serial.readString();
//    String op = data.substring(0, 4);
//    String strValue = data.substring(4);
//    int value = strValue.toInt();
//    //    Serial.println(op);
//    //    Serial.println("data: " +/ data);
//    if (op.equals("VEL-"))
//    {
//      velocidade = value;
//      s = "velocidade";
//    }
//    if (op.equals("ALC-"))
//    {
//      sensorAlcool = value;
//      s = "";
//      s = strValue;
//      //      Serial.println(value);
//    }
//
//  }

   
  //Le sensor de alcool;
  sensorAlcool = analogRead(A0);
//  Serial.println(sensorAlcool);
  //Le sensor de velocidade
  buttonState = digitalRead(4);
  
  if (buttonState == HIGH) {
//    Serial.println("pressed");
    velocidade = 200;
  } else {
    velocidade = 0;
  }
  
  if (sensorAlcool > ALCOOLMAX || flag_alc)
    //individuo bebado, monta payload
    send_payload(OPCODE_BEBADO, 0);

  else if (velocidade > VELMAX || flag_vel)
    //individuo em alta velocidade, monta payload
    send_payload(OPCODE_VEL, 1);



}


