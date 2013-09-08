/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.activity;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.DialogFragment;
import android.support.v4.app.Fragment;
import android.support.v4.content.LocalBroadcastManager;
import android.view.KeyEvent;

import com.actionbarsherlock.app.ActionBar;
import com.actionbarsherlock.app.SherlockFragmentActivity;
import com.actionbarsherlock.view.Menu;
import com.actionbarsherlock.view.MenuItem;
import org.libreoffice.impressremote.R;
import org.libreoffice.impressremote.communication.CommunicationService;
import org.libreoffice.impressremote.communication.SlideShow;
import org.libreoffice.impressremote.communication.Timer;
import org.libreoffice.impressremote.fragment.EmptySlideFragment;
import org.libreoffice.impressremote.fragment.SlidesGridFragment;
import org.libreoffice.impressremote.fragment.SlidesPagerFragment;
import org.libreoffice.impressremote.fragment.TimerEditingDialog;
import org.libreoffice.impressremote.fragment.TimerSettingDialog;
import org.libreoffice.impressremote.util.FragmentOperator;
import org.libreoffice.impressremote.util.Intents;
import org.libreoffice.impressremote.util.Preferences;
import org.libreoffice.impressremote.util.SavedStates;

public class SlideShowActivity extends SherlockFragmentActivity implements ServiceConnection {
    private static enum Mode {
        PAGER, GRID, EMPTY
    }

    private Mode mMode;

    private CommunicationService mCommunicationService;
    private IntentsReceiver mIntentsReceiver;

    @Override
    protected void onCreate(Bundle aSavedInstanceState) {
        super.onCreate(aSavedInstanceState);

        mMode = loadMode(aSavedInstanceState);

        setUpHomeButton();
        setUpFragment();
        setUpKeepingScreenOn();

        bindService();
    }

    private Mode loadMode(Bundle aSavedInstanceState) {
        if (aSavedInstanceState == null) {
            return Mode.PAGER;
        }

        return (Mode) aSavedInstanceState.getSerializable(SavedStates.Keys.MODE);
    }

    private void setUpHomeButton() {
        getSupportActionBar().setHomeButtonEnabled(true);
    }

    private void setUpFragment() {
        FragmentOperator.replaceFragmentAnimated(this, buildFragment());
    }

    private Fragment buildFragment() {
        switch (mMode) {
            case PAGER:
                return SlidesPagerFragment.newInstance();

            case GRID:
                return SlidesGridFragment.newInstance();

            case EMPTY:
                return EmptySlideFragment.newInstance();

            default:
                return SlidesPagerFragment.newInstance();
        }
    }

    private void setUpKeepingScreenOn() {
        findViewById(android.R.id.content).setKeepScreenOn(isKeepingScreenOnRequired());
    }

    private boolean isKeepingScreenOnRequired() {
        Preferences aPreferences = Preferences.getSettingsInstance(this);

        return aPreferences.getBoolean(Preferences.Keys.KEEP_SCREEN_ON);
    }

    private void bindService() {
        Intent aIntent = Intents.buildCommunicationServiceIntent(this);
        bindService(aIntent, this, Context.BIND_AUTO_CREATE);
    }

    @Override
    public void onServiceConnected(ComponentName mComponentName, IBinder aBinder) {
        CommunicationService.CBinder aServiceBinder = (CommunicationService.CBinder) aBinder;
        mCommunicationService = aServiceBinder.getService();

        startSlideShow();
    }

    private void startSlideShow() {
        mCommunicationService.getTransmitter().startPresentation();
    }

    @Override
    protected void onStart() {
        super.onStart();

        registerIntentsReceiver();
    }

    private void registerIntentsReceiver() {
        mIntentsReceiver = new IntentsReceiver(this);
        IntentFilter aIntentFilter = buildIntentsReceiverFilter();

        getBroadcastManager().registerReceiver(mIntentsReceiver, aIntentFilter);
    }

    private static final class IntentsReceiver extends BroadcastReceiver {
        private final SlideShowActivity mSlideShowActivity;

        private IntentsReceiver(SlideShowActivity aSlideShowActivity) {
            mSlideShowActivity = aSlideShowActivity;
        }

        @Override
        public void onReceive(Context aContext, Intent aIntent) {
            if (Intents.Actions.SLIDE_CHANGED.equals(aIntent.getAction())) {
                mSlideShowActivity.setUpSlideShowInformation();
                return;
            }

            if (Intents.Actions.TIMER_UPDATED.equals(aIntent.getAction())) {
                mSlideShowActivity.setUpSlideShowInformation();
                return;
            }

            if (Intents.Actions.TIMER_STARTED.equals(aIntent.getAction())) {
                int aMinutesLength = aIntent.getIntExtra(Intents.Extras.MINUTES, 0);
                mSlideShowActivity.startTimer(aMinutesLength);
                mSlideShowActivity.setUpSlideShowInformation();
                return;
            }

            if (Intents.Actions.TIMER_RESUMED.equals(aIntent.getAction())) {
                mSlideShowActivity.resumeTimer();
                return;
            }

            if (Intents.Actions.TIMER_CHANGED.equals(aIntent.getAction())) {
                int aMinutesLength = aIntent.getIntExtra(Intents.Extras.MINUTES, 0);
                mSlideShowActivity.changeTimer(aMinutesLength);
                mSlideShowActivity.resumeTimer();
                mSlideShowActivity.setUpSlideShowInformation();
            }
        }
    }

    private IntentFilter buildIntentsReceiverFilter() {
        IntentFilter aIntentFilter = new IntentFilter();
        aIntentFilter.addAction(Intents.Actions.SLIDE_CHANGED);
        aIntentFilter.addAction(Intents.Actions.TIMER_UPDATED);
        aIntentFilter.addAction(Intents.Actions.TIMER_STARTED);
        aIntentFilter.addAction(Intents.Actions.TIMER_RESUMED);
        aIntentFilter.addAction(Intents.Actions.TIMER_CHANGED);

        return aIntentFilter;
    }

    private LocalBroadcastManager getBroadcastManager() {
        return LocalBroadcastManager.getInstance(getApplicationContext());
    }

    private void setUpSlideShowInformation() {
        if (!isServiceBound()) {
            return;
        }

        ActionBar aActionBar = getSupportActionBar();

        aActionBar.setTitle(buildSlideShowProgress());
        aActionBar.setSubtitle(buildSlideShowTimerProgress());
    }

    private boolean isServiceBound() {
        return mCommunicationService != null;
    }

    private String buildSlideShowProgress() {
        SlideShow aSlideShow = mCommunicationService.getSlideShow();

        int aCurrentSlideIndex = aSlideShow.getHumanCurrentSlideIndex();
        int aSlidesCount = aSlideShow.getSlidesCount();

        return getString(R.string.mask_slide_show_progress, aCurrentSlideIndex, aSlidesCount);
    }

    private String buildSlideShowTimerProgress() {
        Timer aTimer = mCommunicationService.getSlideShow().getTimer();

        if (!aTimer.isSet()) {
            return null;
        }

        if (aTimer.isTimeUp()) {
            return getString(R.string.message_time_is_up);
        }

        int aMinutesLeft = aTimer.getMinutesLeft();

        return getResources().getQuantityString(R.plurals.mask_timer_progress, aMinutesLeft, aMinutesLeft);
    }

    private void startTimer(int aMinutesLength) {
        Timer aTimer = mCommunicationService.getSlideShow().getTimer();

        aTimer.setMinutesLength(aMinutesLength);
        aTimer.start();
    }

    private void resumeTimer() {
        mCommunicationService.getSlideShow().getTimer().resume();
    }

    private void changeTimer(int aMinutesLength) {
        Timer aTimer = mCommunicationService.getSlideShow().getTimer();

        aTimer.setMinutesLength(aMinutesLength);
    }

    @Override
    protected void onResume() {
        super.onResume();

        setUpSlideShowInformation();
    }

    @Override
    public boolean onKeyDown(int aKeyCode, KeyEvent aKeyEvent) {
        if (!areVolumeKeysActionsRequired()) {
            return super.onKeyDown(aKeyCode, aKeyEvent);
        }

        switch (aKeyCode) {
            case KeyEvent.KEYCODE_VOLUME_UP:
                if (!isLastSlideDisplayed()) {
                    mCommunicationService.getTransmitter().performNextTransition();
                }
                return true;

            case KeyEvent.KEYCODE_VOLUME_DOWN:
                mCommunicationService.getTransmitter().performPreviousTransition();
                return true;

            default:
                return super.onKeyDown(aKeyCode, aKeyEvent);
        }
    }

    private boolean areVolumeKeysActionsRequired() {
        Preferences aPreferences = Preferences.getSettingsInstance(this);

        return aPreferences.getBoolean(Preferences.Keys.VOLUME_KEYS_ACTIONS);
    }

    private boolean isLastSlideDisplayed() {
        int aCurrentSlideIndex = mCommunicationService.getSlideShow().getHumanCurrentSlideIndex();
        int aSlidesCount = mCommunicationService.getSlideShow().getSlidesCount();

        return aCurrentSlideIndex == aSlidesCount;
    }

    @Override
    public boolean onKeyUp(int aKeyCode, KeyEvent aKeyEvent) {
        if (!areVolumeKeysActionsRequired()) {
            return super.onKeyUp(aKeyCode, aKeyEvent);
        }

        // Suppress sound of volume changing

        switch (aKeyCode) {
            case KeyEvent.KEYCODE_VOLUME_UP:
            case KeyEvent.KEYCODE_VOLUME_DOWN:
                return true;

            default:
                return super.onKeyUp(aKeyCode, aKeyEvent);
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu aMenu) {
        getSupportMenuInflater().inflate(R.menu.menu_action_bar_slide_show, aMenu);

        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu aMenu) {
        MenuItem aSlidesPagerMenuItem = aMenu.findItem(R.id.menu_slides_pager);
        MenuItem aSlidesGridMenuItem = aMenu.findItem(R.id.menu_slides_grid);
        MenuItem aSlideShowResumeMenuItem = aMenu.findItem(R.id.menu_resume_slide_show);

        switch (mMode) {
            case PAGER:
                setMenuItemsVisibility(aMenu, true);
                aSlidesPagerMenuItem.setVisible(false);
                aSlidesGridMenuItem.setVisible(true);
                aSlideShowResumeMenuItem.setVisible(false);
                break;

            case GRID:
                setMenuItemsVisibility(aMenu, true);
                aSlidesPagerMenuItem.setVisible(true);
                aSlidesGridMenuItem.setVisible(false);
                aSlideShowResumeMenuItem.setVisible(false);
                break;

            case EMPTY:
                setMenuItemsVisibility(aMenu, false);
                aSlideShowResumeMenuItem.setVisible(true);
                break;

            default:
                break;
        }

        return super.onPrepareOptionsMenu(aMenu);
    }

    private void setMenuItemsVisibility(Menu aMenu, boolean aAreItemsVisible) {
        for (int aItemIndex = 0; aItemIndex < aMenu.size(); aItemIndex++) {
            aMenu.getItem(aItemIndex).setVisible(aAreItemsVisible);
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem aMenuItem) {
        switch (aMenuItem.getItemId()) {
            case android.R.id.home:
                navigateUp();
                return true;

            case R.id.menu_slides_grid:
                changeMode(Mode.GRID);
                return true;

            case R.id.menu_slides_pager:
                changeMode(Mode.PAGER);
                return true;

            case R.id.menu_timer:
                callTimer();
                return true;

            case R.id.menu_resume_slide_show:
                changeMode(Mode.PAGER);
                setUpSlideShowInformation();
                resumeSlideShow();
                resumeTimer();
                return true;

            case R.id.menu_pause_slide_show:
                changeMode(Mode.EMPTY);
                setUpSlideShowPausedInformation();
                pauseSlideShow();
                pauseTimer();
                return true;

            case R.id.menu_stop_slide_show:
                stopSlideShow();
                return true;

            default:
                return super.onOptionsItemSelected(aMenuItem);
        }
    }

    private void navigateUp() {
        finish();
    }

    private void changeMode(Mode aMode) {
        mMode = aMode;

        setUpFragment();
        refreshActionBarMenu();
    }

    private void refreshActionBarMenu() {
        supportInvalidateOptionsMenu();
    }

    private void callTimer() {
        Timer aTimer = mCommunicationService.getSlideShow().getTimer();

        if (aTimer.isSet()) {
            callEditingTimer(aTimer);
        } else {
            callSettingTimer();
        }
    }

    private void callEditingTimer(Timer aTimer) {
        DialogFragment aTimerDialog = buildTimerEditingDialog(aTimer);
        aTimerDialog.show(getSupportFragmentManager(), TimerEditingDialog.TAG);

        aTimer.pause();
    }

    private DialogFragment buildTimerEditingDialog(Timer aTimer) {
        if (aTimer.isTimeUp()) {
            return TimerEditingDialog.newInstance(aTimer.getMinutesLength());
        } else {
            return TimerEditingDialog.newInstance(aTimer.getMinutesLeft());
        }
    }

    private void callSettingTimer() {
        DialogFragment aTimerDialog = TimerSettingDialog.newInstance();
        aTimerDialog.show(getSupportFragmentManager(), TimerSettingDialog.TAG);
    }

    private void resumeSlideShow() {
        mCommunicationService.getTransmitter().resumePresentation();
    }

    private void pauseSlideShow() {
        mCommunicationService.getTransmitter().setUpBlankScreen();
    }

    private void setUpSlideShowPausedInformation() {
        ActionBar aActionBar = getSupportActionBar();

        aActionBar.setTitle(R.string.title_slide_show);
        aActionBar.setSubtitle(R.string.message_paused);
    }

    private void pauseTimer() {
        mCommunicationService.getSlideShow().getTimer().pause();
    }

    private void stopSlideShow() {
        mCommunicationService.getTransmitter().stopPresentation();

        finish();
    }

    @Override
    protected void onStop() {
        super.onStop();

        unregisterIntentsReceiver();
    }

    private void unregisterIntentsReceiver() {
        try {
            getBroadcastManager().unregisterReceiver(mIntentsReceiver);
        } catch (IllegalArgumentException e) {
            // Receiver not registered.
            // Fixed in Honeycomb: Android’s issue #6191.
        }
    }

    @Override
    protected void onSaveInstanceState(Bundle aOutState) {
        super.onSaveInstanceState(aOutState);

        saveMode(aOutState);
    }

    private void saveMode(Bundle aOutState) {
        aOutState.putSerializable(SavedStates.Keys.MODE, mMode);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (!isServiceBound()) {
            return;
        }

        stopTimer();

        // TODO: disconnect computer

        unbindService();
    }

    private void stopTimer() {
        mCommunicationService.getSlideShow().getTimer().stop();
    }

    private void unbindService() {
        unbindService(this);
    }

    @Override
    public void onServiceDisconnected(ComponentName aComponentName) {
        mCommunicationService = null;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
