/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko.gfx;

import android.graphics.PointF;
import android.os.Handler;

class SubdocumentScrollHelper {
    private static final String LOGTAG = "GeckoSubdocumentScrollHelper";

    private final Handler mUiHandler;

    /* This is the amount of displacement we have accepted but not yet sent to JS; this is
     * only valid when mOverrideScrollPending is true. */
    private final PointF mPendingDisplacement;

    /* When this is true, we're sending scroll events to JS to scroll the active subdocument. */
    private boolean mOverridePanning;

    /* When this is true, we have received an ack for the last scroll event we sent to JS, and
     * are ready to send the next scroll event. Note we only ever have one scroll event inflight
     * at a time. */
    private boolean mOverrideScrollAck;

    /* When this is true, we have a pending scroll that we need to send to JS; we were unable
     * to send it when it was initially requested because mOverrideScrollAck was not true. */
    private boolean mOverrideScrollPending;

    /* When this is true, the last scroll event we sent actually did some amount of scrolling on
     * the subdocument; we use this to decide when we have reached the end of the subdocument. */
    private boolean mScrollSucceeded;

    SubdocumentScrollHelper() {
        // mUiHandler will be bound to the UI thread since that's where this constructor runs
        mUiHandler = new Handler();
        mPendingDisplacement = new PointF();
    }

    void destroy() {
    }

    boolean scrollBy(PointF displacement) {
        if (! mOverridePanning) {
            return false;
        }

        if (! mOverrideScrollAck) {
            mOverrideScrollPending = true;
            mPendingDisplacement.x += displacement.x;
            mPendingDisplacement.y += displacement.y;
            return true;
        }

        mOverrideScrollAck = false;
        mOverrideScrollPending = false;
        // clear the |mPendingDisplacement| after serializing |displacement| to
        // JSON because they might be the same object
        mPendingDisplacement.x = 0;
        mPendingDisplacement.y = 0;

        return true;
    }

    void cancel() {
        mOverridePanning = false;
    }

    boolean scrolling() {
        return mOverridePanning;
    }

    boolean lastScrollSucceeded() {
        return mScrollSucceeded;
    }
}
