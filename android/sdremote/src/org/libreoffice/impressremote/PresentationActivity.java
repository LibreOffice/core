/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.TimeZone;

import org.libreoffice.impressremote.communication.CommunicationService;
import org.libreoffice.impressremote.communication.SlideShow.Timer;

import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.preference.PreferenceManager;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.content.LocalBroadcastManager;
import android.text.format.DateFormat;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.TextView;
import android.widget.ToggleButton;

import com.actionbarsherlock.app.ActionBar;
import com.actionbarsherlock.app.SherlockFragmentActivity;

public class PresentationActivity extends SherlockFragmentActivity {
    private CommunicationService mCommunicationService;
    private FrameLayout mOuterLayout;
    private ThumbnailFragment mThumbnailFragment;
    private PresentationFragment mPresentationFragment;
    private static ActionBarManager mActionBarManager;
    private ActivityChangeBroadcastProcessor mBroadcastProcessor;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // bind preference defaults
        PreferenceManager.setDefaultValues(this, R.xml.preferences,
                                           false /* retain user settings */);

        mBroadcastProcessor = new ActivityChangeBroadcastProcessor(this);

        bindService(new Intent(this, CommunicationService.class), mConnection,
                        Context.BIND_IMPORTANT);

        IntentFilter aFilter = new IntentFilter();
        mBroadcastProcessor = new ActivityChangeBroadcastProcessor(this);
        mBroadcastProcessor.addToFilter(aFilter);
        LocalBroadcastManager.getInstance(this).registerReceiver(mListener,
                        aFilter);

        //((FrameLayout) findViewById(R.id.framelayout)).addView(mLayout);
        setContentView(R.layout.activity_presentation);

        mPresentationFragment = new PresentationFragment();

        FragmentManager fragmentManager = getSupportFragmentManager();

        FragmentTransaction fragmentTransaction = fragmentManager
            .beginTransaction();
        fragmentTransaction.replace(R.id.presentation_interceptor,
                                    mPresentationFragment, "fragment_presentation");
        fragmentTransaction.commit();
        mOuterLayout = (FrameLayout) findViewById(R.id.framelayout);
    }

    @Override
    public void onBackPressed() {
        if (getSupportFragmentManager().getBackStackEntryCount() > 0) {
            super.onBackPressed();
            return;
        }
        Intent aIntent = new Intent(this, SelectorActivity.class);
        aIntent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        startActivity(aIntent);
        if ( mCommunicationService != null )
            mCommunicationService.disconnect();
    }

    @Override
    protected void onDestroy() {
        mActionBarManager.stop();
        unbindService(mConnection);
        super.onDestroy();
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        SharedPreferences aPref = PreferenceManager
                        .getDefaultSharedPreferences(this);
        boolean aVolumeSwitching = aPref.getBoolean("option_volumeswitching",
                        false);
        boolean aRelevantFragmentVisible = ((mPresentationFragment != null) && mPresentationFragment
                        .isVisible())
                        || ((mThumbnailFragment != null) && mThumbnailFragment
                                        .isVisible());
        if (aVolumeSwitching && aRelevantFragmentVisible) {

            int action = event.getAction();
            int keyCode = event.getKeyCode();
            switch (keyCode) {
            case KeyEvent.KEYCODE_VOLUME_UP:
                if (action == KeyEvent.ACTION_UP) {
                    mCommunicationService.getTransmitter().nextTransition();
                }
                return true;
            case KeyEvent.KEYCODE_VOLUME_DOWN:
                if (action == KeyEvent.ACTION_DOWN) {
                    mCommunicationService.getTransmitter().previousTransition();
                }
                return true;
            }
        }
        return super.dispatchKeyEvent(event);
    }

    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName aClassName,
                        IBinder aService) {
            mCommunicationService = ((CommunicationService.CBinder) aService)
                            .getService();

            if (mThumbnailFragment != null)
                mThumbnailFragment
                                .setCommunicationService(mCommunicationService);

        }

        @Override
        public void onServiceDisconnected(ComponentName aClassName) {
            mCommunicationService = null;
        }
    };

    // ---------------------------------------------- ACTION BAR ---------------

    @Override
    public boolean onCreateOptionsMenu(com.actionbarsherlock.view.Menu menu) {
        getSupportMenuInflater().inflate(R.menu.actionbar_presentation, menu);
        mActionBarManager = new ActionBarManager();
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(
                    com.actionbarsherlock.view.MenuItem item) {
        Intent aIntent;
        switch (item.getItemId()) {
        case R.id.actionbar_presentation_submenu_options:

            aIntent = new Intent(this, SettingsActivity.class);
            startActivity(aIntent);
            return true;
        case R.id.actionbar_presentation_submenu_blank:
            boolean aRelevantFragmentVisible = (mPresentationFragment != null && mPresentationFragment
                            .isVisible())
                            || (mThumbnailFragment != null && mThumbnailFragment
                                            .isVisible());
            if (aRelevantFragmentVisible) {

                BlankScreenFragment aFragment = new BlankScreenFragment(
                                mCommunicationService);

                FragmentTransaction ft = getSupportFragmentManager()
                                .beginTransaction();
                ft.replace(R.id.presentation_interceptor, aFragment);
                ft.setTransition(FragmentTransaction.TRANSIT_FRAGMENT_OPEN);
                ft.addToBackStack(null);
                ft.commit();
            }
            return true;
        case R.id.actionbar_presentation_submenu_about:
            AboutDialogBuilder aBuilder = new AboutDialogBuilder(this);
            AlertDialog aDialog = aBuilder.create();
            aDialog.show();
            return true;
        default:
            return super.onOptionsItemSelected(item);
        }
    }

    private class ActionBarManager implements OnClickListener,
                    FragmentManager.OnBackStackChangedListener,
                    TextView.OnEditorActionListener {

        private Handler timerHandler = new Handler();

        private ToggleButton mTimeLabel;
        private ToggleButton mThumbnailButton;

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

        public void stop() {
            timerHandler.removeCallbacks(timerUpdateRunnable);
        }

        public ActionBarManager() {

            ActionBar aBar = getSupportActionBar();
            aBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM);
            aBar.setCustomView(R.layout.presentation_actionbar);

            mThumbnailButton = (ToggleButton) aBar.getCustomView()
                            .findViewById(R.id.actionbar_thumbnailtoggle);
            mThumbnailButton.setOnClickListener(this);

            mTimeLabel = (ToggleButton) aBar.getCustomView().findViewById(
                            R.id.actionbar_time);
            mTimeLabel.setOnClickListener(this);

            setupClockBar();

            getSupportFragmentManager().addOnBackStackChangedListener(this);

            timerHandler.removeCallbacks(timerUpdateRunnable);
            timerHandler.postDelayed(timerUpdateRunnable, 50);

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
            mStopwatchBar = mOuterLayout
                            .findViewById(R.id.clockbar_stopwatchbar);

            mStopwatchButtonRun = (Button) mStopwatchBar
                            .findViewById(R.id.clockbar_stopwatch_run);
            mStopwatchButtonReset = (Button) mStopwatchBar
                            .findViewById(R.id.clockbar_stopwatch_reset);
            mStopwatchButtonRun.setOnClickListener(this);
            mStopwatchButtonReset.setOnClickListener(this);

            // Countdown bar
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
            if (mCommunicationService == null) {
                return;
            }
            mClockBar_clockButton.setChecked(!mTimerOn);

            //            FrameLayout.LayoutParams aParams = (LayoutParams) mCountdownBar
            //                            .getLayoutParams();
            //            aParams.topMargin = mClockBar.getBottom();
            //            //            aParams.height = mClockBar.getHeight();
            //            //            aParams.bottomMargin = aParams.topMargin + aParams.height;
            //            //            aParams.leftMargin = 100;
            //            mCountdownBar.setLayoutParams(aParams);
            //            mCountdownBar.setPadding(0, mClockBar.getBottom(), 0, 0);
            //            mStopwatchBar.setPadding(0, mClockBar.getBottom(), 0, 0);
            //            //            mCountdownBar.requestLayout();
            //            //            mOuterLayout.requestLayout();
            //            //            mOuterLayout.invalidate();
            //            //            aParams = (LayoutParams) mStopwatchBar.getLayoutParams();
            //            //            aParams.topMargin = mClockBar.getHeight();
            //            //            mOuterLayout.invalidate();
            //            //            mStopwatchBar.setY(mClockBar.getHeight());

            boolean aIsCountdown = mCommunicationService.getSlideShow()
                            .getTimer().isCountdown();
            // Stopwatch
            boolean aStopwatchMode = mTimerOn && !aIsCountdown;
            mClockBar_stopwatchButton.setChecked(aStopwatchMode);
            mStopwatchBar.setVisibility(aStopwatchMode ? View.VISIBLE
                            : View.GONE);
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
                            : View.GONE);
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

        private Runnable timerUpdateRunnable = new Runnable() {

            @Override
            public void run() {
                CharSequence aTimeString;
                long aTime = System.currentTimeMillis();
                if (mTimerOn && mCommunicationService != null) {
                    aTime = mCommunicationService.getSlideShow().getTimer()
                                    .getTimeMillis();
                    aTimeString = DateFormat.format(aTimerFormat, aTime);
                } else {
                    aTimeString = DateFormat.format(aTimeFormat, aTime);
                }
                mTimeLabel.setText(aTimeString);
                timerHandler.postDelayed(this, 50);
            }

        };

        @Override
        public void onClick(View aSource) {
            Timer aTimer = mCommunicationService.getSlideShow().getTimer();
            // --------------------------------- ACTIONBAR BUTTONS -------------
            if (aSource == mThumbnailButton) {
                if (mThumbnailFragment == null) {
                    mThumbnailFragment = (ThumbnailFragment) getSupportFragmentManager()
                                    .findFragmentByTag("ThumbnailFragment");
                    if (mThumbnailFragment == null) {
                        mThumbnailFragment = new ThumbnailFragment();
                        mThumbnailFragment
                                        .setCommunicationService(mCommunicationService);
                    }
                }
                if (!mThumbnailFragment.isVisible()) {
                    FragmentTransaction ft = getSupportFragmentManager()
                                    .beginTransaction();
                    ft.replace(R.id.presentation_interceptor,
                                    mThumbnailFragment, "ThumbnailFragment");
                    ft.setTransition(FragmentTransaction.TRANSIT_FRAGMENT_OPEN);
                    ft.addToBackStack(null);
                    ft.commit();
                } else {
                    getSupportFragmentManager().popBackStack();
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
            if (getSupportFragmentManager().getBackStackEntryCount() == 0) {
                mThumbnailButton.setChecked(false);
            }
        }

        @Override
        public boolean onEditorAction(TextView tv, int aID, KeyEvent aEvent) {
            if (aEvent != null && aEvent.getKeyCode() == KeyEvent.KEYCODE_ENTER) {

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
     *
     */
    public static class InterceptorLayout extends FrameLayout {

        public InterceptorLayout(Context context, AttributeSet aAttrs) {
            super(context, aAttrs);
        }

        @Override
        public boolean onInterceptTouchEvent(MotionEvent aEvent) {
            mActionBarManager.hidePopups();
            return super.onInterceptTouchEvent(aEvent);
        }
    }

    private BroadcastReceiver mListener = new BroadcastReceiver() {

        @Override
        public void onReceive(Context aContext, Intent aIntent) {
            mBroadcastProcessor.onReceive(aContext, aIntent);
        }
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
