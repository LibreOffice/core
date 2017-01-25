/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko.gfx;

import android.graphics.PointF;
import android.view.MotionEvent;
import android.view.View;
import org.libreoffice.LibreOfficeMainActivity;

interface PanZoomController {

    class Factory {
        static PanZoomController create(LibreOfficeMainActivity context, PanZoomTarget target, View view) {
            return new JavaPanZoomController(context, target, view);
        }
    }

    void destroy();

    boolean onTouchEvent(MotionEvent event);
    boolean onMotionEvent(MotionEvent event);
    void notifyDefaultActionPrevented(boolean prevented);

    boolean getRedrawHint();
    PointF getVelocityVector();

    void pageRectUpdated();
    void abortPanning();
    void abortAnimation();

    void setOverScrollMode(int overscrollMode);
    int getOverScrollMode();
}
