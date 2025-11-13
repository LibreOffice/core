/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko.gfx;

import android.view.View;

import org.mozilla.gecko.util.FloatUtils;

/**
 * This class represents the physics for one axis of movement (i.e. either
 * horizontal or vertical). It tracks the different properties of movement
 * like displacement, velocity, viewport dimensions, etc. pertaining to
 * a particular axis.
 */

abstract class Axis {

    // This fraction of velocity remains after every animation frame when the velocity is low.
    private static final float FRICTION_SLOW = getFrameAdjustedFriction(0.85f);
    // This fraction of velocity remains after every animation frame when the velocity is high.
    private static final float FRICTION_FAST = getFrameAdjustedFriction(0.97f);
    // Below this velocity (in pixels per frame), the friction starts increasing from FRICTION_FAST
    // to FRICTION_SLOW.

    static final float MS_PER_FRAME = 4.0f;
    private static final float FRAMERATE_MULTIPLIER = (1000f/60f) / MS_PER_FRAME;
    private static final float VELOCITY_THRESHOLD = 10 / FRAMERATE_MULTIPLIER;
    // The maximum velocity change factor between events, per ms, in %.
    // Direction changes are excluded.
    private static final float MAX_EVENT_ACCELERATION = 0.012f;

    // The rate of deceleration when the surface has overscrolled.
    private static final float OVERSCROLL_DECEL_RATE = getFrameAdjustedFriction(0.04f);
    // The percentage of the surface which can be overscrolled before it must snap back.
    private static final float SNAP_LIMIT = 0.3f;

    // The minimum amount of space that must be present for an axis to be considered scrollable,
    // in pixels.
    private static final float MIN_SCROLLABLE_DISTANCE = 0.5f;


    //  The values we use for friction are based on a 16.6ms frame, adjust them to MS_PER_FRAME:
    //  FRICTION^1 = FRICTION_ADJUSTED^(16/MS_PER_FRAME)
    //  FRICTION_ADJUSTED = e ^ ((ln(FRICTION))/FRAMERATE_MULTIPLIER)
    static float getFrameAdjustedFriction(float baseFriction) {
        return (float)Math.pow(Math.E, (Math.log(baseFriction) / FRAMERATE_MULTIPLIER));
    }

    private enum FlingStates {
        STOPPED,
        PANNING,
        FLINGING,
    }

    private enum Overscroll {
        NONE,
        MINUS,      // Overscrolled in the negative direction
        PLUS,       // Overscrolled in the positive direction
        BOTH,       // Overscrolled in both directions (page is zoomed to smaller than screen)
    }

    private int mOverscrollMode; /* Default to only overscrolling if we're allowed to scroll in a direction */
    private float mFirstTouchPos;           /* Position of the first touch event on the current drag. */
    private float mTouchPos;                /* Position of the most recent touch event on the current drag. */
    private float mLastTouchPos;            /* Position of the touch event before touchPos. */
    private float mVelocity;                /* Velocity in this direction; pixels per animation frame. */
    private boolean mScrollingDisabled;     /* Whether movement on this axis is locked. */
    private boolean mDisableSnap;           /* Whether overscroll snapping is disabled. */
    private float mDisplacement;

    private FlingStates mFlingState;        /* The fling state we're in on this axis. */

    protected abstract float getOrigin();
    protected abstract float getViewportLength();
    protected abstract float getPageStart();
    protected abstract float getPageLength();

    Axis() {
        mOverscrollMode = View.OVER_SCROLL_IF_CONTENT_SCROLLS;
    }

    public int getOverScrollMode() {
        return mOverscrollMode;
    }

    private float getViewportEnd() {
        return getOrigin() + getViewportLength();
    }

    private float getPageEnd() {
        return getPageStart() + getPageLength();
    }

    void startTouch(float pos) {
        mVelocity = 0.0f;
        mScrollingDisabled = false;
        mFirstTouchPos = mTouchPos = mLastTouchPos = pos;
    }

    float panDistance(float currentPos) {
        return currentPos - mFirstTouchPos;
    }

    void setScrollingDisabled(boolean disabled) {
        mScrollingDisabled = disabled;
    }

    void saveTouchPos() {
        mLastTouchPos = mTouchPos;
    }

    void updateWithTouchAt(float pos, float timeDelta) {
        float newVelocity = (mTouchPos - pos) / timeDelta * MS_PER_FRAME;

        // If there's a direction change, or current velocity is very low,
        // allow setting of the velocity outright. Otherwise, use the current
        // velocity and a maximum change factor to set the new velocity.
        boolean curVelocityIsLow = Math.abs(mVelocity) < 1.0f / FRAMERATE_MULTIPLIER;
        boolean directionChange = (mVelocity > 0) != (newVelocity > 0);
        if (curVelocityIsLow || (directionChange && !FloatUtils.fuzzyEquals(newVelocity, 0.0f))) {
            mVelocity = newVelocity;
        } else {
            float maxChange = Math.abs(mVelocity * timeDelta * MAX_EVENT_ACCELERATION);
            mVelocity = Math.min(mVelocity + maxChange, Math.max(mVelocity - maxChange, newVelocity));
        }

        mTouchPos = pos;
    }

    boolean overscrolled() {
        return getOverscroll() != Overscroll.NONE;
    }

    private Overscroll getOverscroll() {
        boolean minus = (getOrigin() < getPageStart());
        boolean plus = (getViewportEnd() > getPageEnd());
        if (minus && plus) {
            return Overscroll.BOTH;
        } else if (minus) {
            return Overscroll.MINUS;
        } else if (plus) {
            return Overscroll.PLUS;
        } else {
            return Overscroll.NONE;
        }
    }

    // Returns the amount that the page has been overscrolled. If the page hasn't been
    // overscrolled on this axis, returns 0.
    private float getExcess() {
        switch (getOverscroll()) {
        case MINUS:     return getPageStart() - getOrigin();
        case PLUS:      return getViewportEnd() - getPageEnd();
        case BOTH:      return (getViewportEnd() - getPageEnd()) + (getPageStart() - getOrigin());
        default:        return 0.0f;
        }
    }

    /*
     * Returns true if the page is zoomed in to some degree along this axis such that scrolling is
     * possible and this axis has not been scroll locked while panning. Otherwise, returns false.
     */
    boolean scrollable() {
        // if we are axis locked, return false
        if (mScrollingDisabled) {
            return false;
        }

        // there is scrollable space, and we're not disabled, or the document fits the viewport
        // but we always allow overscroll anyway
        return getViewportLength() <= getPageLength() - MIN_SCROLLABLE_DISTANCE ||
               getOverScrollMode() == View.OVER_SCROLL_ALWAYS;
    }

    /*
     * Returns the resistance, as a multiplier, that should be taken into account when
     * tracking or pinching.
     */
    float getEdgeResistance(boolean forPinching) {
        float excess = getExcess();
        if (excess > 0.0f && (getOverscroll() == Overscroll.BOTH || !forPinching)) {
            // excess can be greater than viewport length, but the resistance
            // must never drop below 0.0
            return Math.max(0.0f, SNAP_LIMIT - excess / getViewportLength());
        }
        return 1.0f;
    }

    /* Returns the velocity. If the axis is locked, returns 0. */
    float getRealVelocity() {
        return scrollable() ? mVelocity : 0f;
    }

    void startPan() {
        mFlingState = FlingStates.PANNING;
    }

    void startFling(boolean stopped) {
        mDisableSnap = false;

        if (stopped) {
            mFlingState = FlingStates.STOPPED;
        } else {
            mFlingState = FlingStates.FLINGING;
        }
    }

    /* Advances a fling animation by one step. */
    boolean advanceFling() {
        if (mFlingState != FlingStates.FLINGING) {
            return false;
        }

        float excess = getExcess();
        Overscroll overscroll = getOverscroll();
        boolean decreasingOverscroll = false;
        if ((overscroll == Overscroll.MINUS && mVelocity > 0) ||
            (overscroll == Overscroll.PLUS && mVelocity < 0))
        {
            decreasingOverscroll = true;
        }

        if (mDisableSnap || FloatUtils.fuzzyEquals(excess, 0.0f) || decreasingOverscroll) {
            // If we aren't overscrolled, just apply friction.
            if (Math.abs(mVelocity) >= VELOCITY_THRESHOLD) {
                mVelocity *= FRICTION_FAST;
            } else {
                float t = mVelocity / VELOCITY_THRESHOLD;
                mVelocity *= FloatUtils.interpolate(FRICTION_SLOW, FRICTION_FAST, t);
            }
        } else {
            // Otherwise, decrease the velocity linearly.
            float elasticity = 1.0f - excess / (getViewportLength() * SNAP_LIMIT);
            if (overscroll == Overscroll.MINUS) {
                mVelocity = Math.min((mVelocity + OVERSCROLL_DECEL_RATE) * elasticity, 0.0f);
            } else { // must be Overscroll.PLUS
                mVelocity = Math.max((mVelocity - OVERSCROLL_DECEL_RATE) * elasticity, 0.0f);
            }
        }

        return true;
    }

    void stopFling() {
        mVelocity = 0.0f;
        mFlingState = FlingStates.STOPPED;
    }

    // Performs displacement of the viewport position according to the current velocity.
    void displace() {
        // if this isn't scrollable just return
        if (!scrollable())
            return;

        if (mFlingState == FlingStates.PANNING)
            mDisplacement += (mLastTouchPos - mTouchPos) * getEdgeResistance(false);
        else
            mDisplacement += mVelocity;

        // if overscroll is disabled and we're trying to overscroll, reset the displacement
        // to remove any excess. Using getExcess alone isn't enough here since it relies on
        // getOverscroll which doesn't take into account any new displacement being applied
        if (getOverScrollMode() == View.OVER_SCROLL_NEVER) {
            if (mDisplacement + getOrigin() < getPageStart()) {
                mDisplacement = getPageStart() - getOrigin();
                stopFling();
            } else if (mDisplacement + getViewportEnd() > getPageEnd()) {
                mDisplacement = getPageEnd() - getViewportEnd();
                stopFling();
            }
        }
    }

    float resetDisplacement() {
        float d = mDisplacement;
        mDisplacement = 0.0f;
        return d;
    }
}
