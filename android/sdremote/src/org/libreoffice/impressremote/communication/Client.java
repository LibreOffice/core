package org.libreoffice.impressremote.communication;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;

import org.apache.http.util.ByteArrayBuffer;
import org.json.JSONException;
import org.json.JSONObject;

/**
 * Generic Client for the remote control. To implement a Client for a specific
 * transport medium you must provide input and output streams (
 * <code>mInputStream</code> and <code>mOutputStream</code> before calling any
 * methods.
 *
 * @author Andrzej J.R. Hunt
 */
public abstract class Client {

	private static final String CHARSET = "UTF-8";

	protected InputStream mInputStream;
	protected OutputStream mOutputStream;

	public abstract void closeConnection();

	private Receiver mReceiver;

	public void setReceiver(Receiver aReceiver) {
		mReceiver = aReceiver;
	}

	protected void startListening() {

		Thread t = new Thread() {
			public void run() {
				listen();
			};

		};
		t.start();
	}

	private void listen() {
		BufferedReader aReader;
		try {
			aReader = new BufferedReader(new InputStreamReader(mInputStream,
					CHARSET));
			while (true) {
				ArrayList<String> aList = new ArrayList<String>();
				String aTemp;
				// read until empty line
				while ((aTemp = aReader.readLine()).length() != 0) {
					aList.add(aTemp);
				}
				mReceiver.parseCommand(aList);
			}
		} catch (UnsupportedEncodingException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e1) {
			// TODO stream couldn't be opened.
			e1.printStackTrace();
		}

	}

	/**
	 * Send a valid JSON string to the server.
	 *
	 * @param command
	 *            Must be a valid JSON string.
	 */
	public void sendCommand(String command) {
		try {
			mOutputStream.write(command.getBytes(CHARSET));
		} catch (UnsupportedEncodingException e) {
			throw new Error("Specified network encoding [" + CHARSET
					+ " not available.");
		} catch (IOException e) {
			// TODO Notify that stream has closed.
		}
	}

}
