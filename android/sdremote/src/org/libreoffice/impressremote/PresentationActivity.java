package org.libreoffice.impressremote;

import org.libreoffice.impressremote.communication.CommunicationService;

import android.app.Activity;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.view.Menu;
import android.widget.FrameLayout;

public class PresentationActivity extends Activity {
	private CommunicationService mCommunicationService;
	private boolean mIsBound = false;
	private FrameLayout mLayout;
	private ThumbnailFragment mThumbnailFragment;
	private PresentationFragment mPresentationFragment;
	private ActionBarManager mActionBarManager;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_presentation);

		bindService(new Intent(this, CommunicationService.class), mConnection,
		                Context.BIND_IMPORTANT);

		FragmentManager fragmentManager = getFragmentManager();
		FragmentTransaction fragmentTransaction = fragmentManager
		                .beginTransaction();
		mThumbnailFragment = new ThumbnailFragment();
		mPresentationFragment = new PresentationFragment();

		// fragmentTransaction.add(R.id.framelayout, mThumbnailFragment,
		// "fragment_thumbnail");
		fragmentTransaction.add(R.id.framelayout, mPresentationFragment,
		                "fragment_presentation");
		fragmentTransaction.commit();

		mLayout = (FrameLayout) findViewById(R.id.framelayout);

		mIsBound = true;
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.actionbar_presentation, menu);
		mActionBarManager = new ActionBarManager(this, menu,
		                mCommunicationService);
		return true;
	}

	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName aClassName,
		                IBinder aService) {
			mCommunicationService = ((CommunicationService.CBinder) aService)
			                .getService();
			mCommunicationService.setActivityMessenger(mMessenger);

			mPresentationFragment
			                .setCommunicationService(mCommunicationService);
			mThumbnailFragment.setCommunicationService(mCommunicationService);

		}

		@Override
		public void onServiceDisconnected(ComponentName aClassName) {
			mCommunicationService = null;
		}
	};

	final Messenger mMessenger = new Messenger(new MessageHandler());

	protected class MessageHandler extends Handler {
		@Override
		public void handleMessage(Message aMessage) {
			mPresentationFragment.handleMessage(aMessage);
			mThumbnailFragment.handleMessage(aMessage);
			// Bundle aData = aMessage.getData();
			// TODO: pass to fragments
			// switch (aMessage.what) {
			// case CommunicationService.MSG_SLIDE_CHANGED:
			// int aSlide = aData.getInt("slide_number");
			// break;
			// case CommunicationService.MSG_SLIDE_PREVIEW:
			// // int aNSlide = aData.getInt("slide_number");
			// break;
			//
			// }
		}
	}

}
