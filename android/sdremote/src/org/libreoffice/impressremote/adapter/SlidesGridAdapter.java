/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import org.libreoffice.impressremote.R;
import org.libreoffice.impressremote.communication.SlideShow;

public class SlidesGridAdapter extends BaseAdapter {
    private final LayoutInflater mLayoutInflater;

    private final SlideShow mSlideShow;

    public SlidesGridAdapter(Context aContext, SlideShow aSlideShow) {
        mLayoutInflater = buildLayoutInflater(aContext);

        mSlideShow = aSlideShow;
    }

    private LayoutInflater buildLayoutInflater(Context aContext) {
        return (LayoutInflater) aContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    @Override
    public int getCount() {
        return mSlideShow.getSlidesCount();
    }

    @Override
    public Object getItem(int aPosition) {
        return mSlideShow.getSlidePreview(aPosition);
    }

    @Override
    public long getItemId(int aPosition) {
        return aPosition;
    }

    @Override
    public View getView(int aPosition, View aConvertView, ViewGroup aViewGroup) {
        View aSlideView = getView(aConvertView, aViewGroup);
        ViewHolder aSlideViewHolder = getViewHolder(aSlideView);

        if (isSlidePreviewAvailable(aPosition)) {
            aSlideViewHolder.aSlidePreview.setImageBitmap(mSlideShow.getSlidePreview(aPosition));
        } else {
            aSlideViewHolder.aSlidePreview.setImageResource(
                R.drawable.slide_unknown);
        }

        aSlideViewHolder.aSlideIndex.setText(buildSlideIndex(aPosition));

        return aSlideView;
    }

    private View getView(View aConvertView, ViewGroup aViewGroup) {
        if (aConvertView != null) {
            return aConvertView;
        }

        return mLayoutInflater.inflate(R.layout.view_grid_slide, aViewGroup, false);
    }

    private ViewHolder getViewHolder(View aView) {
        if (aView.getTag() != null) {
            return (ViewHolder) aView.getTag();
        }

        return buildViewHolder(aView);
    }

    private static final class ViewHolder {
        public ImageView aSlidePreview;
        public TextView aSlideIndex;
    }

    private ViewHolder buildViewHolder(View aView) {
        ViewHolder aViewHolder = new ViewHolder();

        aViewHolder.aSlidePreview = (ImageView) aView.findViewById(R.id.image_slide_preview);
        aViewHolder.aSlideIndex = (TextView) aView.findViewById(R.id.text_slide_index);

        return aViewHolder;
    }

    private boolean isSlidePreviewAvailable(int aSlideIndex) {
        return mSlideShow.getSlidePreview(aSlideIndex) != null;
    }

    private String buildSlideIndex(int aPosition) {
        int aHumanSlideIndex = aPosition + 1;

        return Integer.toString(aHumanSlideIndex);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
