package org.libreoffice.impressremote.communication;

import org.json.JSONException;
import org.json.JSONObject;

/**
 * Interface to send commands to the server.
 *
 * @author Andrzej J.R. Hunt
 *
 */
public class Transmitter {

	private Client mClient;

	public Transmitter(Client aClient) {
		mClient = aClient;
	}

	public void nextTransition() {
		JSONObject aCommand = new JSONObject();
		try {
			aCommand.put("command", "transition_next");
		} catch (JSONException e) {
			e.printStackTrace();
			// TODO: clean
		}
		// Create JSON
		mClient.sendCommand(aCommand.toString());
	}

	public void previousTransition() {
		JSONObject aCommand = new JSONObject();
		try {
			aCommand.put("command", "transition_previous");
		} catch (JSONException e) {
			e.printStackTrace();
			// TODO: clean
		}
		// Create JSON
		mClient.sendCommand(aCommand.toString());
	}

	public void gotoSlide(int slide) {
		JSONObject aCommand = new JSONObject();
		try {
			aCommand.put("command", "goto_slide");
			aCommand.put("slide_numer", slide);
		} catch (JSONException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		mClient.sendCommand(aCommand.toString());
	}

}
