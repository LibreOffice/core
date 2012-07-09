package org.libreoffice.impressremote.communication;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;

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

	private Transmitter mTransmitter;

	private Client mClient;

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

	public void connectTo(Protocol aProtocol, String address) {
		switch (aProtocol) {
		case NETWORK:
			mClient = new NetworkClient(address);
			mTransmitter = new Transmitter(mClient);
			break;

		}

	}

	public void disconnect() {

	}

}
