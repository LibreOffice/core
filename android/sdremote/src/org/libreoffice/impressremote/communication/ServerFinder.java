package org.libreoffice.impressremote.communication;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;

public class ServerFinder {

	private static final int PORT = 1598;
	private static final String CHARSET = "UTF-8";

	private DatagramSocket mSocket = null;

	private Thread mListenerThread = null;

	public ServerFinder() {

	}

	private void listenForServer() {
		byte[] aBuffer = new byte[500];
		DatagramPacket aPacket = new DatagramPacket(aBuffer, aBuffer.length);

		try {
			String aCommand = null;
			String aAddress = null;
			System.out.println("SF:listening for packet\n");
			mSocket.receive(aPacket);
			System.out.println("SF:received packet\n");
			int i;
			for (i = 0; i < aBuffer.length; i++) {
				if (aPacket.getData()[i] == '\n') {
					aCommand = new String(aPacket.getData(), 0, i, CHARSET);
					break;
				}
			}
			if (i == aBuffer.length || !aCommand.equals("LOREMOTE_ADVERTISE")) {
				return;
			}

			for (int j = i + 1; j < aBuffer.length; j++) {
				if (aPacket.getData()[j] == '\n') {
					aAddress = new String(aPacket.getData(), i + 1, j, CHARSET);
				}
			}

			if (aAddress != null) {
				System.out.println("Address is :" + aAddress + "\n");
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}

	public void startFinding() {
		if (mSocket != null)
			return;

		if (mListenerThread == null) {
			mListenerThread = new Thread() {
				@Override
				public void run() {
					try {
						mSocket = new DatagramSocket();
						String aString = "LOREMOTE_SEARCH\n";
						DatagramPacket aPacket = new DatagramPacket(
						                aString.getBytes(CHARSET),
						                aString.length(),
						                InetAddress.getByName("239.0.0.1"),
						                PORT);
						mSocket.send(aPacket);
						System.out.println("SF:sent packet\n");
						while (true) {
							listenForServer();
						}
					} catch (SocketException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					} catch (UnsupportedEncodingException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					} catch (IOException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}

				}
			};
			mListenerThread.start();
		}

	}

	public void stopFinding() {
		if (mListenerThread != null) {

		}
	}
}
