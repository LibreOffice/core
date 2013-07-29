/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import java.util.concurrent.TimeUnit;

import android.os.Handler;

public class Timer implements Runnable {
    public interface TimerListener {
        public void onTimerUpdated();
    }

    private static final long UPDATE_PERIOD_IN_MINUTES = 1;

    private final Handler mTimerHandler;
    private final TimerListener mTimerListener;

    private int mTotalMinutes;
    private int mPassedMinutes;

    public Timer(TimerListener aTimerListener) {
        mTimerHandler = new Handler();
        mTimerListener = aTimerListener;

        mTotalMinutes = 0;
        mPassedMinutes = 0;
    }

    public void setMinutesLength(int aLengthInMinutes) {
        mTotalMinutes = aLengthInMinutes;
    }

    public int getMinutesLength() {
        return mTotalMinutes;
    }

    public boolean isSet() {
        return mTotalMinutes != 0;
    }

    public void start() {
        if (!isSet()) {
            return;
        }

        mTimerHandler.postDelayed(this, TimeUnit.MINUTES.toMillis(UPDATE_PERIOD_IN_MINUTES));
    }

    @Override
    public void run() {
        updatePassedMinutes();

        mTimerListener.onTimerUpdated();

        start();
    }

    private void updatePassedMinutes() {
        mPassedMinutes++;
    }

    public void pause() {
        stop();
    }

    public void resume() {
        start();
    }

    public void stop() {
        mTimerHandler.removeCallbacks(this);
    }

    public boolean isTimeUp() {
        return getMinutesLeft() <= 0;
    }

    public int getMinutesLeft() {
        return mTotalMinutes - mPassedMinutes;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
