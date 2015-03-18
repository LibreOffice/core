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
import android.graphics.RectF;

import org.mozilla.gecko.gfx.RectUtils;

/**
 * This class is responsible to draw the selection handles, track the handle
 * position and perform a hit test to determine if the selection handle was
 * touched.
 */
public class DrawElementHandle {
    public PointF mPosition = new PointF();
    private float mRadius = 20.0f;
    private Paint mGraphicHandleFillPaint = new Paint();
    private Paint mGraphicSelectionPaint = new Paint();
    private Paint mGraphicHandleSelectedFillPaint = new Paint();
    private RectF mHitRect = new RectF();

    public DrawElementHandle(Paint graphicSelectionPaint) {
        mGraphicSelectionPaint = graphicSelectionPaint;

        mGraphicHandleFillPaint.setStyle(Paint.Style.FILL);
        mGraphicHandleFillPaint.setColor(Color.WHITE);

        mGraphicHandleSelectedFillPaint.setStyle(Paint.Style.FILL);
        mGraphicHandleSelectedFillPaint.setColor(Color.BLACK);
    }

    public void draw(Canvas canvas) {
        drawFilledCircle(canvas, mPosition.x, mPosition.y, mRadius, mGraphicSelectionPaint, mGraphicHandleFillPaint);
    }

    public void drawSelected(Canvas canvas) {
        drawFilledCircle(canvas, mPosition.x, mPosition.y, mRadius, mGraphicSelectionPaint, mGraphicHandleSelectedFillPaint);
    }

    private void drawFilledCircle(Canvas canvas, float x, float y, float radius, Paint strokePaint, Paint fillPaint) {
        canvas.drawCircle(x, y, radius, fillPaint);
        canvas.drawCircle(x, y, radius, strokePaint);
    }

    public void reposition(float x, float y) {
        mPosition.x = x;
        mPosition.y = y;
        mHitRect.left = mPosition.x - mRadius * 2;
        mHitRect.right = mPosition.x + mRadius * 2;
        mHitRect.top = mPosition.y - mRadius * 2;
        mHitRect.bottom = mPosition.y + mRadius * 2;
    }

    public boolean contains(float x, float y) {
        return mHitRect.contains(x, y);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */