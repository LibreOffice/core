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
public class GraphicSelectionHandle extends CommonCanvasElement {
    /**
     * The factor used to inflate the hit area.
     */
    private final float HIT_AREA_INFLATE_FACTOR = 1.75f;

    private final HandlePosition mHandlePosition;
    public PointF mPosition = new PointF();
    private float mRadius = 20.0f;
    private Paint mStrokePaint = new Paint();
    private Paint mFillPaint = new Paint();
    private Paint mSelectedFillPaint = new Paint();
    private RectF mHitRect = new RectF();
    private boolean mSelected = false;

    /**
     * Construct the handle - set the handle position on the selection.
     * @param position - the handle position on the selection
     */
    public GraphicSelectionHandle(HandlePosition position) {
        mHandlePosition = position;

        mStrokePaint.setStyle(Paint.Style.STROKE);
        mStrokePaint.setColor(Color.GRAY);
        mStrokePaint.setStrokeWidth(3);
        mStrokePaint.setAntiAlias(true);

        mFillPaint.setStyle(Paint.Style.FILL);
        mFillPaint.setColor(Color.WHITE);
        mFillPaint.setAlpha(200);
        mFillPaint.setAntiAlias(true);

        mSelectedFillPaint.setStyle(Paint.Style.FILL);
        mSelectedFillPaint.setColor(Color.GRAY);
        mSelectedFillPaint.setAlpha(200);
        mSelectedFillPaint.setAntiAlias(true);
    }

    /**
     * The position of the handle.
     * @return
     */
    public HandlePosition getHandlePosition() {
        return mHandlePosition;
    }

    /**
     * Draws the handle to the canvas.
     *
     * @see org.libreoffice.canvas.CanvasElement#draw(android.graphics.Canvas)
     */
    @Override
    public void onDraw(Canvas canvas) {
        if (mSelected) {
            drawFilledCircle(canvas, mPosition.x, mPosition.y, mRadius, mStrokePaint, mSelectedFillPaint);
        } else {
            drawFilledCircle(canvas, mPosition.x, mPosition.y, mRadius, mStrokePaint, mFillPaint);
        }
    }

    /**
     * Draw a filled and stroked circle to the canvas.
     */
    private void drawFilledCircle(Canvas canvas, float x, float y, float radius, Paint strokePaint, Paint fillPaint) {
        canvas.drawCircle(x, y, radius, fillPaint);
        canvas.drawCircle(x, y, radius, strokePaint);
    }

    /**
     * Viewport has changed, reposition the handle to the input coordinates.
     */
    public void reposition(float x, float y) {
        mPosition.x = x;
        mPosition.y = y;

        // inflate the radius by HIT_AREA_INFLATE_FACTOR
        float inflatedRadius = mRadius * HIT_AREA_INFLATE_FACTOR;

        // reposition the hit area rectangle
        mHitRect.left = mPosition.x - inflatedRadius;
        mHitRect.right = mPosition.x + inflatedRadius;
        mHitRect.top = mPosition.y - inflatedRadius;
        mHitRect.bottom = mPosition.y + inflatedRadius;
    }

    /**
     * Hit test for the handle.
     * @see org.libreoffice.canvas.CanvasElement#draw(android.graphics.Canvas)
     */
    @Override
    public boolean onHitTest(float x, float y) {
        return mHitRect.contains(x, y);
    }

    /**
     * Mark the handle as selected.
     */
    public void select() {
        mSelected = true;
    }

    /**
     * Reset the selection for the handle.
     */
    public void reset() {
        mSelected = false;
    }

    /**
     * All possible handle positions. The selection rectangle has 8 possible
     * handles.
     */
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