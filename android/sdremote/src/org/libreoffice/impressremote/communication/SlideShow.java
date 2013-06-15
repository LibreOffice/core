/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.RectF;
import android.util.SparseArray;

import org.libreoffice.impressremote.R;


public class SlideShow {
    private final Context mContext;

    private int mSlidesCount;
    private int mCurrentSlideIndex;

    private final SparseArray<Bitmap> mSlidePreviews;
    private final SparseArray<String> mSlideNotes;

    public SlideShow(Context aContext) {
        this.mContext = aContext;

        this.mSlidesCount = 0;
        this.mCurrentSlideIndex = 0;

        this.mSlidePreviews = new SparseArray<Bitmap>();
        this.mSlideNotes = new SparseArray<String>();
    }

    public void setSlidesCount(int aSlidesCount) {
        this.mSlidesCount = aSlidesCount;
    }

    public int getSlidesCount() {
        return mSlidesCount;
    }

    public void setCurrentSlideIndex(int aCurrentSlideIndex) {
        this.mCurrentSlideIndex = aCurrentSlideIndex;
    }

    public int getCurrentSlideIndex() {
        return mCurrentSlideIndex;
    }

    public void setSlidePreview(int aSlideIndex, byte[] aSlidePreview) {
        Bitmap aBitmap = BitmapFactory
            .decodeByteArray(aSlidePreview, 0, aSlidePreview.length);
        final int borderWidth = 8;

        Paint p = new Paint(Paint.ANTI_ALIAS_FLAG);
        p.setShadowLayer(borderWidth, 0, 0, Color.BLACK);

        RectF aRect = new RectF(borderWidth, borderWidth, borderWidth
            + aBitmap.getWidth(), borderWidth
            + aBitmap.getHeight());
        Bitmap aOut = Bitmap.createBitmap(aBitmap.getWidth() + 2
            * borderWidth, aBitmap.getHeight() + 2
            * borderWidth, aBitmap.getConfig());
        Canvas canvas = new Canvas(aOut);
        canvas.drawColor(mContext.getResources().getColor(R.color.light_grey));
        canvas.drawRect(aRect, p);
        canvas.drawBitmap(aBitmap, null, aRect, null);

        mSlidePreviews.put(aSlideIndex, aOut);
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

    // ---------------------------------------------------- TIMER --------------
    private Timer mTimer = new Timer();

    public Timer getTimer() {
        return mTimer;
    }

    public class Timer {
        /**
         * This stores the starting time of the timer if running.
         * <p/>
         * If paused this stores how long the timer was previously running.
         */
        private long aTime = 0;

        private long mCountdownTime = 0;

        private boolean mIsRunning = false;

        private boolean mIsCountdown = false;

        /**
         * Set whether this timer should be a normal or a countdown timer.
         *
         * @param aIsCountdown Whether this should be a countdown timer.
         */
        public void setCountdown(boolean aIsCountdown) {
            mIsCountdown = aIsCountdown;
            if (mIsRunning) {
                mIsRunning = false;
                aTime = 0;
            }
        }

        /**
         * Set the countdown time. Can be set, and isn't lost, whatever mode
         * the timer is running in.
         *
         * @param aCountdownTime The countdown time.
         */
        public void setCountdownTime(long aCountdownTime) {
            mCountdownTime = aCountdownTime;
        }

        public long getCountdownTime() {
            return mCountdownTime;
        }

        public boolean isCountdown() {
            return mIsCountdown;
        }

        public boolean isRunning() {
            return mIsRunning;
        }

        /**
         * Reset the timer, and stop it it was running.
         */
        public void reset() {
            mIsRunning = false;
            aTime = 0;
        }

        public void startTimer() {
            if (mIsRunning)
                return;

            aTime = System.currentTimeMillis() - aTime;
            mIsRunning = true;
        }

        public void stopTimer() {
            if (!mIsRunning)
                return;

            aTime = System.currentTimeMillis() - aTime;
            mIsRunning = false;
        }

        /**
         * Get either how long this timer has been running, or how long the
         * timer still has left to run.
         *
         * @return
         */
        public long getTimeMillis() {
            long aTimeRunning;
            // Determine how long we've been going.
            if (mIsRunning) {
                aTimeRunning = System.currentTimeMillis() - aTime;
            } else {
                aTimeRunning = aTime;
            }
            // And give the time going, or time left
            if (!mIsCountdown) {
                return aTimeRunning;
            } else {
                long aRet = mCountdownTime - aTimeRunning;
                if (aRet < 0) { // We have completed!
                    mIsRunning = false;
                    aRet = 0;
                }
                return aRet;
            }

        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
