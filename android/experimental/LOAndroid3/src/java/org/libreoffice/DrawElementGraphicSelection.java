package org.libreoffice;

import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.PointF;
import android.graphics.RectF;

/**
 * This class is responsible to draw and reposition the selection
 * rectangle.
 */
public class DrawElementGraphicSelection {
    private final Paint mGraphicSelectionPaint;
    public RectF mRectangle = new RectF();
    public RectF mScaledRectangle = new RectF();
    private RectF mDrawRectangle = new RectF();
    private DragType mType = DragType.NONE;
    private PointF mStartDragPosition;

    public DrawElementGraphicSelection(Paint graphicSelectionPaint) {
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

    enum DragType {
        NONE,
        MOVE,
        EXTEND
    }
}
