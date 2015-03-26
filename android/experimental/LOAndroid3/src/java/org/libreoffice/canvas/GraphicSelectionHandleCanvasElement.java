/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.canvas;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PointF;
import android.graphics.RectF;

/**
 * This class is responsible to draw the selection handles, track the handle
 * position and perform a hit test to determine if the selection handle was
 * touched.
 */
public class GraphicSelectionHandleCanvasElement {
    private final HandlePosition mHandlePosition;
    public PointF mPosition = new PointF();
    private float mRadius = 20.0f;
    private Paint mStrokePaint = new Paint();
    private Paint mFillPaint = new Paint();
    private Paint mSelectedFillPaint = new Paint();
    private RectF mHitRect = new RectF();
    private boolean mSelected = false;

    public GraphicSelectionHandleCanvasElement(HandlePosition position) {
        mHandlePosition = position;

        mStrokePaint.setStyle(Paint.Style.STROKE);
        mStrokePaint.setColor(Color.BLACK);
        mStrokePaint.setStrokeWidth(3);

        mFillPaint.setStyle(Paint.Style.FILL);
        mFillPaint.setColor(Color.WHITE);

        mSelectedFillPaint.setStyle(Paint.Style.FILL);
        mSelectedFillPaint.setColor(Color.BLUE);
    }

    public HandlePosition getHandlePosition() {
        return mHandlePosition;
    }

    public void draw(Canvas canvas) {
        if (mSelected) {
            drawFilledCircle(canvas, mPosition.x, mPosition.y, mRadius, mStrokePaint, mSelectedFillPaint);
        } else {
            drawFilledCircle(canvas, mPosition.x, mPosition.y, mRadius, mStrokePaint, mFillPaint);
        }
    }

    private void drawFilledCircle(Canvas canvas, float x, float y, float radius, Paint strokePaint, Paint fillPaint) {
        canvas.drawCircle(x, y, radius, fillPaint);
        canvas.drawCircle(x, y, radius, strokePaint);
    }

    public void reposition(float x, float y) {
        mPosition.x = x;
        mPosition.y = y;
        mHitRect.left = mPosition.x - mRadius * 1.75f;
        mHitRect.right = mPosition.x + mRadius * 1.75f;
        mHitRect.top = mPosition.y - mRadius * 1.75f;
        mHitRect.bottom = mPosition.y + mRadius * 1.75f;
    }

    public boolean contains(float x, float y) {
        return mHitRect.contains(x, y);
    }

    public void select() {
        mSelected = true;
    }

    public void reset() {
        mSelected = false;
    }

    public enum HandlePosition {
        TOP_LEFT,
        TOP,
        TOP_RIGHT,
        RIGHT,
        BOTTOM_RIGHT,
        BOTTOM,
        BOTTOM_LEFT,
        LEFT
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */