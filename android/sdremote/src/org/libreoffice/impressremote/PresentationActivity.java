package org.libreoffice.impressremote;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.TimeZone;

import org.libreoffice.impressremote.communication.CommunicationService;
import org.libreoffice.impressremote.communication.SlideShow.Timer;

import android.annotation.SuppressLint;
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
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.TextView;
import android.widget.ToggleButton;

public class PresentationActivity extends Activity {
	private CommunicationService mCommunicationService;
	private boolean mIsBound = false;
	private FrameLayout mLayout;
	private FrameLayout mOuterLayout;
	private ThumbnailFragment mThumbnailFragment;
	private PresentationFragment mPresentationFragment;
	private ActionBarManager mActionBarManager;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		bindService(new Intent(this, CommunicationService.class), mConnection,
		                Context.BIND_IMPORTANT);
		mIsBound = true;

		setContentView(R.layout.activity_presentation);
		mOuterLayout = (FrameLayout) findViewById(R.id.framelayout);
		mLayout = new InterceptorLayout(this);
		mOuterLayout.addView(mLayout);
		mLayout.setId(R.id.presentation_innerFrame);
		//		((FrameLayout) findViewById(R.id.framelayout)).addView(mLayout);
		mThumbnailFragment = new ThumbnailFragment();
		mPresentationFragment = new PresentationFragment();

		FragmentManager fragmentManager = getFragmentManager();
		FragmentTransaction fragmentTransaction = fragmentManager
		                .beginTransaction();
		fragmentTransaction.add(R.id.presentation_innerFrame,
		                mPresentationFragment, "fragment_presentation");
		fragmentTransaction.commit();
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

	@SuppressLint("HandlerLeak")
	protected class MessageHandler extends Handler {
		@Override
		public void handleMessage(Message aMessage) {
			mPresentationFragment.handleMessage(aMessage);
			mThumbnailFragment.handleMessage(aMessage);
		}
	}

	// ---------------------------------------------- ACTION BAR ---------------
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.actionbar_presentation, menu);
		mActionBarManager = new ActionBarManager();
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		Intent aIntent;
		switch (item.getItemId()) {
		case R.id.actionbar_presentation_submenu_options:
			//			FragmentTransaction ft = getFragmentManager().beginTransaction();
			//			ft.replace(R.id.presentation_innerFrame, new SettingsFragment());
			//			ft.setTransition(FragmentTransaction.TRANSIT_FRAGMENT_OPEN);
			//			ft.addToBackStack(null);
			//			ft.commit();
			aIntent = new Intent(this, SettingsActivity.class);
			startActivity(aIntent);
			return true;
		case R.id.actionbar_presentation_submenu_blank:
			return true;
		default:
			return super.onOptionsItemSelected(item);
		}
	}

	private class ActionBarManager implements OnClickListener,
	                FragmentManager.OnBackStackChangedListener,
	                TextView.OnEditorActionListener {

		private ToggleButton mTimeLabel;
		private ToggleButton mThumbnailButton;

		private View mDropdownOptions;
		private View mDropdownBlank;

		// ------- CLOCKBAR
		private View mClockBar;
		private ToggleButton mClockBar_clockButton;
		private ToggleButton mClockBar_stopwatchButton;
		private ToggleButton mClockBar_countdownButton;

		// ------- STOPWATCH BAR
		private View mStopwatchBar;
		private Button mStopwatchButtonRun;
		private Button mStopwatchButtonReset;

		// ------- COUNTDOWN BAR
		private View mCountdownBar;
		private EditText mCountdownEntry;
		private Button mCountdownButton;

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
			aBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM);
			aBar.setCustomView(R.layout.presentation_actionbar);

			mThumbnailButton = (ToggleButton) aBar.getCustomView()
			                .findViewById(R.id.actionbar_thumbnailtoggle);
			mThumbnailButton.setOnClickListener(this);

			mTimeLabel = (ToggleButton) aBar.getCustomView().findViewById(
			                R.id.actionbar_time);
			mTimeLabel.setOnClickListener(this);

			setupClockBar();

			getFragmentManager().addOnBackStackChangedListener(this);

			timerHandler.removeCallbacks(timerUpdateThread);
			timerHandler.postDelayed(timerUpdateThread, 50);

		}

		public void hidePopups() {
			if (mClockBar.getVisibility() == View.VISIBLE) {
				mClockBar.setVisibility(View.INVISIBLE);
				mStopwatchBar.setVisibility(View.INVISIBLE);
				mCountdownBar.setVisibility(View.INVISIBLE);
				mTimeLabel.setChecked(false);
			}
		}

		private void setupClockBar() {
			// ClockBar
			LayoutInflater aInflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			aInflater.inflate(R.layout.presentation_clockbar, mOuterLayout);
			mClockBar = mOuterLayout.findViewById(R.id.clockbar);

			mClockBar_clockButton = (ToggleButton) mClockBar
			                .findViewById(R.id.clockbar_toggle_clockmode);
			mClockBar_stopwatchButton = (ToggleButton) mClockBar
			                .findViewById(R.id.clockbar_toggle_stopwatchmode);
			mClockBar_countdownButton = (ToggleButton) mClockBar
			                .findViewById(R.id.clockbar_toggle_countdownmode);
			mClockBar_clockButton.setOnClickListener(this);
			mClockBar_stopwatchButton.setOnClickListener(this);
			mClockBar_countdownButton.setOnClickListener(this);

			// Stopwatch bar
			aInflater.inflate(R.layout.presentation_clockbar_stopwatchbar,
			                mOuterLayout);
			mStopwatchBar = mOuterLayout
			                .findViewById(R.id.clockbar_stopwatchbar);

			mStopwatchButtonRun = (Button) mStopwatchBar
			                .findViewById(R.id.clockbar_stopwatch_run);
			mStopwatchButtonReset = (Button) mStopwatchBar
			                .findViewById(R.id.clockbar_stopwatch_reset);
			mStopwatchButtonRun.setOnClickListener(this);
			mStopwatchButtonReset.setOnClickListener(this);

			// Countdown bar
			aInflater.inflate(R.layout.presentation_clockbar_countdownbar,
			                mOuterLayout);
			mCountdownBar = mOuterLayout
			                .findViewById(R.id.clockbar_countdownbar);

			mCountdownEntry = (EditText) mCountdownBar
			                .findViewById(R.id.clockbar_countdown_time);
			mCountdownButton = (Button) mCountdownBar
			                .findViewById(R.id.clockbar_countdown_button);
			mCountdownButton.setOnClickListener(this);
			mCountdownEntry.setOnEditorActionListener(this);

			updateClockBar();
			hidePopups();

		}

		private void updateClockBar() {
			mClockBar_clockButton.setChecked(!mTimerOn);

			mCountdownBar.setY(mClockBar.getHeight());
			mStopwatchBar.setY(mClockBar.getHeight());

			boolean aIsCountdown = mCommunicationService.getSlideShow()
			                .getTimer().isCountdown();
			// Stopwatch
			boolean aStopwatchMode = mTimerOn && !aIsCountdown;
			mClockBar_stopwatchButton.setChecked(aStopwatchMode);
			mStopwatchBar.setVisibility(aStopwatchMode ? View.VISIBLE
			                : View.INVISIBLE);
			mStopwatchBar.bringToFront();
			if (aStopwatchMode) {
				Timer aTimer = mCommunicationService.getSlideShow().getTimer();
				if (aTimer.isRunning()) {
					mStopwatchButtonRun.setText(R.string.clock_timer_pause);
					mStopwatchButtonReset.setText(R.string.clock_timer_restart);
				} else {
					mStopwatchButtonRun.setText(R.string.clock_timer_start);
					mStopwatchButtonReset.setText(R.string.clock_timer_reset);
				}
			}

			// Countdown
			boolean aCountdownMode = mTimerOn && aIsCountdown;
			mClockBar_countdownButton.setChecked(mTimerOn && aIsCountdown);
			mCountdownBar.setVisibility(mTimerOn && aIsCountdown ? View.VISIBLE
			                : View.INVISIBLE);
			mCountdownBar.bringToFront();
			if (aCountdownMode) {
				Timer aTimer = mCommunicationService.getSlideShow().getTimer();
				if (aTimer.isRunning()) {
					mCountdownButton.setText(R.string.clock_timer_pause);
				} else {
					mCountdownButton.setText(R.string.clock_timer_resume);
				}
			}

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
			Timer aTimer = mCommunicationService.getSlideShow().getTimer();
			// --------------------------------- ACTIONBAR BUTTONS -------------
			if (aSource == mThumbnailButton) {
				if (!mThumbnailFragment.isVisible()) {
					FragmentTransaction ft = getFragmentManager()
					                .beginTransaction();
					ft.replace(R.id.presentation_innerFrame, mThumbnailFragment);
					ft.setTransition(FragmentTransaction.TRANSIT_FRAGMENT_OPEN);
					ft.addToBackStack(null);
					ft.commit();
				} else {
					getFragmentManager().popBackStack();
				}
			} else if (aSource == mTimeLabel) {
				if (mClockBar.getVisibility() == View.VISIBLE) {
					hidePopups();
				} else {
					mClockBar.setVisibility(View.VISIBLE);
					updateClockBar();
					mClockBar.bringToFront();
				}
			}
			// ------------------------------------ CLOCKBAR BUTTONS -----------
			else if (aSource == mClockBar_clockButton) {
				mTimerOn = false;
				updateClockBar();
			} else if (aSource == mClockBar_stopwatchButton) {
				mTimerOn = true;
				if (aTimer.isCountdown()) { // Changing mode.
					aTimer.reset();
				}
				aTimer.setCountdown(false);
				updateClockBar();
			} else if (aSource == mClockBar_countdownButton) {
				mTimerOn = true;
				if (!aTimer.isCountdown()) { // Changing mode
					aTimer.reset();
				}
				aTimer.setCountdown(true);
				updateClockBar();
			}
			// ------------------------------------- TIMER BUTTONS
			else if (aSource == mStopwatchButtonRun) {
				if (aTimer.isRunning()) {
					aTimer.stopTimer();
				} else {
					aTimer.startTimer();
				}
				updateClockBar();
			} else if (aSource == mStopwatchButtonReset) {
				if (aTimer.isRunning()) {
					aTimer.reset();
					aTimer.startTimer();
				} else {
					aTimer.reset();
				}
				updateClockBar();
			} else if (aSource == mCountdownButton) {
				if (aTimer.isRunning()) {
					aTimer.stopTimer();
				} else {
					aTimer.startTimer();
				}
				updateClockBar();
			}

		}

		@Override
		public void onBackStackChanged() {
			if (getFragmentManager().getBackStackEntryCount() == 0) {
				mThumbnailButton.setChecked(false);
			}
		}

		@Override
		public boolean onEditorAction(TextView tv, int aID, KeyEvent aEvent) {
			if (aEvent.getKeyCode() == KeyEvent.KEYCODE_ENTER) {

				long aTime = 0;
				try {
					SimpleDateFormat aFormat = new SimpleDateFormat("HH:mm:ss");
					aFormat.setTimeZone(TimeZone.getTimeZone("UTC"));
					aTime = aFormat.parse(mCountdownEntry.getText().toString())
					                .getTime();
				} catch (ParseException e) {
				}
				if (aTime == 0) {
					try {
						SimpleDateFormat aFormat = new SimpleDateFormat("mm:ss");
						aFormat.setTimeZone(TimeZone.getTimeZone("UTC"));
						aTime = aFormat.parse(
						                mCountdownEntry.getText().toString())
						                .getTime();
					} catch (ParseException e) {
					}
				}
				mCommunicationService.getSlideShow().getTimer()
				                .setCountdownTime(aTime);
				return true;
			}
			return false;
		}
	}

	/**
	 * Intermediate layout that catches all touches, used in order to hide
	 * the clock menu as appropriate.
	 * @author andy
	 *
	 */
	private class InterceptorLayout extends FrameLayout {

		public InterceptorLayout(Context context) {
			super(context);
		}

		@Override
		public boolean onInterceptTouchEvent(MotionEvent aEvent) {
			mActionBarManager.hidePopups();
			return super.onInterceptTouchEvent(aEvent);
		}

		@Override
		public boolean onTouchEvent(MotionEvent aEvent) {
			return super.onTouchEvent(aEvent);
		}

	}
}
