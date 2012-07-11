package org.libreoffice.impressremote.communication;

import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;

import android.os.StrictMode;

/**
 * Standard Network client. Connects to a server using Sockets.
 *
 * @author Andrzej J.R. Hunt
 */
public class NetworkClient extends Client {

	private static final int PORT = 1599;

	private Socket mSocket;

	public NetworkClient(String ipAddress) {
		StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder()
				.permitAll().build();
		StrictMode.setThreadPolicy(policy);
		System.out.println("Attempting to open port.");
		try {
			mSocket = new Socket(ipAddress, PORT);
			System.out.println("We seem to have opened.");
			mInputStream = mSocket.getInputStream();
			mOutputStream = mSocket.getOutputStream();
			startListening();
		} catch (UnknownHostException e) {
			// TODO Tell the user we have a problem
			e.printStackTrace();
		} catch (IOException e) {
			// TODO As above
			e.printStackTrace();
		}

	}

	@Override
	public void closeConnection() {
		try {
			if (mSocket != null)
				mSocket.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

}
