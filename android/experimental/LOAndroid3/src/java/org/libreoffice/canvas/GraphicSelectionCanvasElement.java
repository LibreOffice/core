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
import android.graphics.Paint;
import android.graphics.PointF;
import android.graphics.RectF;

/**
 * This class is responsible to draw and reposition the selection
 * rectangle.
 */
public class GraphicSelectionCanvasElement {
    private final Paint mGraphicSelectionPaint;
    public RectF mRectangle = new RectF();
    public RectF mScaledRectangle = new RectF();
    private RectF mDrawRectangle = new RectF();
    private DragType mType = DragType.NONE;
    private PointF mStartDragPosition;

    public GraphicSelectionCanvasElement(Paint graphicSelectionPaint) {
        mGraphicSelectionPaint = graphicSelectionPaint;
    }

    public void reposition(RectF scaledRectangle) {
        mScaledRectangle = scaledRectangle;
        mDrawRectangle = scaledRectangle;
    }

    public boolean contains(float x, float y) {
        return mScaledRectangle.contains(x, y);
    }

    public void draw(Canvas canvas) {
        canvas.drawRect(mDrawRectangle, mGraphicSelectionPaint);
    }

    public void dragStart(DragType type, PointF position) {
        mType = type;
        mStartDragPosition = position;
    }

    public void dragging(PointF position) {
        if (mType == DragType.MOVE) {

            float deltaX = position.x - mStartDragPosition.x;
            float deltaY = position.y - mStartDragPosition.y;

            mDrawRectangle = new RectF(mScaledRectangle);
            mDrawRectangle.offset(deltaX, deltaY);
        } else if (mType == DragType.EXTEND) {
            mDrawRectangle = new RectF(mScaledRectangle);
            mDrawRectangle.union(position.x, position.y);
        }
    }

    public void dragEnd() {
        mType = DragType.NONE;
        mDrawRectangle = mScaledRectangle;
    }

    public enum DragType {
        NONE,
        MOVE,
        EXTEND
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */