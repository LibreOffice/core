/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.os.Messenger;

public class CommunicationService extends Service {

	/**
	 * Return the service to clients.
	 */
	public class CBinder extends Binder {
		public CommunicationService getService() {
			return CommunicationService.this;
		}
	}

	private final IBinder mBinder = new CBinder();

	public enum Protocol {
		NETWORK, BLUETOOTH
	};

	public static final int MSG_SLIDESHOW_STARTED = 1;
	public static final int MSG_SLIDE_CHANGED = 2;
	public static final int MSG_SLIDE_PREVIEW = 3;
	public static final int MSG_SLIDE_NOTES = 4;

	public static final String MSG_SERVERLIST_CHANGED = "SERVERLIST_CHANGED";

	private Transmitter mTransmitter;

	private Client mClient;

	private Receiver mReceiver = new Receiver();

	private ServerFinder mFinder = new ServerFinder(this);

	public void setActivityMessenger(Messenger aActivityMessenger) {
		mReceiver.setActivityMessenger(aActivityMessenger);
	}

	@Override
	public IBinder onBind(Intent intent) {
		// TODO Auto-generated method stub
		return mBinder;
	}

	@Override
	public void onCreate() {
		// TODO Create a notification (if configured).
	}

	@Override
	public void onDestroy() {
		// TODO Destroy the notification (as necessary).
	}

	public Transmitter getTransmitter() {
		return mTransmitter;
	}

	public Server[] getServers() {
		return mFinder.getServerList();
	}

	public void startFindingServers() {
		mFinder.startFinding();
	}

	public void stopFindingServers() {
		mFinder.stopFinding();
	}

	/**
	 * Connect to a specific server. This method cannot be called on the main
	 * activity thread.
	 *
	 * @param aServer
	 *            The Server to connect to.
	 */
	public void connectTo(Server aServer) {
		connectTo(aServer.getProtocol(), aServer.getAddress());
	}

	/**
	 * Connect to a specific server. This method cannot be called on the main
	 * activity thread.
	 *
	 * @param aProtocol
	 * @param address
	 */
	public void connectTo(Protocol aProtocol, String address) {
		switch (aProtocol) {
		case NETWORK:
			mClient = new NetworkClient(address);
			mTransmitter = new Transmitter(mClient);
			mClient.setReceiver(mReceiver);
			break;
		case BLUETOOTH:
			break;
		default:
			break;

		}

	}

	public void disconnect() {
		mClient.closeConnection();
	}

	public SlideShow getSlideShow() {
		return mReceiver.getSlideShow();
	}

	// ---------------------------------------------------- SERVER -------------
	/**
	 * Class describing a remote server.
	 */
	public static class Server {
		private Protocol mProtocol;
		private String mAddress;
		private String mName;
		private long mTimeDiscovered;

		protected Server(Protocol aProtocol, String aAddress, String aName,
		                long aTimeDiscovered) {
			mProtocol = aProtocol;
			mAddress = aAddress;
			mName = aName;
			mTimeDiscovered = aTimeDiscovered;
		}

		public Protocol getProtocol() {
			return mProtocol;
		}

		public String getAddress() {
			return mAddress;
		}

		/**
		 * Get a human friendly name for the server.
		 *
		 * @return The name.
		 */
		public String getName() {
			return mName;
		}

		public long getTimeDiscovered() {
			return mTimeDiscovered;
		}

	}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */