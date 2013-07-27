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
import android.support.v4.view.PagerAdapter;
import android.support.v4.view.ViewPager;
import android.text.Html;
import android.text.TextUtils;
import android.util.DisplayMetrics;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextSwitcher;
import android.widget.TextView;
import android.widget.ViewAnimator;

import com.actionbarsherlock.app.SherlockFragment;
import org.libreoffice.impressremote.communication.SlideShow;
import org.libreoffice.impressremote.util.Intents;
import org.libreoffice.impressremote.R;
import org.libreoffice.impressremote.adapter.SlidesPagerAdapter;
import org.libreoffice.impressremote.communication.CommunicationService;

public class SlidesPagerFragment extends SherlockFragment implements ServiceConnection, ViewPager.OnPageChangeListener {
    private CommunicationService mCommunicationService;
    private BroadcastReceiver mIntentsReceiver;

    public static SlidesPagerFragment newInstance() {
        return new SlidesPagerFragment();
    }

    @Override
    public View onCreateView(LayoutInflater aInflater, ViewGroup aContainer, Bundle aSavedInstanceState) {
        return aInflater.inflate(R.layout.fragment_slides_pager, aContainer, false);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        bindService();
    }

    private void bindService() {
        Intent aServiceIntent = Intents.buildCommunicationServiceIntent(getActivity());
        getActivity().bindService(aServiceIntent, this, Context.BIND_AUTO_CREATE);
    }

    @Override
    public void onServiceConnected(ComponentName aComponentName, IBinder aBinder) {
        CommunicationService.CBinder aServiceBinder = (CommunicationService.CBinder) aBinder;
        mCommunicationService = aServiceBinder.getService();

        setUpSlidesPager();
    }

    private void setUpSlidesPager() {
        ViewPager aSlidesPager = getSlidesPager();

        aSlidesPager.setAdapter(buildSlidesAdapter());
        aSlidesPager.setCurrentItem(mCommunicationService.getSlideShow().getCurrentSlideIndex());
        aSlidesPager.setPageMargin(getSlidesMarginInPx());
        aSlidesPager.setOnPageChangeListener(this);
    }

    private ViewPager getSlidesPager() {
        return (ViewPager) getView().findViewById(R.id.pager_slides);
    }

    private PagerAdapter buildSlidesAdapter() {
        SlideShow aSlideShow = mCommunicationService.getSlideShow();

        return new SlidesPagerAdapter(getActivity(), aSlideShow);
    }

    private int getSlidesMarginInPx() {
        int aPxUnit = TypedValue.COMPLEX_UNIT_PX;
        float aSlideMarginInDp = getResources().getDimension(R.dimen.margin_slide);
        DisplayMetrics aDisplayMetrics = getResources().getDisplayMetrics();

        return (int) TypedValue.applyDimension(aPxUnit, aSlideMarginInDp, aDisplayMetrics);
    }

    @Override
    public void onPageSelected(int aPosition) {
        mCommunicationService.getTransmitter().setCurrentSlide(aPosition);

        setUpSlideNotes(aPosition);
    }

    private void setUpSlideNotes(int aSlideIndex) {
        if (areSlideNotesAvailable(aSlideIndex)) {
            showSlideNotes(aSlideIndex);
        }
        else {
            hideSlideNotes();
        }
    }

    private boolean areSlideNotesAvailable(int aSlideIndex) {
        String aSlideNotes = mCommunicationService.getSlideShow().getSlideNotes(aSlideIndex);

        return !TextUtils.isEmpty(Html.fromHtml(aSlideNotes).toString().trim());
    }

    private void showSlideNotes(int aSlideIndex) {
        ViewAnimator aViewAnimator = (ViewAnimator) getView().findViewById(R.id.view_animator);
        ViewGroup aNotesLayout = (ViewGroup) getView().findViewById(R.id.layout_notes);

        if (aViewAnimator.getDisplayedChild() != aViewAnimator.indexOfChild(aNotesLayout)) {
            aViewAnimator.setDisplayedChild(aViewAnimator.indexOfChild(aNotesLayout));
        }

        setSlideNotes(aSlideIndex);
    }

    private void setSlideNotes(int aSlideIndex) {
        TextSwitcher aSlideNotesTextSwitcher = (TextSwitcher) getView().findViewById(R.id.text_switcher_notes);
        String aSlideNotes = mCommunicationService.getSlideShow().getSlideNotes(aSlideIndex);

        aSlideNotesTextSwitcher.setText(Html.fromHtml(aSlideNotes));
    }

    private void hideSlideNotes() {
        ViewAnimator aViewAnimator = (ViewAnimator) getView().findViewById(R.id.view_animator);
        View aEmptyView = getView().findViewById(R.id.view_empty);

        aViewAnimator.setDisplayedChild(aViewAnimator.indexOfChild(aEmptyView));
    }

    @Override
    public void onPageScrolled(int aPosition, float aPositionOffset, int aPositionOffsetPixels) {
    }

    @Override
    public void onPageScrollStateChanged(int aState) {
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
        private final SlidesPagerFragment mSlidesGridFragment;

        private IntentsReceiver(SlidesPagerFragment aSlidesGridFragment) {
            mSlidesGridFragment = aSlidesGridFragment;
        }

        @Override
        public void onReceive(Context aContext, Intent aIntent) {
            if (Intents.Actions.SLIDE_PREVIEW.equals(aIntent.getAction())) {
                mSlidesGridFragment.refreshSlidesPager();
            }
        }
    }

    private IntentFilter buildIntentsReceiverFilter() {
        IntentFilter aIntentFilter = new IntentFilter();
        aIntentFilter.addAction(Intents.Actions.SLIDE_PREVIEW);

        return aIntentFilter;
    }

    private LocalBroadcastManager getBroadcastManager() {
        Context aContext = getActivity().getApplicationContext();

        return LocalBroadcastManager.getInstance(aContext);
    }

    private void refreshSlidesPager() {
        getSlidesPager().getAdapter().notifyDataSetChanged();
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
