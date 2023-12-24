var mqtt = require('mqtt');
var express = require('express');
const axios = require('axios');


const MQTT_SERVER = "192.168.1.14";
const MQTT_PORT = "1883";
const MQTT_USER = "";
const MQTT_PASSWORD = "";

const app = express();
const port = 3000;

app.use(express.json());


var client = mqtt.connect({
    host: MQTT_SERVER,
    port: MQTT_PORT,
    username: MQTT_USER,
    password: MQTT_PASSWORD
});

client.on('connect', function () {
    console.log("MQTT Connect");
    client.subscribe('HR', function (err) {
        if (err) {
            console.log(err);
        }
    });
});

let previousMessage = 0;

client.on('message', function (topic, message) {
    console.log('Received message:', message.toString());
    const bpmAvg = parseInt(message.toString());

    if (previousMessage !== null && message.toString() !== previousMessage) {
        const sensorData = { "bpmAvg": bpmAvg };

        axios.post("http://192.168.1.14:4000/bpm", sensorData)
            .then(response => {
                console.log('Data sent successfully to the server:', response.data);
            })
            .catch(error => {
                console.error('Error sending data to the server:', error.message);
            });
    }

    previousMessage = message.toString();
});

app.get('/on', (req, res) => {
    const data = "on"

    client.publish("On/Off", data.toString());
    res.json({ message: 'Command received successfully.' });
})

app.get('/off', (req, res) => {
    const data = "off"

    client.publish("On/Off", data.toString());
    res.json({ message: 'Command received successfully.' });
})

app.listen(port, () => {
    console.log(`Server listening on  https://localhost:${port}`);
});