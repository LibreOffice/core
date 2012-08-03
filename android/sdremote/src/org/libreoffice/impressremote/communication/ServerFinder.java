package org.libreoffice.impressremote.communication;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.util.Vector;

import org.libreoffice.impressremote.communication.CommunicationService.Server;

import android.content.Context;
import android.content.Intent;
import android.support.v4.content.LocalBroadcastManager;

public class ServerFinder {

	private Context mContext;

	private static final int PORT = 1598;
	private static final String CHARSET = "UTF-8";

	private DatagramSocket mSocket = null;

	private Thread mListenerThread = null;

	private boolean mFinishRequested = false;

	private Vector<Server> mServerList = new Vector<Server>();

	public ServerFinder(Context aContext) {
		mContext = aContext;
	}

	private void listenForServer() {
		byte[] aBuffer = new byte[500];
		DatagramPacket aPacket = new DatagramPacket(aBuffer, aBuffer.length);

		try {
			String aCommand = null;
			String aName = null;
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
			for (int j = i; j < aBuffer.length; j++) {
				if (aPacket.getData()[i] == '\n') {
					aName = new String(aPacket.getData(), i + 1, j, CHARSET);
					break;
				}
			}
			if (aName == null) {
				return;
			}
			Server aServer = new Server(CommunicationService.Protocol.NETWORK,
			                aPacket.getAddress().toString(), aName,
			                System.currentTimeMillis());
			mServerList.add(aServer);

			//			System.out.println("SF FOUND: IP="
			//			                + aPacket.getAddress().toString() + " HOSTNAME="
			//			                + aName);

			Intent aIntent = new Intent(
			                CommunicationService.MSG_SERVERLIST_CHANGED);
			LocalBroadcastManager.getInstance(mContext).sendBroadcast(aIntent);

		} catch (java.net.SocketTimeoutException e) {
			// Ignore -- we want to timeout to enable checking whether we
			// should stop listening periodically
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}

	public void startFinding() {
		if (mSocket != null)
			return;

		mFinishRequested = false;

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
						mSocket.setSoTimeout(1000 * 10);
						while (!mFinishRequested) {
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
			mFinishRequested = true;
			mListenerThread = null;
		}
	}

	public Server[] getServerList() {
		return mServerList.toArray(new Server[mServerList.size()]);
	}
}
