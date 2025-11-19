from flask import Flask, render_template, jsonify
import json
from paho.mqtt import client as mqtt_client

app = Flask(__name__)

# Store data
data_history = {
    'labels': [],
    'temperatures': [],
    'humidity': []
}

# MQTT Configuration
broker = 'public.mqtthq.com'
port = 1883
topic = "Data_dht"
client_id = "flask-mqtt-subscriber"

def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
            client.subscribe(topic)
        else:
            print("Failed to connect, return code:", rc)

    def on_message(client, userdata, msg):
        print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")

        try:
            data = json.loads(msg.payload.decode())
            temperature = float(data.get("temperature"))
            humidity = float(data.get("humidity"))

            index = len(data_history['temperatures'])
            data_history['labels'].append(index)
            data_history['temperatures'].append(temperature)
            data_history['humidity'].append(humidity)

        except Exception as e:
            print("Error parsing MQTT data:", e)

    client = mqtt_client.Client(client_id)
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(broker, port)
    return client


# Flask Routes
@app.route('/')
def index():
    return render_template("Frontend.html")


@app.route('/api/get-graph-data', methods=['GET'])
def get_graph_data():
    dht_data = {
        "labels": data_history['labels'],
        "temperatures": data_history['temperatures'],
        "humidity": data_history['humidity']
    }
    return jsonify(dht_data)


if __name__ == "__main__":
    mqtt_client_instance = connect_mqtt()
    mqtt_client_instance.loop_start()   # Start MQTT listener thread
    app.run(port=3000, debug=True)
