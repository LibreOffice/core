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

import com.actionbarsherlock.app.ActionBar;
import com.actionbarsherlock.app.SherlockFragmentActivity;
import com.actionbarsherlock.view.Menu;
import com.actionbarsherlock.view.MenuItem;
import org.libreoffice.impressremote.R;
import org.libreoffice.impressremote.communication.CommunicationService;
import org.libreoffice.impressremote.communication.SlideShow;
import org.libreoffice.impressremote.communication.Timer;
import org.libreoffice.impressremote.fragment.SlidesGridFragment;
import org.libreoffice.impressremote.fragment.SlidesPagerFragment;
import org.libreoffice.impressremote.fragment.TimerEditingDialog;
import org.libreoffice.impressremote.fragment.TimerSettingDialog;
import org.libreoffice.impressremote.util.FragmentOperator;
import org.libreoffice.impressremote.util.Intents;

public class SlideShowActivity extends SherlockFragmentActivity implements ServiceConnection {
    private static enum Mode {
        PAGER, GRID
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

        bindService();
    }

    private Mode loadMode(Bundle aSavedInstanceState) {
        if (aSavedInstanceState == null) {
            return Mode.PAGER;
        }

        return (Mode) aSavedInstanceState.getSerializable("MODE");
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

            default:
                return SlidesPagerFragment.newInstance();
        }
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
    public boolean onCreateOptionsMenu(Menu aMenu) {
        getSupportMenuInflater().inflate(R.menu.menu_action_bar_slide_show, aMenu);

        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu aMenu) {
        MenuItem aSlidesPagerMenuItem = aMenu.findItem(R.id.menu_slides_pager);
        MenuItem aSlidesGridMenuItem = aMenu.findItem(R.id.menu_slides_grid);

        switch (mMode) {
            case PAGER:
                aSlidesPagerMenuItem.setVisible(false);
                aSlidesGridMenuItem.setVisible(true);
                break;

            case GRID:
                aSlidesPagerMenuItem.setVisible(true);
                aSlidesGridMenuItem.setVisible(false);
                break;

            default:
                break;
        }

        return super.onPrepareOptionsMenu(aMenu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem aMenuItem) {
        switch (aMenuItem.getItemId()) {
            case android.R.id.home:
                navigateUp();

                return true;

            case R.id.menu_slides_grid:
                mMode = Mode.GRID;

                setUpFragment();
                refreshActionBarMenu();

                return true;

            case R.id.menu_slides_pager:
                mMode = Mode.PAGER;

                setUpFragment();
                refreshActionBarMenu();

                return true;

            case R.id.menu_timer:
                callTimer();

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

    private void refreshActionBarMenu() {
        supportInvalidateOptionsMenu();
    }

    private void callTimer() {
        Timer aTimer = mCommunicationService.getSlideShow().getTimer();

        if (aTimer.isSet()) {
            callEditingTimer(aTimer);
        }
        else {
            callSettingTimer();
        }
    }

    private void callEditingTimer(Timer aTimer) {
        int aTimerLength = aTimer.getMinutesLeft();

        DialogFragment aFragment = TimerEditingDialog.newInstance(aTimerLength);
        aFragment.show(getSupportFragmentManager(), TimerEditingDialog.TAG);

        pauseTimer();
    }

    private void pauseTimer() {
        mCommunicationService.getSlideShow().getTimer().pause();
    }

    private void callSettingTimer() {
        DialogFragment aFragment = TimerSettingDialog.newInstance();
        aFragment.show(getSupportFragmentManager(), TimerSettingDialog.TAG);
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
        aOutState.putSerializable("MODE", mMode);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (!isServiceBound()) {
            return;
        }

        stopTimer();

        disconnectComputer();

        unbindService();
    }

    private void stopTimer() {
        mCommunicationService.getSlideShow().getTimer().stop();
    }

    private void disconnectComputer() {
        mCommunicationService.disconnect();
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
