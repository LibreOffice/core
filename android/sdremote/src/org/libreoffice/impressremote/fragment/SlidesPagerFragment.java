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
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ScrollView;
import android.widget.TextSwitcher;

import com.actionbarsherlock.app.SherlockFragment;
import org.libreoffice.impressremote.communication.SlideShow;
import org.libreoffice.impressremote.util.Intents;
import org.libreoffice.impressremote.R;
import org.libreoffice.impressremote.adapter.SlidesPagerAdapter;
import org.libreoffice.impressremote.communication.CommunicationService;

public class SlidesPagerFragment extends SherlockFragment implements ServiceConnection, ViewPager.OnPageChangeListener, View.OnClickListener {
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
        CommunicationService.CBinder aServiceBinder = (CommunicationService.CBinder) aBinder;
        mCommunicationService = aServiceBinder.getService();

        setUpSlidesPager();
    }

    private void setUpSlidesPager() {
        if (!isServiceBound()) {
            return;
        }

        if (!isAdded()) {
            return;
        }

        ViewPager aSlidesPager = getSlidesPager();

        aSlidesPager.setAdapter(buildSlidesAdapter());
        aSlidesPager.setPageMargin(getSlidesMargin());
        aSlidesPager.setOnPageChangeListener(this);

        setUpCurrentSlide();
        setUpCurrentSlideNotes();
    }

    private ViewPager getSlidesPager() {
        return (ViewPager) getView().findViewById(R.id.pager_slides);
    }

    private PagerAdapter buildSlidesAdapter() {
        SlideShow aSlideShow = mCommunicationService.getSlideShow();

        return new SlidesPagerAdapter(getActivity(), aSlideShow, this);
    }

    @Override
    public void onClick(View aView) {
        if (!isLastSlideDisplayed()) {
            showNextTransition();
        }
    }

    private boolean isLastSlideDisplayed() {
        int aCurrentSlideIndex = mCommunicationService.getSlideShow().getHumanCurrentSlideIndex();
        int aSlidesCount = mCommunicationService.getSlideShow().getSlidesCount();

        return aCurrentSlideIndex == aSlidesCount;
    }

    private void showNextTransition() {
        mCommunicationService.getTransmitter().performNextTransition();
    }

    private int getSlidesMargin() {
        return getResources().getDimensionPixelSize(R.dimen.margin_slide);
    }

    private void setUpCurrentSlide() {
        SlideShow aSlideShow = mCommunicationService.getSlideShow();

        getSlidesPager().setCurrentItem(aSlideShow.getCurrentSlideIndex());
    }

    private void setUpCurrentSlideNotes() {
        SlideShow aSlideShow = mCommunicationService.getSlideShow();

        setUpSlideNotes(aSlideShow.getCurrentSlideIndex());
    }

    @Override
    public void onPageSelected(int aPosition) {
        mCommunicationService.getTransmitter().setCurrentSlide(aPosition);

        setUpSlideNotes(aPosition);
    }

    private void setUpSlideNotes(int aSlideIndex) {
        if (!isSlideNotesLayoutAvailable()) {
            return;
        }

        if (areSlideNotesAvailable(aSlideIndex)) {
            showSlideNotes(aSlideIndex);
            scrollSlideNotes();
        }
        else {
            hideSlideNotes();
        }
    }

    private boolean isSlideNotesLayoutAvailable() {
        ViewGroup aSlideNotesLayout = (ViewGroup) getView().findViewById(R.id.layout_notes);

        return aSlideNotesLayout != null;
    }

    private boolean areSlideNotesAvailable(int aSlideIndex) {
        String aSlideNotes = mCommunicationService.getSlideShow().getSlideNotes(aSlideIndex);

        return !TextUtils.isEmpty(Html.fromHtml(aSlideNotes).toString().trim());
    }

    private void showSlideNotes(int aSlideIndex) {
        TextSwitcher aSlideNotesSwitcher = getSlideNotesSwitcher();
        String aSlideNotes = mCommunicationService.getSlideShow().getSlideNotes(aSlideIndex);

        aSlideNotesSwitcher.setText(Html.fromHtml(aSlideNotes));
    }

    private TextSwitcher getSlideNotesSwitcher() {
        return (TextSwitcher) getView().findViewById(R.id.text_switcher_notes);
    }

    private void scrollSlideNotes() {
        ScrollView aSlideNotesScroll = (ScrollView) getView().findViewById(R.id.scroll_notes);

        aSlideNotesScroll.scrollTo(0, 0);
    }

    private void hideSlideNotes() {
        TextSwitcher aSlideNotesSwitcher = getSlideNotesSwitcher();

        aSlideNotesSwitcher.setText(getString(R.string.message_notes_empty));
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
        private final SlidesPagerFragment mSlidesPagerFragment;

        private IntentsReceiver(SlidesPagerFragment aSlidesGridFragment) {
            mSlidesPagerFragment = aSlidesGridFragment;
        }

        @Override
        public void onReceive(Context aContext, Intent aIntent) {
            if (Intents.Actions.SLIDE_SHOW_RUNNING.equals(aIntent.getAction())) {
                mSlidesPagerFragment.setUpSlidesPager();
                return;
            }

            if (Intents.Actions.SLIDE_SHOW_STOPPED.equals(aIntent.getAction())) {
                mSlidesPagerFragment.setUpSlidesPager();
                return;
            }

            if (Intents.Actions.SLIDE_CHANGED.equals(aIntent.getAction())) {
                mSlidesPagerFragment.setUpCurrentSlide();
                return;
            }

            if (Intents.Actions.SLIDE_PREVIEW.equals(aIntent.getAction())) {
                int aSlideIndex = aIntent.getIntExtra(Intents.Extras.SLIDE_INDEX, 0);

                mSlidesPagerFragment.refreshSlide(aSlideIndex);
            }
        }
    }

    private IntentFilter buildIntentsReceiverFilter() {
        IntentFilter aIntentFilter = new IntentFilter();
        aIntentFilter.addAction(Intents.Actions.SLIDE_SHOW_RUNNING);
        aIntentFilter.addAction(Intents.Actions.SLIDE_SHOW_STOPPED);
        aIntentFilter.addAction(Intents.Actions.SLIDE_CHANGED);
        aIntentFilter.addAction(Intents.Actions.SLIDE_PREVIEW);

        return aIntentFilter;
    }

    private LocalBroadcastManager getBroadcastManager() {
        Context aContext = getActivity().getApplicationContext();

        return LocalBroadcastManager.getInstance(aContext);
    }

    private void refreshSlide(int aSlideIndex) {
        // Refresh only loaded slides to avoid images blinking on large slides count.
        // There is no way to invalidate only a certain slide.

        int aCurrentSlideIndex = mCommunicationService.getSlideShow().getCurrentSlideIndex();

        if (aSlideIndex == aCurrentSlideIndex) {
            refreshSlidesPager();
            return;
        }

        int aSlidesOffscreenCount = getSlidesPager().getOffscreenPageLimit();

        if (aSlideIndex < aCurrentSlideIndex - aSlidesOffscreenCount) {
            return;
        }

        if (aSlideIndex > aCurrentSlideIndex + aSlidesOffscreenCount) {
            return;
        }

        refreshSlidesPager();
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
