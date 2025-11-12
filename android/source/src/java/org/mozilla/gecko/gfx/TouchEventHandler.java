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

    // The time limit for listeners to respond with preventDefault on touchevents
    // before we begin panning the page
    private static final int EVENT_LISTENER_TIMEOUT = 200;

    private final View mView;
    private final GestureDetector mGestureDetector;
    private final SimpleScaleGestureDetector mScaleGestureDetector;
    private final JavaPanZoomController mPanZoomController;

    private final ListenerTimeoutProcessor mListenerTimeoutProcessor;

    // this next variable requires some explanation. strap yourself in.
    //
    // for each block of events, we do two things: (1) send the events to gecko and expect
    // exactly one default-prevented notification in return, and (2) kick off a delayed
    // ListenerTimeoutProcessor that triggers in case we don't hear from the listener in
    // a timely fashion.
    // since events are constantly coming in, we need to be able to handle more than one
    // block of events in the queue.
    //
    // this means that there are ordering restrictions on these that we can take advantage of,
    // and need to abide by. blocks of events in the queue will always be in the order that
    // the user generated them. default-prevented notifications we get from gecko will be in
    // the same order as the blocks of events in the queue. the ListenerTimeoutProcessors that
    // have been posted will also fire in the same order as the blocks of events in the queue.
    // HOWEVER, we may get multiple default-prevented notifications interleaved with multiple
    // ListenerTimeoutProcessor firings, and that interleaving is not predictable.
    //
    // therefore, we need to make sure that for each block of events, we process the queued
    // events exactly once, either when we get the default-prevented notification, or when the
    // timeout expires (whichever happens first). there is no way to associate the
    // default-prevented notification with a particular block of events other than via ordering,
    //
    // so what we do to accomplish this is to track a "processing balance", which is the number
    // of default-prevented notifications that we have received, minus the number of ListenerTimeoutProcessors
    // that have fired. (think "balance" as in teeter-totter balance). this value is:
    // - zero when we are in a state where the next default-prevented notification we expect
    //   to receive and the next ListenerTimeoutProcessor we expect to fire both correspond to
    //   the next block of events in the queue.
    // - positive when we are in a state where we have received more default-prevented notifications
    //   than ListenerTimeoutProcessors. This means that the next default-prevented notification
    //   does correspond to the block at the head of the queue, but the next n ListenerTimeoutProcessors
    //   need to be ignored as they are for blocks we have already processed. (n is the absolute value
    //   of the balance.)
    // - negative when we are in a state where we have received more ListenerTimeoutProcessors than
    //   default-prevented notifications. This means that the next ListenerTimeoutProcessor that
    //   we receive does correspond to the block at the head of the queue, but the next n
    //   default-prevented notifications need to be ignored as they are for blocks we have already
    //   processed. (n is the absolute value of the balance.)
    private int mProcessingBalance;

    TouchEventHandler(Context context, View view, JavaPanZoomController panZoomController) {
        mView = view;

        mPanZoomController = panZoomController;
        mGestureDetector = new GestureDetector(context, mPanZoomController);
        mScaleGestureDetector = new SimpleScaleGestureDetector(mPanZoomController);
        mListenerTimeoutProcessor = new ListenerTimeoutProcessor();

        mGestureDetector.setOnDoubleTapListener(mPanZoomController);
    }

    void destroy() {
    }

    /* This function MUST be called on the UI thread */
    public boolean handleEvent(MotionEvent event) {
        if (isDownEvent(event)) {
            // this is the start of a new block of events! whee!
            mPanZoomController.startingNewEventBlock(event, false);

            // set the timeout so that we dispatch these events and update mProcessingBalance
            // if we don't get a default-prevented notification
            mView.postDelayed(mListenerTimeoutProcessor, EVENT_LISTENER_TIMEOUT);
        }

        dispatchEvent(event);

        return true;
    }

    private boolean isDownEvent(MotionEvent event) {
        int action = (event.getAction() & MotionEvent.ACTION_MASK);
        return (action == MotionEvent.ACTION_DOWN || action == MotionEvent.ACTION_POINTER_DOWN);
    }

    private boolean touchFinished(MotionEvent event) {
        int action = (event.getAction() & MotionEvent.ACTION_MASK);
        return (action == MotionEvent.ACTION_UP || action == MotionEvent.ACTION_CANCEL);
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

    private void processEventBlock() {
        // no more logic here
    }

    private class ListenerTimeoutProcessor implements Runnable {
        /* This MUST be run on the UI thread */
        public void run() {
            if (mProcessingBalance < 0) {
                // gecko already responded with default-prevented notification, and so
                // the block of events this ListenerTimeoutProcessor corresponds to have
                // already been removed from the queue.
            } else {
                processEventBlock();
            }
            mProcessingBalance++;
        }
    }
}
