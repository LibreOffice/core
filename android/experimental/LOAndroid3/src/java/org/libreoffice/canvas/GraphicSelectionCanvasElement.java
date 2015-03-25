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

import org.libreoffice.LOKitShell;
import org.mozilla.gecko.gfx.LayerView;

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

    private GraphicSelectionHandleCanvasElement mHandles[] = new GraphicSelectionHandleCanvasElement[8];
    private GraphicSelectionHandleCanvasElement mDragHandle = null;

    public GraphicSelectionCanvasElement(Paint graphicSelectionPaint) {
        mGraphicSelectionPaint = graphicSelectionPaint;

        mHandles[0] = new GraphicSelectionHandleCanvasElement(mGraphicSelectionPaint);
        mHandles[1] = new GraphicSelectionHandleCanvasElement(mGraphicSelectionPaint);
        mHandles[2] = new GraphicSelectionHandleCanvasElement(mGraphicSelectionPaint);
        mHandles[3] = new GraphicSelectionHandleCanvasElement(mGraphicSelectionPaint);
        mHandles[4] = new GraphicSelectionHandleCanvasElement(mGraphicSelectionPaint);
        mHandles[5] = new GraphicSelectionHandleCanvasElement(mGraphicSelectionPaint);
        mHandles[6] = new GraphicSelectionHandleCanvasElement(mGraphicSelectionPaint);
        mHandles[7] = new GraphicSelectionHandleCanvasElement(mGraphicSelectionPaint);
    }

    public void reposition(RectF scaledRectangle) {
        mScaledRectangle = scaledRectangle;
        mDrawRectangle = scaledRectangle;

        mHandles[0].reposition(scaledRectangle.left, scaledRectangle.top);
        mHandles[1].reposition(scaledRectangle.centerX(), scaledRectangle.top);
        mHandles[2].reposition(scaledRectangle.right, scaledRectangle.top);
        mHandles[3].reposition(scaledRectangle.left, scaledRectangle.centerY());
        mHandles[4].reposition(scaledRectangle.right, scaledRectangle.centerY());
        mHandles[5].reposition(scaledRectangle.left, scaledRectangle.bottom);
        mHandles[6].reposition(scaledRectangle.centerX(), scaledRectangle.bottom);
        mHandles[7].reposition(scaledRectangle.right, scaledRectangle.bottom);
    }

    public boolean contains(float x, float y) {
        // Check if handle was hit
        for (GraphicSelectionHandleCanvasElement handle : mHandles) {
            if (handle.contains(x, y)) {
                return true;
            }
        }
        return mScaledRectangle.contains(x, y);
    }

    public void draw(Canvas canvas) {
        canvas.drawRect(mDrawRectangle, mGraphicSelectionPaint);
        for (GraphicSelectionHandleCanvasElement handle : mHandles) {
            if (mType == DragType.MOVE || mType == DragType.EXTEND) {
                handle.drawSelected(canvas);
            } else {
                handle.draw(canvas);
            }
        }
    }

    public void dragStart(PointF position) {
        mDragHandle = null;
        mType = DragType.NONE;
        for (GraphicSelectionHandleCanvasElement handle : mHandles) {
            if (handle.contains(position.x, position.y)) {
                mDragHandle = handle;
                mType = DragType.EXTEND;
                sendGraphicSelectionStart(handle.mPosition);
            }
        }
        if (mDragHandle == null) {
            mType = DragType.MOVE;
            sendGraphicSelectionStart(position);
        }

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

    public void dragEnd(PointF position) {
        PointF point = new PointF();
        if (mDragHandle != null) {
            point.x = mDragHandle.mPosition.x;
            point.y = mDragHandle.mPosition.y;
        } else {
            point.x = mStartDragPosition.x;
            point.y = mStartDragPosition.y;
        }
        float deltaX = position.x - mStartDragPosition.x;
        float deltaY = position.y - mStartDragPosition.y;
        point.offset(deltaX, deltaY);

        sendGraphicSelectionEnd(point);

        mDrawRectangle = mScaledRectangle;
        mDragHandle = null;
        mType = DragType.NONE;
    }

    private void sendGraphicSelectionStart(PointF screenPosition) {
        LayerView layerView = LOKitShell.getLayerView();
        if (layerView != null) {
            PointF documentPoint = layerView.getLayerClient().convertViewPointToLayerPoint(screenPosition);
            LOKitShell.sendTouchEvent("GraphicSelectionStart", documentPoint);
        }
    }

    private void sendGraphicSelectionEnd(PointF screenPosition) {
        LayerView layerView = LOKitShell.getLayerView();
        if (layerView != null) {
            PointF documentPoint = layerView.getLayerClient().convertViewPointToLayerPoint(screenPosition);
            LOKitShell.sendTouchEvent("GraphicSelectionEnd", documentPoint);
        }
    }

    public enum DragType {
        NONE,
        MOVE,
        EXTEND
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */