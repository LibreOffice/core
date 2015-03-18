/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PointF;

public class DrawElementHandle {
    public PointF mPosition = new PointF();
    private float mRadius = 20.0f;
    private Paint mGraphicHandleFillPaint = new Paint();
    private Paint mGraphicSelectionPaint = new Paint();

    public DrawElementHandle(Paint graphicSelectionPaint) {
        mGraphicSelectionPaint = graphicSelectionPaint;

        mGraphicHandleFillPaint.setStyle(Paint.Style.FILL);
        mGraphicHandleFillPaint.setColor(Color.WHITE);
    }

    public void draw(Canvas canvas) {
        canvas.drawCircle(mPosition.x, mPosition.y, mRadius, mGraphicHandleFillPaint);
        canvas.drawCircle(mPosition.x, mPosition.y, mRadius, mGraphicSelectionPaint);
    }

    public void reposition(float x, float y) {
        mPosition.x = x;
        mPosition.y = y;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */