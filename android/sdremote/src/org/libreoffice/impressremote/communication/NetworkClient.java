package org.libreoffice.impressremote.communication;

import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;

import android.content.Context;
import android.content.ServiceConnection;

/**
 * Standard Network client. Connects to a server using Sockets.
 *
 * @author Andrzej J.R. Hunt
 *
 */
public class NetworkClient extends Client {

	private static final int PORT = 1599;

	public NetworkClient(String ipAddress) {

		Socket aSocket;
		try {
			aSocket = new Socket(ipAddress, PORT);
			mInputStream = aSocket.getInputStream();
			mOutputStream = aSocket.getOutputStream();
		} catch (UnknownHostException e) {
			// TODO Tell the user we have a problem
			e.printStackTrace();
		} catch (IOException e) {
			// TODO As above
			e.printStackTrace();
		}

	}

}
