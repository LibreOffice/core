package org.libreoffice.impressremote;

import java.util.HashMap;

import org.libreoffice.impressremote.communication.CommunicationService;
import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

public class TestClient extends Activity {

	private HashMap<Integer, Bitmap> mPreviewImages = new HashMap<Integer, Bitmap>();
	private boolean mCurrentPreviewImageMissing = false;

	private boolean mIsBound = false;

	private CommunicationService mCommunicationService;

	final Messenger mMessenger = new Messenger(new MessageHandler());

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.testlayout);
		setupUI();

	}

	@Override
	protected void onResume() {
		super.onResume();
		doBindService();
	}

	// FIXME: move all necessary code to CommunicationService.onUnbind

	@Override
	protected void onPause() {
		super.onPause();
		doUnbindService();
		stopService(new Intent(this, CommunicationService.class));
	}

	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName aClassName,
				IBinder aService) {
			mCommunicationService = ((CommunicationService.CBinder) aService)
					.getService();
			mCommunicationService.connectTo(
					CommunicationService.Protocol.NETWORK, "10.0.2.2");
			mCommunicationService.setActivityMessenger(mMessenger);
			enableButtons(true);
		}

		@Override
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
		mCommunicationService.disconnect();
		mCommunicationService.setActivityMessenger(null);
		if (mIsBound) {
			unbindService(mConnection);
			mIsBound = false;
		}
	}

	private Button mButtonNext;

	private Button mButtonPrevious;

	private ImageView mImageView;

	private TextView mSlideLabel;

	private void setupUI() {
		mButtonNext = (Button) findViewById(R.id.button_next);
		mButtonPrevious = (Button) findViewById(R.id.button_previous);
		mImageView = (ImageView) findViewById(R.id.image_preview);
		mSlideLabel = (TextView) findViewById(R.id.label_curSlide);

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

	class MessageHandler extends Handler {
		@Override
		public void handleMessage(Message aMessage) {
			Bundle aData = aMessage.getData();
			switch (aMessage.what) {
			case CommunicationService.MSG_SLIDE_CHANGED:
				int newSlide = aData.getInt("slide_number");
				mSlideLabel.setText("Slide " + newSlide);
				if (mPreviewImages.containsKey(newSlide)) {
					mImageView.setImageBitmap(mPreviewImages.get(newSlide));
					mCurrentPreviewImageMissing = false;
				} else {
					mCurrentPreviewImageMissing = true;
				}
				break;
			case CommunicationService.MSG_SLIDE_PREVIEW:
				int aSlideNumber = aData.getInt("slide_number");
				byte[] aPreviewImage = aData.getByteArray("preview_image");
				Bitmap aBitmap = BitmapFactory.decodeByteArray(aPreviewImage,
						0, aPreviewImage.length);
				mPreviewImages.put(aSlideNumber, aBitmap);
				if (mCurrentPreviewImageMissing) {
					mImageView.setImageBitmap(aBitmap);
					mCurrentPreviewImageMissing = false;
				}
				break;

			}
		}
	}
}
