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

import org.libreoffice.LOKitShell;
import org.mozilla.gecko.gfx.LayerView;

import static org.libreoffice.canvas.GraphicSelectionHandle.HandlePosition;

/**
 * This class is responsible to draw and reposition the selection
 * rectangle.
 */
public class GraphicSelection extends CommonCanvasElement {
    private final Paint mPaintStroke;
    private final Paint mPaintFill;
    public RectF mRectangle = new RectF();
    public RectF mScaledRectangle = new RectF();
    private RectF mDrawRectangle = new RectF();
    private DragType mType = DragType.NONE;
    private PointF mStartDragPosition;

    private GraphicSelectionHandle mHandles[] = new GraphicSelectionHandle[8];
    private GraphicSelectionHandle mDragHandle = null;
    private boolean mTriggerSinglePress = false;

    /**
     * Construct the graphic selection.
     */
    public GraphicSelection() {
        // Create the paint, which is needed at drawing
        mPaintStroke = new Paint();
        mPaintStroke.setStyle(Paint.Style.STROKE);
        mPaintStroke.setColor(Color.GRAY);
        mPaintStroke.setStrokeWidth(2);
        mPaintStroke.setAntiAlias(true);

        mPaintFill = new Paint();
        mPaintFill.setStyle(Paint.Style.FILL);
        mPaintFill.setColor(Color.WHITE);
        mPaintFill.setAlpha(200);
        mPaintFill.setAntiAlias(true);

        // Create the handles of the selection
        mHandles[0] = new GraphicSelectionHandle(HandlePosition.TOP_LEFT);
        mHandles[1] = new GraphicSelectionHandle(HandlePosition.TOP);
        mHandles[2] = new GraphicSelectionHandle(HandlePosition.TOP_RIGHT);
        mHandles[3] = new GraphicSelectionHandle(HandlePosition.LEFT);
        mHandles[4] = new GraphicSelectionHandle(HandlePosition.RIGHT);
        mHandles[5] = new GraphicSelectionHandle(HandlePosition.BOTTOM_LEFT);
        mHandles[6] = new GraphicSelectionHandle(HandlePosition.BOTTOM);
        mHandles[7] = new GraphicSelectionHandle(HandlePosition.BOTTOM_RIGHT);
    }

    /**
     * Viewport has changed, reposition the selection to the new rectangle.
     * @param scaledRectangle - rectangle of selection position on the document
     */
    public void reposition(RectF scaledRectangle) {
        mScaledRectangle = scaledRectangle;
        mDrawRectangle = scaledRectangle; // rectangle that will be draw

        // reposition the handles too
        mHandles[0].reposition(scaledRectangle.left, scaledRectangle.top);
        mHandles[1].reposition(scaledRectangle.centerX(), scaledRectangle.top);
        mHandles[2].reposition(scaledRectangle.right, scaledRectangle.top);
        mHandles[3].reposition(scaledRectangle.left, scaledRectangle.centerY());
        mHandles[4].reposition(scaledRectangle.right, scaledRectangle.centerY());
        mHandles[5].reposition(scaledRectangle.left, scaledRectangle.bottom);
        mHandles[6].reposition(scaledRectangle.centerX(), scaledRectangle.bottom);
        mHandles[7].reposition(scaledRectangle.right, scaledRectangle.bottom);
    }

    /**
     * Hit test for the selection.
     * @see org.libreoffice.canvas.CanvasElement#draw(android.graphics.Canvas)
     */
    @Override
    public boolean onHitTest(float x, float y) {
        // Check if handle was hit
        for (GraphicSelectionHandle handle : mHandles) {
            if (handle.contains(x, y)) {
                return true;
            }
        }
        return mScaledRectangle.contains(x, y);
    }

    /**
     * Draw the selection on the canvas.
     * @see org.libreoffice.canvas.CanvasElement#draw(android.graphics.Canvas)
     */
    @Override
    public void onDraw(Canvas canvas) {
        canvas.drawRect(mDrawRectangle, mPaintStroke);
        if (mType != DragType.NONE) {
            canvas.drawRect(mDrawRectangle, mPaintFill);
        }
        for (GraphicSelectionHandle handle : mHandles) {
            handle.draw(canvas);
        }
    }

    /**
     * Dragging on the screen has started.
     * @param position - position where the dragging started
     */
    public void dragStart(PointF position) {
        mDragHandle = null;
        mType = DragType.NONE;
        for (GraphicSelectionHandle handle : mHandles) {
            if (handle.contains(position.x, position.y)) {
                mDragHandle = handle;
                mDragHandle.select();
                mType = DragType.EXTEND;
                sendGraphicSelectionStart(handle.mPosition);
            }
        }
        if (mDragHandle == null) {
            mType = DragType.MOVE;
            sendGraphicSelectionStart(position);
        }
        mStartDragPosition = position;
        mTriggerSinglePress = true;
    }

    /**
     * Dragging is in process.
     * @param position - position of the drag
     */
    public void dragging(PointF position) {
        if (mType == DragType.MOVE) {
            float deltaX = position.x - mStartDragPosition.x;
            float deltaY = position.y - mStartDragPosition.y;

            mDrawRectangle = new RectF(mScaledRectangle);
            mDrawRectangle.offset(deltaX, deltaY);
        } else if (mType == DragType.EXTEND) {
            adaptDrawRectangle(position.x, position.y);
        }
        mTriggerSinglePress = false;
    }

    /**
     * Dragging has ended.
     * @param position - last position of the drag
     */
    public void dragEnd(PointF position) {
        PointF point = new PointF();
        if (mDragHandle != null) {
            point.x = mDragHandle.mPosition.x;
            point.y = mDragHandle.mPosition.y;
            mDragHandle.reset();
            mDragHandle = null;
        } else {
            point.x = mStartDragPosition.x;
            point.y = mStartDragPosition.y;
        }
        float deltaX = position.x - mStartDragPosition.x;
        float deltaY = position.y - mStartDragPosition.y;
        point.offset(deltaX, deltaY);

        sendGraphicSelectionEnd(point);

        if (mTriggerSinglePress && mDragHandle == null) {
            onSinglePress(point);
            mTriggerSinglePress = false;
        }

        mDrawRectangle = mScaledRectangle;
        mType = DragType.NONE;
    }

    /**
     * Adapt the selection depending on which handle was dragged.
     */
    private void adaptDrawRectangle(float x, float y) {
        mDrawRectangle = new RectF(mScaledRectangle);
        switch(mDragHandle.getHandlePosition()) {
            case TOP_LEFT:
                mDrawRectangle.left = x;
                mDrawRectangle.top = y;
                break;
            case TOP:
                mDrawRectangle.top = y;
                break;
            case TOP_RIGHT:
                mDrawRectangle.right = x;
                mDrawRectangle.top = y;
                break;
            case LEFT:
                mDrawRectangle.left = x;
                break;
            case RIGHT:
                mDrawRectangle.right = x;
                break;
            case BOTTOM_LEFT:
                mDrawRectangle.left = x;
                mDrawRectangle.bottom = y;
                break;
            case BOTTOM:
                mDrawRectangle.bottom = y;
                break;
            case BOTTOM_RIGHT:
                mDrawRectangle.right = x;
                mDrawRectangle.bottom = y;
                break;
        }
    }

    /**
     * Send graphic selection start event to LOKitTread.
     * @param screenPosition - screen position of the selection
     */
    private void sendGraphicSelectionStart(PointF screenPosition) {
        sendGraphicSelection("GraphicSelectionStart", screenPosition);
    }

    /**
     * Send graphic selection end event to LOKitTread.
     * @param screenPosition - screen position of the selection
     */
    private void sendGraphicSelectionEnd(PointF screenPosition) {
        sendGraphicSelection("GraphicSelectionEnd", screenPosition);
    }

    /**
     * Send graphic selection event to LOKitTread.
     * @param type - type of the graphic selection
     * @param screenPosition - screen position of the selection
     */
    private void sendGraphicSelection(String type, PointF screenPosition)
    {
        LayerView layerView = LOKitShell.getLayerView();
        if (layerView != null) {
            // Position is in screen coordinates. We need to convert them to
            // document coordinates.
            PointF documentPoint = layerView.getLayerClient().convertViewPointToLayerPoint(screenPosition);
            LOKitShell.sendTouchEvent(type, documentPoint);
        }
    }

    /**
     * When a single press (no dragging happened) was performed.
     */
    private void onSinglePress(PointF screenPosition) {
        sendGraphicSelection("LongPress", screenPosition);
    }

    /**
     * Set the visibility of the graphic selection.
     */
    @Override
    public void setVisible(boolean visible) {
        super.setVisible(visible);
        for (GraphicSelectionHandle handle: mHandles) {
            handle.setVisible(visible);
        }
    }

    /**
     * Reset the selection.
     */
    public void reset() {
        mDragHandle = null;
        for (GraphicSelectionHandle handle : mHandles) {
            handle.reset();
        }
    }

    /**
     * Type of the selection dragging.
     */
    public enum DragType {
        NONE,
        MOVE,
        EXTEND
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
