/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko;

public final class ZoomConstraints {
    private final float mDefaultZoom;
    private final float mMinZoom;
    private final float mMaxZoom;

    public ZoomConstraints(float defaultZoom, float minZoom, float maxZoom) {
        mDefaultZoom = defaultZoom;
        mMinZoom = minZoom;
        mMaxZoom = maxZoom;
    }

    public final float getDefaultZoom() {
        return mDefaultZoom;
    }

    public final float getMinZoom() {
        return mMinZoom;
    }

    public final float getMaxZoom() {
        return mMaxZoom;
    }
}
