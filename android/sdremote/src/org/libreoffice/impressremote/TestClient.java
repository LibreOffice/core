package org.libreoffice.impressremote;

import org.libreoffice.impressremote.communication.CommunicationService;
import org.libreoffice.impressremote.communication.Transmitter;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class TestClient extends Activity {

	private boolean mIsBound = false;

	private CommunicationService mCommunicationService;

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.testlayout);
		setupUI();
		doBindService();

	}

	private ServiceConnection mConnection = new ServiceConnection() {
		public void onServiceConnected(ComponentName aClassName,
				IBinder aService) {
			mCommunicationService = ((CommunicationService.CBinder) aService)
					.getService();
			mCommunicationService.connectTo(
					CommunicationService.Protocol.NETWORK, "128.232.128.144");

			enableButtons(true);
		}

		public void onServiceDisconnected(ComponentName aClassName) {
			mCommunicationService = null;
			enableButtons(false);
		}
	};

	void doBindService() {
		bindService(new Intent(this, CommunicationService.class), mConnection,
				Context.BIND_AUTO_CREATE);
		mIsBound = true;
	}

	void doUnbindService() {
		if (mIsBound) {
			unbindService(mConnection);
			mIsBound = false;
		}
	}

	private Button mButtonNext;

	private Button mButtonPrevious;

	private void setupUI() {
		mButtonNext = (Button) findViewById(R.id.button_next);
		mButtonPrevious = (Button) findViewById(R.id.button_previous);

		enableButtons(false);

		mButtonNext.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				mCommunicationService.getTransmitter().nextTransition();

			}

		});

		mButtonPrevious.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				mCommunicationService.getTransmitter().previousTransition();

			}

		});

	}

	private void enableButtons(boolean aEnabled) {
		mButtonNext.setEnabled(aEnabled);
		mButtonPrevious.setEnabled(aEnabled);
	}
}
