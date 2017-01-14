/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko.gfx;

import android.graphics.PointF;
import android.view.MotionEvent;
import android.view.View;

public interface PanZoomController {

    static class Factory {
        static PanZoomController create(PanZoomTarget target, View view) {
            return new JavaPanZoomController(target, view);
        }
    }

    public void destroy();

    public boolean onTouchEvent(MotionEvent event);
    public boolean onMotionEvent(MotionEvent event);
    public void notifyDefaultActionPrevented(boolean prevented);

    public boolean getRedrawHint();
    public PointF getVelocityVector();

    public void pageRectUpdated();
    public void abortPanning();
    public void abortAnimation();

    public void setOverScrollMode(int overscrollMode);
    public int getOverScrollMode();
}
