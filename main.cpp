/*
 * Main.cpp
 * Connected plant - 2019
 * by Lucas Consejo & Théo Ferreira
 * Ingesup B2B - Ynov Bordeaux
 */
#include "mbed.h"
#include "zest-radio-atzbrf233.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"

NetworkInterface *net;
MQTTNetwork mqttNetwork;
MQTT::Client<MQTTNetwork, Countdown> client;

static I2C i2c(I2C1_SDA, I2C1_SCL);
static AnalogIn analogic(ADC_IN1);

uint8_t lm75_adress = 0x48 << 1;
int arrivedcount = 0;
char buf[10];

char* topic_temp = "LucasYnovB2b/feeds/temperature";
char* topic_hum = "LucasYnovB2b/feeds/humidity";

namespace {
#define PERIOD_MS 500
}

// function that returns the temperature
float temperature(){
	char cmd[2];
	cmd[0]= 0x00;
	i2c.write(lm75_adress, cmd, 1);
	i2c.read(lm75_adress, cmd, 2);

	return ((cmd[0] << 8)| cmd[1] >> 7)*0.5;
}

// function that returns humidity
float humidity(){
	return analogic.read()*3.3*100.0/3.3;
}

/* Printf the message received and its configuration */
void messageArrived(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
    printf("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
}

// function that uses MQTT
int mqtt(){

	int result;

	nsapi_addr_t new_dns = {
		NSAPI_IPv6,
		{ 0xfd, 0x9f, 0x59, 0x0a, 0xb1, 0x58, 0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0x01 }
	};
	nsapi_dns_add_server(new_dns);

	// Get default Network interface (6LowPAN)
	net = NetworkInterface::get_default_instance();
	if (!net) {
		printf("Error! No network inteface found.\n");
		return 0;
	}

	// Connect 6LowPAN interface
	result = net->connect();
	if (result != 0) {
		printf("Error! net->connect() returned: %d\n", result);
		return result;
	}

	// Build the socket that will be used for MQTT
	mqttNetwork(net);

	// Declare a MQTT Client
	client(mqttNetwork);

	// Connect the socket to the MQTT Broker
	char* domaine = " io.adafruit.com";
	char* username = "LucasYnovB2b";
	char* key = "57a25d8a5ae84641999cc28fb7009ee2";
	char* hostname =  "mqtts://"+username+":"+key+"@"+domaine;
	uint16_t port = 1883;

	printf("Connecting to %s:%d\r\n", hostname, port);
	int rc = mqttNetwork.connect(hostname, port);
	if (rc != 0){
		printf("rc from TCP connect is %d\r\n", rc);
	}

	// Connect the MQTT Client
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.MQTTVersion = 3;
	data.clientID.cstring = "mbed-sample";
	data.username.cstring = username;
	data.password.cstring = key;
	if ((rc = client.connect(data)) != 0){
		printf("rc from MQTT connect is %d\r\n", rc);
	}

	if ((rc = client.subscribe(topic_temp, MQTT::QOS2, messageArrived)) != 0){
			printf("rc from MQTT subscribe is %d\r\n", rc);
	}

	MQTT::Message message;

	// QoS 0
	message.qos = MQTT::QOS0;
	message.retained = false;
	message.dup = false;
	message.payload = (void*)buf;
	message.payloadlen = strlen(buf)+1;

	return 1;
}

// main function
int main()
{
	mqtt();

	while(true){
		float temperature_value = temperature();
		printf("The temperature is %f \n", temperature_value);

		float humidity_value = humidity();
		printf("Humidity is %f \n\n", humidity_value);

		wait_ms(PERIOD_MS);
	}

	 // Disconnect client and socket
	client.disconnect();
	mqttNetwork.disconnect();

	// Bring down the 6LowPAN interface
	net->disconnect();
	printf("Done\n");

}
