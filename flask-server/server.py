from flask import Flask, render_template, request, jsonify
from Adafruit_IO import Client
import random  # For simulating sensor data when not connected to actual sensors

# Adafruit IO details
ADAFRUIT_IO_USERNAME = 'inusha'
ADAFRUIT_IO_KEY = ''

# Initialize Adafruit IO Client
aio = Client(ADAFRUIT_IO_USERNAME, ADAFRUIT_IO_KEY)

# Flask App
app = Flask(__name__)

# Simulated sensor data (replace with actual sensor reading logic)
def get_simulated_sensor_data():
    return {
        'humidity': round(random.uniform(30, 70), 2),
        'temperature': round(random.uniform(20, 35), 2),
        'soil_moisture': round(random.uniform(20, 80), 2),
        'ph_value': round(random.uniform(6.0, 8.0), 2)
    }


# Route to render the login page
@app.route('/')
def login():
    """Render the login page"""
    return render_template('login.html')

# Route to render the dashboard
@app.route('/dashboard')
def index():
    """Render the main dashboard page"""
    return render_template('index.html')

# Route to get sensor data
@app.route('/get_sensor_data', methods=['GET'])
def get_sensor_data():
    """Retrieve sensor data from Adafruit IO or simulate"""
    try:
        # Try to get real sensor data from Adafruit IO
        humidity = aio.receive('humidity').value
        temperature = aio.receive('temperature').value

        # Print the received data for inspection
        # print(f"Received humidity: {humidity}")
        # print(f"Received temperature: {temperature}")
        
        return jsonify({
            'humidity': humidity,
            'temperature': temperature,
            'soil_moisture': get_simulated_sensor_data()['soil_moisture'],
            'ph_value': get_simulated_sensor_data()['ph_value']
        })
    except Exception:
        # Fallback to simulated data if Adafruit IO fails
        # return jsonify(get_simulated_sensor_data())
        print("Error getting data from Adafruit IO")
        return "Error getting data from Adafruit IO"

# Route to toggle the actuator
@app.route('/toggle_actuator', methods=['POST'])
def toggle_actuator():
    """Toggle the actuator state"""
    try:
        # Get the state from the request
        state = request.form.get('state', 'off')
        
        # Convert state to Adafruit IO compatible value
        value = '1' if state == 'on' else '0'
        
        # Send to Adafruit IO
        aio.send('actuator', value)
        
        return jsonify({
            'status': 'success', 
            'message': f'Actuator turned {state}',
            'state': state
        })
    except Exception as e:
        print("Error toggling actuator")
        return jsonify({
            'status': 'error', 
            'message': str(e)
        }), 400

# Route to get the current actuator state
@app.route('/get_actuator_state', methods=['GET'])
def get_actuator_state():
    """Retrieve the current actuator state from Adafruit IO"""
    try:
        # Get the actuator state from Adafruit IO
        actuator_state = aio.receive('actuator').value
        
        # Convert to boolean for clarity
        state = 'on' if actuator_state == '1' else 'off'
        
        return jsonify({
            'actuator_state': state
        })
    except Exception as e:
        print("Error getting actuator state")
        return jsonify({
            'status': 'error', 
            'message': str(e)
        }), 400

if __name__ == '__main__':
    app.run(debug=True)
