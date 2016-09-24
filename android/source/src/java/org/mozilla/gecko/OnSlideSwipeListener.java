/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.mozilla.gecko;

import android.content.Context;
import android.view.GestureDetector;
import android.view.GestureDetector.SimpleOnGestureListener;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.util.Log;

import org.libreoffice.LOKitShell;
import org.mozilla.gecko.gfx.GeckoLayerClient;
import org.mozilla.gecko.gfx.ImmutableViewportMetrics;


public class OnSlideSwipeListener implements OnTouchListener {
    private static String LOGTAG = OnSlideSwipeListener.class.getName();

    private final GestureDetector mGestureDetector;
    private GeckoLayerClient mLayerClient;

    public OnSlideSwipeListener(Context ctx, GeckoLayerClient client){
        mGestureDetector = new GestureDetector(ctx, new GestureListener());
        mLayerClient = client;
    }

    private final class GestureListener extends SimpleOnGestureListener {

        private static final int SWIPE_THRESHOLD = 100;
        private static final int SWIPE_VELOCITY_THRESHOLD = 100;

        @Override
        public boolean onDown(MotionEvent e) {
            return false;
        }

        @Override
        public boolean onFling(MotionEvent e1, MotionEvent e2, float velX, float velY) {
            // Check if the page is already zoomed-in.
            // Disable swiping gesture if that's the case.
            ImmutableViewportMetrics viewportMetrics = mLayerClient.getViewportMetrics();
            if (viewportMetrics.viewportRectLeft > viewportMetrics.pageRectLeft ||
                    viewportMetrics.viewportRectRight < viewportMetrics.pageRectRight) {
                return false;
            }

            // Otherwise, the page is smaller than viewport, perform swipe
            // gesture.
            try {
                float diffY = e2.getY() - e1.getY();
                float diffX = e2.getX() - e1.getX();
                if (Math.abs(diffX) > Math.abs(diffY)) {
                    if (Math.abs(diffX) > SWIPE_THRESHOLD
                            && Math.abs(velX) > SWIPE_VELOCITY_THRESHOLD) {
                        if (diffX > 0) {
                            onSwipeRight();
                        } else {
                            onSwipeLeft();
                        }
                    }
                }
            } catch (Exception exception) {
                exception.printStackTrace();
            }
            return false;
        }
    }

    public void onSwipeRight() {
        Log.d(LOGTAG, "onSwipeRight");
        LOKitShell.sendSwipeRightEvent();
    }

    public void onSwipeLeft() {
        Log.d(LOGTAG, "onSwipeLeft");
        LOKitShell.sendSwipeLeftEvent();
    }

    @Override
    public boolean onTouch(View v, MotionEvent me) {
        return mGestureDetector.onTouchEvent(me);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
