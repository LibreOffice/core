/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import android.util.SparseArray;


public class SlideShow {
    private int mSlidesCount;
    private int mCurrentSlideIndex;

    private final SparseArray<byte[]> mSlidePreviewsBytes;
    private final SparseArray<String> mSlideNotes;

    private final Timer mTimer;

    public SlideShow(Timer aTimer) {
        this.mSlidesCount = 0;
        this.mCurrentSlideIndex = 0;

        this.mSlidePreviewsBytes = new SparseArray<byte[]>();
        this.mSlideNotes = new SparseArray<String>();

        this.mTimer = aTimer;
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

    public int getHumanCurrentSlideIndex() {
        return getCurrentSlideIndex() + 1;
    }

    public void setSlidePreviewBytes(int aSlideIndex, byte[] aSlidePreviewBytes) {
        mSlidePreviewsBytes.put(aSlideIndex, aSlidePreviewBytes);
    }

    public byte[] getSlidePreviewBytes(int aSlideIndex) {
        return mSlidePreviewsBytes.get(aSlideIndex);
    }

    public void setSlideNotes(int aSlideIndex, String aSlideNotes) {
        mSlideNotes.put(aSlideIndex, aSlideNotes);
    }

    public String getSlideNotes(int aSlideIndex) {
        String aSlideNotes = mSlideNotes.get(aSlideIndex);

        if (aSlideNotes == null) {
            return "";
        }

        return aSlideNotes;
    }

    public Timer getTimer() {
        return mTimer;
    }

    public void cleanUp() {
        mSlidesCount = 0;
        mCurrentSlideIndex = 0;

        mSlidePreviewsBytes.clear();
        mSlideNotes.clear();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
