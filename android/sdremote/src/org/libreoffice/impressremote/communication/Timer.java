/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

public class Timer {
    /**
     * This stores the starting time of the timer if running.
     * <p/>
     * If paused this stores how long the timer was previously running.
     */
    private long mTime;
    private long mCountdownTime;

    private boolean mIsRunning;
    private boolean mIsCountdown;

    public Timer() {
        mTime = 0;
        mCountdownTime = 0;

        mIsRunning = false;
        mIsCountdown = false;
    }

    /**
     * Set whether this timer should be a normal or a countdown timer.
     *
     * @param aIsCountdown Whether this should be a countdown timer.
     */
    public void setCountdown(boolean aIsCountdown) {
        mIsCountdown = aIsCountdown;

        if (mIsRunning) {
            reset();
        }
    }

    public boolean isCountdown() {
        return mIsCountdown;
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

    public boolean isRunning() {
        return mIsRunning;
    }

    /**
     * Reset the timer, and stop it it was running.
     */
    public void reset() {
        mIsRunning = false;
        mTime = 0;
    }

    public void start() {
        if (mIsRunning) {
            return;
        }

        mTime = System.currentTimeMillis() - mTime;
        mIsRunning = true;
    }

    public void stop() {
        if (!mIsRunning)
            return;

        mTime = System.currentTimeMillis() - mTime;
        mIsRunning = false;
    }

    /**
     * Get either how long this timer has been running, or how long the
     * timer still has left to run.
     *
     * @return running time in millis.
     */
    public long getTimeMillis() {
        if (mIsCountdown) {
            return calculateCountdownRunningTime();
        }

        return calculateRunningTime();
    }

    private long calculateCountdownRunningTime() {
        long aRunningTime = mCountdownTime - calculateRunningTime();

        if (aRunningTime < 0) {
            reset();
        }

        return aRunningTime;
    }

    private long calculateRunningTime() {
        if (mIsRunning) {
            return System.currentTimeMillis() - mTime;
        }

        return mTime;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
