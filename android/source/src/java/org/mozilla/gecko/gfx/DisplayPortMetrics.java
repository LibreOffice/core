/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko.gfx;

import android.graphics.RectF;

import org.mozilla.gecko.util.FloatUtils;

/*
 * This class keeps track of the area we request Gecko to paint, as well
 * as the resolution of the paint. The area may be different from the visible
 * area of the page, and the resolution may be different from the resolution
 * used in the compositor to render the page. This is so that we can ask Gecko
 * to paint a much larger area without using extra memory, and then render some
 * subsection of that with compositor scaling.
 */
public final class DisplayPortMetrics {
    private final RectF mPosition;
    private final float mResolution;

    public RectF getPosition() {
        return mPosition;
    }

    public float getResolution() {
        return mResolution;
    }

    public DisplayPortMetrics() {
        this(0, 0, 0, 0, 1);
    }

    public DisplayPortMetrics(float left, float top, float right, float bottom, float resolution) {
        mPosition = new RectF(left, top, right, bottom);
        mResolution = resolution;
    }

    public boolean contains(RectF rect) {
        return mPosition.contains(rect);
    }

    public boolean fuzzyEquals(DisplayPortMetrics metrics) {
        return RectUtils.fuzzyEquals(mPosition, metrics.mPosition)
            && FloatUtils.fuzzyEquals(mResolution, metrics.mResolution);
    }

    public String toJSON() {
        StringBuffer sb = new StringBuffer(256);
        sb.append("{ \"left\": ").append(mPosition.left)
          .append(", \"top\": ").append(mPosition.top)
          .append(", \"right\": ").append(mPosition.right)
          .append(", \"bottom\": ").append(mPosition.bottom)
          .append(", \"resolution\": ").append(mResolution)
          .append('}');
        return sb.toString();
    }

    @Override
    public String toString() {
        return "DisplayPortMetrics v=(" + mPosition.left + ","
                + mPosition.top + "," + mPosition.right + ","
                + mPosition.bottom + ") z=" + mResolution;
    }
}
