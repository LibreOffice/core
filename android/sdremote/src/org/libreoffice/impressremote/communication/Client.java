package org.libreoffice.impressremote.communication;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;

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
 *
 */
public abstract class Client {

	private static final String CHARSET = "UTF-16";

	protected InputStream mInputStream;
	protected OutputStream mOutputStream;

	private void listen() {
		ByteArrayBuffer aBuffer = new ByteArrayBuffer(10);
		byte aTemp;

		try {
			while ((aTemp = (byte) mInputStream.read()) != 0x0d) {
				aBuffer.append(aTemp);
			}
		} catch (IOException e1) {
			// TODO stream couldn't be opened.
			e1.printStackTrace();
		}

		String aLengthString;
		try {
			aLengthString = new String(aBuffer.toByteArray(), CHARSET);
		} catch (UnsupportedEncodingException e1) {
			throw new Error("Specified network encoding [" + CHARSET
					+ " not available.");
		}

		int aLength = Integer.parseInt(aLengthString);

		byte[] aCommand = new byte[aLength];
		try {
			mInputStream.read(aCommand, 0, aLength);
		} catch (IOException e) {
			// TODO close and notify that the connection has closed
			e.printStackTrace();
		}
		String aCommandString;
		try {
			aCommandString = new String(aCommand, CHARSET);
		} catch (UnsupportedEncodingException e) {
			throw new Error("Specified network encoding [" + CHARSET
					+ " not available.");
		}
		parseCommand(aCommandString);

	}

	private void parseCommand(String aCommand) {
		JSONObject aCommandObject;
		String aInstruction;
		try {
			aCommandObject = new JSONObject(aCommand);
			aInstruction = aCommandObject.getString("command");
			if (aInstruction.equals("slide_changed")) {
				// TODO: process and notify
			}
		} catch (JSONException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}

	/**
	 * Send a valid JSON string to the server.
	 *
	 * @param command
	 *            Must be a valid JSON string.
	 */
	public void sendCommand(String command) {
		String aLengthString = Integer.toString(command.length());
		byte[] aLengthBytes;
		try {
			aLengthBytes = aLengthString.getBytes(CHARSET);

			mOutputStream.write(aLengthBytes);
			mOutputStream.write(0x0d);
			mOutputStream.write(command.getBytes(CHARSET));
		} catch (UnsupportedEncodingException e) {
			throw new Error("Specified network encoding [" + CHARSET
					+ " not available.");
		} catch (IOException e) {
			// TODO Notify that stream has closed.
		}
	}

}
