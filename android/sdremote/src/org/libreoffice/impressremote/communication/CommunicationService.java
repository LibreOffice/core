package org.libreoffice.impressremote.communication;

import android.app.Service;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
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

	private Transmitter mTransmitter;

	private Client mClient;

	private Receiver mReceiver = new Receiver();

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

	public void connectTo(Protocol aProtocol, String address) {
		switch (aProtocol) {
		case NETWORK:
			mClient = new NetworkClient(address);
			mTransmitter = new Transmitter(mClient);
			mClient.setReceiver(mReceiver);
			break;

		}

	}

	public void disconnect() {
		mClient.closeConnection();
	}

	public SlideShow getSlideShow() {
		return mReceiver.getSlideShow();
	}


}
