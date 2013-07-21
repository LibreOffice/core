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
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.content.LocalBroadcastManager;

import com.actionbarsherlock.app.SherlockFragmentActivity;
import com.actionbarsherlock.view.Menu;
import com.actionbarsherlock.view.MenuItem;
import org.libreoffice.impressremote.R;
import org.libreoffice.impressremote.communication.CommunicationService;
import org.libreoffice.impressremote.communication.SlideShow;
import org.libreoffice.impressremote.fragment.SlidesGridFragment;
import org.libreoffice.impressremote.fragment.SlidesPagerFragment;
import org.libreoffice.impressremote.util.Intents;

public class SlideShowActivity extends SherlockFragmentActivity implements ServiceConnection {
    private static enum Mode {
        PAGER, GRID
    }

    private Mode mMode;

    private CommunicationService mCommunicationService;
    private IntentsReceiver mIntentsReceiver;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mMode = Mode.PAGER;

        setUpHomeButton();
        setUpFragment();

        bindService();
    }

    private void setUpHomeButton() {
        getSupportActionBar().setHomeButtonEnabled(true);
    }

    private void setUpFragment() {
        setUpFragment(buildFragment());
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

    private void setUpFragment(Fragment aFragment) {
        FragmentTransaction aTransaction = getSupportFragmentManager().beginTransaction();
        aTransaction.setCustomAnimations(android.R.anim.fade_in, android.R.anim.fade_out);

        aTransaction.replace(android.R.id.content, aFragment);

        aTransaction.commit();
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
            }
        }
    }

    private IntentFilter buildIntentsReceiverFilter() {
        IntentFilter aIntentFilter = new IntentFilter();
        aIntentFilter.addAction(Intents.Actions.SLIDE_CHANGED);

        return aIntentFilter;
    }

    private LocalBroadcastManager getBroadcastManager() {
        return LocalBroadcastManager.getInstance(getApplicationContext());
    }

    private void setUpSlideShowInformation() {
        if (!isServiceBound()) {
            return;
        }

        getSupportActionBar().setTitle(buildSlideShowProgress());
        getSupportActionBar().setSubtitle(buildSlideShowTimerProgress());
    }

    private String buildSlideShowProgress() {
        SlideShow aSlideShow = mCommunicationService.getSlideShow();

        int aCurrentSlideIndex = aSlideShow.getHumanCurrentSlideIndex();
        int aSlidesCount = aSlideShow.getSlidesCount();

        return getString(R.string.mask_slide_show_progress, aCurrentSlideIndex, aSlidesCount);
    }

    private String buildSlideShowTimerProgress() {
        return null;
    }

    @Override
    protected void onResume() {
        super.onResume();

        setUpSlideShowInformation();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu aMenu) {
        getSupportMenuInflater().inflate(getActionBarMenuResourceId(), aMenu);

        return true;
    }

    private int getActionBarMenuResourceId() {
        switch (mMode) {
            case PAGER:
                return R.menu.menu_action_bar_slide_show_pager;

            case GRID:
                return R.menu.menu_action_bar_slide_show_grid;

            default:
                return R.menu.menu_action_bar_slide_show_pager;
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem aMenuItem) {
        switch (aMenuItem.getItemId()) {
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

            case android.R.id.home:
                navigateUp();

                return true;

            default:
                return super.onOptionsItemSelected(aMenuItem);
        }
    }

    private void refreshActionBarMenu() {
        supportInvalidateOptionsMenu();
    }

    private void navigateUp() {
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
    protected void onDestroy() {
        super.onDestroy();

        unbindService();
    }

    private void unbindService() {
        if (!isServiceBound()) {
            return;
        }

        unbindService(this);
    }

    private boolean isServiceBound() {
        return mCommunicationService != null;
    }

    @Override
    public void onServiceDisconnected(ComponentName aComponentName) {
        mCommunicationService = null;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
