package org.libreoffice.impressremote;

import org.libreoffice.impressremote.communication.CommunicationService;

import android.app.ActionBar;
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
import android.text.format.DateFormat;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.FrameLayout;
import android.widget.ToggleButton;

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
		mActionBarManager = new ActionBarManager();
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

	private class ActionBarManager implements OnClickListener,
	                FragmentManager.OnBackStackChangedListener {

		private ToggleButton mTimeLabel;
		private ToggleButton mThumbnailButton;

		private View mClockBar;

		private String aTimeFormat = getResources().getString(
		                R.string.actionbar_timeformat);
		private String aTimerFormat = getResources().getString(
		                R.string.actionbar_timerformat);
		/*
		 * True if the timer is being used as a timer, false if we are showing a
		 * clock.
		 */
		private boolean mTimerOn = false;

		public ActionBarManager() {

			ActionBar aBar = getActionBar();
			// Set custom view and unset the title.
			aBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM,
			                ActionBar.DISPLAY_SHOW_CUSTOM
			                                | ActionBar.DISPLAY_SHOW_TITLE);
			aBar.setCustomView(R.layout.presentation_actionbar);

			// Set up the various components
			mThumbnailButton = (ToggleButton) aBar.getCustomView()
			                .findViewById(R.id.actionbar_thumbnailtoggle);
			mThumbnailButton.setOnClickListener(this);

			mTimeLabel = (ToggleButton) aBar.getCustomView().findViewById(
			                R.id.actionbar_time);
			mTimeLabel.setOnClickListener(this);

			// Listen for fragment changes
			getFragmentManager().addOnBackStackChangedListener(this);

			// Setup the auto updater
			timerHandler.removeCallbacks(timerUpdateThread);
			timerHandler.postDelayed(timerUpdateThread, 50);

		}

		private Handler timerHandler = new Handler();

		private Thread timerUpdateThread = new Thread() {

			@Override
			public void run() {
				//			invalidateOptionsMenu();
				CharSequence aTimeString;
				long aTime = mCommunicationService.getSlideShow().getTimer()
				                .getTimeMillis();
				if (mTimerOn) {
					aTimeString = DateFormat.format(aTimerFormat, aTime);
				} else {
					aTimeString = DateFormat.format(aTimeFormat,
					                System.currentTimeMillis());
				}
				mTimeLabel.setText(aTimeString);
				// TODO: set the string
				timerHandler.postDelayed(this, 50);

			}

		};

		@Override
		public void onClick(View aSource) {
			if (aSource == mThumbnailButton) {
				if (!mThumbnailFragment.isVisible()) {
					FragmentTransaction ft = getFragmentManager()
					                .beginTransaction();
					ft.replace(R.id.framelayout, mThumbnailFragment);
					ft.setTransition(FragmentTransaction.TRANSIT_FRAGMENT_OPEN);
					ft.addToBackStack(null);
					ft.commit();
				} else {
					getFragmentManager().popBackStack();
				}
			} else if (aSource == mTimeLabel) {
				if (mClockBar == null) {
					LayoutInflater aInflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
					mClockBar = aInflater.inflate(
					                R.layout.presentation_clockbar, mLayout);
					mClockBar = mLayout.findViewById(R.id.clockbar);
					if (!mTimerOn) {
						((ToggleButton) mClockBar
						                .findViewById(R.id.clockbar_toggle_clockmode))
						                .setChecked(true);
					} else {
						boolean aIsCountdown = mCommunicationService
						                .getSlideShow().getTimer()
						                .isCountdown();
						((ToggleButton) mClockBar
						                .findViewById(R.id.clockbar_toggle_stopwatchmode))
						                .setChecked(!aIsCountdown);
						((ToggleButton) mClockBar
						                .findViewById(R.id.clockbar_toggle_countdownmode))
						                .setChecked(aIsCountdown);
					}

				} else {
					//					mClockBar.setVisibility(View.INVISIBLE);

					//					((ViewGroup) mClockBar.getParent()).removeView(mClockBar);
					if (mClockBar.getVisibility() == View.VISIBLE) {
						mClockBar.setVisibility(View.INVISIBLE);
					} else {
						mClockBar.setVisibility(View.VISIBLE);
						mClockBar.bringToFront();
					}
					//					mLayout.removeView(mClockBar);
					//					mClockBar = null;
				}
			}

		}

		@Override
		public void onBackStackChanged() {
			if (getFragmentManager().getBackStackEntryCount() == 0) {
				mThumbnailButton.setChecked(false);
			}
		}
	}

}
