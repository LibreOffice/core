/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko.gfx;

import android.graphics.RectF;

import org.mozilla.gecko.util.FloatUtils;

/*
 * This class keeps track of the area we request Gecko to paint.
 * The area may be different from the visible
 * area of the page. This is so that we can ask Gecko
 * to paint a much larger area without using extra memory, and then render some
 * subsection of that.
 */
public final class DisplayPortMetrics {
    private final RectF mPosition;

    public RectF getPosition() {
        return mPosition;
    }

    public DisplayPortMetrics() {
        this(0, 0, 0, 0);
    }

    public DisplayPortMetrics(float left, float top, float right, float bottom) {
        mPosition = new RectF(left, top, right, bottom);
    }

    public boolean contains(RectF rect) {
        return mPosition.contains(rect);
    }

    @Override
    public String toString() {
        return "DisplayPortMetrics v=(" + mPosition.left + ","
                + mPosition.top + "," + mPosition.right + ","
                + mPosition.bottom + ")";
    }
}
