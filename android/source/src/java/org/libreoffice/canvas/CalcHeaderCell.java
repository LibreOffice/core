package org.libreoffice.canvas;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Paint.Style;
import android.graphics.RectF;
import android.text.TextPaint;

public class CalcHeaderCell extends CommonCanvasElement {
    private TextPaint mTextPaint = new TextPaint();
    private Paint mBgPaint = new Paint();
    private RectF mBounds;
    private String mText;

    public CalcHeaderCell(float left, float top, float width, float height, String text) {
        mBounds = new RectF(left, top, left + width, top + height);
        mBgPaint.setStyle(Style.STROKE);
        mBgPaint.setColor(Color.GRAY);
        mBgPaint.setAlpha(100);  // hard coded for now
        mTextPaint.setColor(Color.GRAY);
        mTextPaint.setTextSize(24f); // hard coded for now
        mText = text;
    }

    /**
     * Implement hit test here
     *
     * @param x - x coordinate of the
     * @param y - y coordinate of the
     */
    @Override
    public boolean onHitTest(float x, float y) {
        return false;
    }

    /**
     * Called inside draw if the element is visible. Override this method to
     * draw the element on the canvas.
     *
     * @param canvas - the canvas
     */
    @Override
    public void onDraw(Canvas canvas) {
        canvas.drawRect(mBounds, mBgPaint);
        canvas.drawText(mText, mBounds.left, mBounds.bottom, mTextPaint);
    }
}