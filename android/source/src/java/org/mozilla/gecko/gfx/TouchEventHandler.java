/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko.gfx;

import android.content.Context;
import android.os.SystemClock;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.View;

import java.util.LinkedList;
import java.util.Queue;

/**
 * This class handles incoming touch events from the user and sends them to
 * listeners in Gecko and/or performs the "default action" (asynchronous pan/zoom
 * behaviour. EVERYTHING IN THIS CLASS MUST RUN ON THE UI THREAD.
 *
 * In the following code/comments, a "block" of events refers to a contiguous
 * sequence of events that starts with a DOWN or POINTER_DOWN and goes up to
 * but not including the next DOWN or POINTER_DOWN event.
 *
 * "Dispatching" an event refers to performing the default actions for the event,
 * which at our level of abstraction just means sending it off to the gesture
 * detectors and the pan/zoom controller.
 *
 * If an event is "default-prevented" that means one or more listeners in Gecko
 * has called preventDefault() on the event, which means that the default action
 * for that event should not occur. Usually we care about a "block" of events being
 * default-prevented, which means that the DOWN/POINTER_DOWN event that started
 * the block, or the first MOVE event following that, were prevent-defaulted.
 *
 * A "default-prevented notification" is when we here in Java-land receive a notification
 * from gecko as to whether or not a block of events was default-prevented. This happens
 * at some point after the first or second event in the block is processed in Gecko.
 * This code assumes we get EXACTLY ONE default-prevented notification for each block
 * of events.
 *
 * Note that even if all events are default-prevented, we still send specific types
 * of notifications to the pan/zoom controller. The notifications are needed
 * to respond to user actions a timely manner regardless of default-prevention,
 * and fix issues like bug 749384.
 */
public final class TouchEventHandler {
    private static final String LOGTAG = "GeckoTouchEventHandler";

    private final GestureDetector mGestureDetector;
    private final SimpleScaleGestureDetector mScaleGestureDetector;
    private final JavaPanZoomController mPanZoomController;

    TouchEventHandler(Context context, JavaPanZoomController panZoomController) {
        mPanZoomController = panZoomController;
        mGestureDetector = new GestureDetector(context, mPanZoomController);
        mScaleGestureDetector = new SimpleScaleGestureDetector(mPanZoomController);

        mGestureDetector.setOnDoubleTapListener(mPanZoomController);
    }

    void destroy() {
    }

    /* This function MUST be called on the UI thread */
    public boolean handleEvent(MotionEvent event) {
        if (isDownEvent(event)) {
            // this is the start of a new block of events! whee!
            mPanZoomController.startingNewEventBlock(event, false);
        }

        dispatchEvent(event);

        return true;
    }

    private boolean isDownEvent(MotionEvent event) {
        int action = (event.getAction() & MotionEvent.ACTION_MASK);
        return (action == MotionEvent.ACTION_DOWN || action == MotionEvent.ACTION_POINTER_DOWN);
    }

    /**
     * Dispatch the event to the gesture detectors and the pan/zoom controller.
     */
    private void dispatchEvent(MotionEvent event) {
        if (mGestureDetector.onTouchEvent(event)) {
            return;
        }
        mScaleGestureDetector.onTouchEvent(event);
        if (mScaleGestureDetector.isInProgress()) {
            return;
        }
        mPanZoomController.handleEvent(event);
    }
}
