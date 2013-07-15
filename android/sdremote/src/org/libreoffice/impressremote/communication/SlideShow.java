/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.SparseArray;


public class SlideShow {
    private int mSlidesCount;
    private int mCurrentSlideIndex;

    private final SparseArray<Bitmap> mSlidePreviews;
    private final SparseArray<String> mSlideNotes;

    private final Timer mTimer;

    public SlideShow() {
        this.mSlidesCount = 0;
        this.mCurrentSlideIndex = 0;

        this.mSlidePreviews = new SparseArray<Bitmap>();
        this.mSlideNotes = new SparseArray<String>();

        this.mTimer = new Timer();
    }

    public void setSlidesCount(int aSlidesCount) {
        mSlidesCount = aSlidesCount;
    }

    public int getSlidesCount() {
        return mSlidesCount;
    }

    public void setCurrentSlideIndex(int aCurrentSlideIndex) {
        mCurrentSlideIndex = aCurrentSlideIndex;
    }

    public int getCurrentSlideIndex() {
        return mCurrentSlideIndex;
    }

    public void setSlidePreview(int aSlideIndex, byte[] aSlidePreviewBytes) {
        Bitmap aSlidePreview = BitmapFactory
            .decodeByteArray(aSlidePreviewBytes, 0, aSlidePreviewBytes.length);

        mSlidePreviews.put(aSlideIndex, aSlidePreview);
    }

    public Bitmap getSlidePreview(int aSlideIndex) {
        return mSlidePreviews.get(aSlideIndex);
    }

    public void setSlideNotes(int aSlideIndex, String aSlideNotes) {
        mSlideNotes.put(aSlideIndex, aSlideNotes);
    }

    public String getSlideNotes(int aSlideIndex) {
        String aSlideNotes = mSlideNotes.get(aSlideIndex);

        if (aSlideNotes != null) {
            return aSlideNotes;
        } else {
            return "";
        }
    }

    public Timer getTimer() {
        return mTimer;
    }

    public void cleanUp() {
        mSlidesCount = 0;
        mCurrentSlideIndex = 0;

        mSlidePreviews.clear();
        mSlideNotes.clear();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
