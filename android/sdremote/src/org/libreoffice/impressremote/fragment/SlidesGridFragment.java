/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.fragment;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.content.LocalBroadcastManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.GridView;

import com.actionbarsherlock.app.SherlockFragment;
import org.libreoffice.impressremote.activity.SlideShowActivity;
import org.libreoffice.impressremote.communication.SlideShow;
import org.libreoffice.impressremote.util.Intents;
import org.libreoffice.impressremote.R;
import org.libreoffice.impressremote.adapter.SlidesGridAdapter;
import org.libreoffice.impressremote.communication.CommunicationService;

public class SlidesGridFragment extends SherlockFragment implements ServiceConnection, AdapterView.OnItemClickListener {
    private CommunicationService mCommunicationService;
    private BroadcastReceiver mIntentsReceiver;

    public static SlidesGridFragment newInstance() {
        return new SlidesGridFragment();
    }

    @Override
    public View onCreateView(LayoutInflater aInflater, ViewGroup aContainer, Bundle aSavedInstanceState) {
        return aInflater.inflate(R.layout.fragment_slides_grid, aContainer, false);
    }

    @Override
    public void onActivityCreated(Bundle aSavedInstanceState) {
        super.onActivityCreated(aSavedInstanceState);

        bindService();
    }

    private void bindService() {
        Intent aServiceIntent = Intents.buildCommunicationServiceIntent(getActivity());
        getActivity().bindService(aServiceIntent, this, Context.BIND_AUTO_CREATE);
    }

    @Override
    public void onServiceConnected(ComponentName aComponentName, IBinder aBinder) {
        CommunicationService.ServiceBinder aServiceBinder = (CommunicationService.ServiceBinder) aBinder;
        mCommunicationService = aServiceBinder.getService();

        setUpSlidesGrid();
    }

    private void setUpSlidesGrid() {
        if (!isAdded()) {
            return;
        }

        GridView aSlidesGrid = getSlidesGrid();

        aSlidesGrid.setAdapter(buildSlidesAdapter());
        aSlidesGrid.setOnItemClickListener(this);
    }

    private GridView getSlidesGrid() {
        return (GridView) getView().findViewById(R.id.grid_slides);
    }

    private SlidesGridAdapter buildSlidesAdapter() {
        SlideShow aSlideShow = mCommunicationService.getSlideShow();

        return new SlidesGridAdapter(getActivity(), aSlideShow);
    }

    @Override
    public void onItemClick(AdapterView<?> aAdapterView, View aView, int aPosition, long aId) {
        changeCurrentSlide(aPosition);
        changeSlideShowMode();
    }

    private void changeCurrentSlide(int aSlideIndex) {
        mCommunicationService.getCommandsTransmitter().setCurrentSlide(aSlideIndex);
    }

    private void changeSlideShowMode() {
        Intent aIntent = Intents.buildSlideShowModeChangedIntent(SlideShowActivity.Mode.PAGER);
        getBroadcastManager().sendBroadcast(aIntent);
    }

    private LocalBroadcastManager getBroadcastManager() {
        Context aContext = getActivity().getApplicationContext();

        return LocalBroadcastManager.getInstance(aContext);
    }

    @Override
    public void onServiceDisconnected(ComponentName aComponentName) {
        mCommunicationService = null;
    }

    @Override
    public void onResume() {
        super.onResume();

        registerIntentsReceiver();
    }

    private void registerIntentsReceiver() {
        mIntentsReceiver = new IntentsReceiver(this);
        IntentFilter aIntentFilter = buildIntentsReceiverFilter();

        getBroadcastManager().registerReceiver(mIntentsReceiver, aIntentFilter);
    }

    private static final class IntentsReceiver extends BroadcastReceiver {
        private final SlidesGridFragment mSlidesGridFragment;

        private IntentsReceiver(SlidesGridFragment aSlidesGridFragment) {
            mSlidesGridFragment = aSlidesGridFragment;
        }

        @Override
        public void onReceive(Context aContext, Intent aIntent) {
            if (Intents.Actions.SLIDE_SHOW_RUNNING.equals(aIntent.getAction())) {
                mSlidesGridFragment.refreshSlidesGrid();
                return;
            }

            if (Intents.Actions.SLIDE_PREVIEW.equals(aIntent.getAction())) {
                int aSlideIndex = aIntent.getIntExtra(Intents.Extras.SLIDE_INDEX, 0);

                mSlidesGridFragment.refreshSlidePreview(aSlideIndex);
            }
        }
    }

    private IntentFilter buildIntentsReceiverFilter() {
        IntentFilter aIntentFilter = new IntentFilter();
        aIntentFilter.addAction(Intents.Actions.SLIDE_SHOW_RUNNING);
        aIntentFilter.addAction(Intents.Actions.SLIDE_PREVIEW);

        return aIntentFilter;
    }

    private void refreshSlidesGrid() {
        getSlidesGrid().invalidateViews();
    }

    private void refreshSlidePreview(int aSlideIndex) {
        GridView aSlidesGrid = getSlidesGrid();
        View aSlideView = aSlidesGrid.getChildAt(aSlideIndex);

        if (aSlideView == null) {
            return;
        }

        aSlidesGrid.getAdapter().getView(aSlideIndex, aSlideView, aSlidesGrid);
    }

    @Override
    public void onPause() {
        super.onPause();

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
    public void onDestroy() {
        super.onDestroy();

        unbindService();
    }

    private void unbindService() {
        if (!isServiceBound()) {
            return;
        }

        getActivity().unbindService(this);
    }

    private boolean isServiceBound() {
        return mCommunicationService != null;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
