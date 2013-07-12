/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote;

import org.libreoffice.impressremote.communication.CommunicationService;
import org.libreoffice.impressremote.communication.SlideShow;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.graphics.Typeface;
import android.os.Bundle;
import android.support.v4.content.LocalBroadcastManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.ImageView.ScaleType;
import android.widget.TextView;

import com.actionbarsherlock.app.SherlockFragment;

public class ThumbnailFragment extends SherlockFragment {

    private CommunicationService mCommunicationService;

    private GridView mGrid;
    private ImageView mCurrentImage;
    private TextView mCurrentText;

    private SlideShow mSlideShow;
    private Context mContext;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                    Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        //        container.removeAllViews();
        setRetainInstance(true);
        View v = inflater
                        .inflate(R.layout.fragment_thumbnail, container, false);

        mGrid = (GridView) v.findViewById(R.id.thumbnail_grid);

        mGrid.setOnItemClickListener(new ClickListener());
        mContext = getActivity().getApplicationContext();

        if (mCommunicationService != null && mSlideShow != null) {
            mGrid.setAdapter(new ThumbnailAdapter(mContext, mSlideShow));
        }

        IntentFilter aFilter = new IntentFilter(
                        Intents.Actions.SLIDE_CHANGED);
        aFilter.addAction(Intents.Actions.SLIDE_PREVIEW);
        LocalBroadcastManager
                        .getInstance(getActivity().getApplicationContext())
                        .registerReceiver(mListener, aFilter);

        return v;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        LocalBroadcastManager
                        .getInstance(getActivity().getApplicationContext())
                        .unregisterReceiver(mListener);
        mGrid = null;
        mContext = null;
        mCurrentImage = null;
        mCurrentText = null;
    }

    private void setSelected(int position) {
        formatUnselected(mCurrentImage, mCurrentText);

        if (mGrid == null) {
            return;
        }

        View aV = mGrid.getChildAt(position);
        if (aV != null) {
            mCurrentImage = (ImageView) aV.findViewById(R.id.sub_thumbnail);
            mCurrentText = (TextView) aV.findViewById(R.id.sub_number);

            formatSelected(mCurrentImage, mCurrentText);
        }
    }

    private void formatUnselected(ImageView aImage, TextView aText) {
        if (aImage != null) {
            aImage.setBackgroundColor(getResources().getColor(
                            R.color.thumbnail_border));
        }
        if (aText != null) {
            aText.setTypeface(Typeface.create(aText.getTypeface(),
                            Typeface.NORMAL));
        }
    }

    private void formatSelected(ImageView aImage, TextView aText) {
        if (aImage != null) {
            aImage.setBackgroundColor(getResources().getColor(
                            R.color.thumbnail_border_selected));
        }
        if (aText != null) {
            aText.setTypeface(Typeface.create(aText.getTypeface(),
                            Typeface.BOLD));
        }
    }

    // ----------------------------------------------------- CLICK LISTENER ----
    protected class ClickListener implements AdapterView.OnItemClickListener {
        public void onItemClick(AdapterView<?> parent, View v, int position,
                        long id) {
            if (mCommunicationService != null)
                mCommunicationService.getTransmitter().setCurrentSlide(position);
        }
    }

    // ---------------------------------------------------- MESSAGE HANDLER ----

    public void setCommunicationService(
                    CommunicationService aCommunicationService) {
        mCommunicationService = aCommunicationService;
        mSlideShow = mCommunicationService.getSlideShow();
        if (mGrid != null) {
            mGrid.setAdapter(new ThumbnailAdapter(mContext, mSlideShow));
        }
    }

    private BroadcastReceiver mListener = new BroadcastReceiver() {

        @Override
        public void onReceive(Context aContext, Intent aIntent) {
            if (mGrid == null)
                return;
            if (aIntent.getAction().equals(
                            Intents.Actions.SLIDE_CHANGED)) {
                int aSlide = aIntent.getExtras().getInt("slide_number");
                setSelected(aSlide);
            } else if (aIntent.getAction().equals(
                            Intents.Actions.SLIDE_PREVIEW)) {
                mGrid.invalidateViews();
            }

        }
    };

    // ------------------------------------------------- THUMBNAIL ADAPTER ----
    protected class ThumbnailAdapter extends BaseAdapter {

        private Context mContext;

        private SlideShow mSlideShow;

        public ThumbnailAdapter(Context aContext, SlideShow aSlideShow) {
            mContext = aContext;
            mSlideShow = aSlideShow;
        }

        @Override
        public int getCount() {
            return mSlideShow.getSlidesCount();
        }

        @Override
        public Object getItem(int arg0) {
            return null;
        }

        @Override
        public long getItemId(int position) {
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            LayoutInflater aInflater = (LayoutInflater) mContext
                            .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            View v = aInflater.inflate(R.layout.slide_thumbnail, null);

            ImageView aImage = (ImageView) v.findViewById(R.id.sub_thumbnail);
            TextView aText = (TextView) v.findViewById(R.id.sub_number);

            // Do the image & number styling
            int aBorderWidth = getResources().getInteger(
                            R.integer.thumbnail_border_width);
            aImage.setPadding(aBorderWidth, aBorderWidth, aBorderWidth,
                            aBorderWidth);

            if ((mSlideShow != null)
                            && (position == mSlideShow.getCurrentSlideIndex())) {
                formatSelected(aImage, aText);
                mCurrentImage = aImage;
                mCurrentText = aText;
            } else {
                formatUnselected(aImage, aText);
            }

            Bitmap aBitmap = mSlideShow.getSlidePreview(position);
            // Width
            int aWidth = (mGrid.getWidth()) / 3 - 20;
            aImage.setMaxWidth(aWidth);
            aImage.setScaleType(ScaleType.FIT_CENTER);

            if (aBitmap != null) {
                aImage.setImageBitmap(aBitmap);
            }

            aText.setText(String.valueOf(position + 1));

            return v;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
