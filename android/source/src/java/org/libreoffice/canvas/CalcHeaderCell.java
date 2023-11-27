package org.libreoffice.canvas;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Paint.Style;
import android.graphics.Rect;
import android.graphics.RectF;
import android.text.TextPaint;

public class CalcHeaderCell extends CommonCanvasElement {
    private final TextPaint mTextPaint = new TextPaint();

    private final Paint mFramePaint = new Paint();
    private final Paint mBgPaint = new Paint();
    private final RectF mBounds;
    private final Rect mTextBounds = new Rect();
    private final String mText;

    public CalcHeaderCell(float left, float top, float width, float height, String text, boolean selected) {
        mBounds = new RectF(left, top, left + width, top + height);

        mFramePaint.setStyle(Style.STROKE);
        mFramePaint.setColor(Color.BLACK);

        mBgPaint.setStyle(Style.FILL);
        mBgPaint.setColor(Color.GRAY);
        // draw background more intensely when cell is selected
        if (selected) {
            mBgPaint.setAlpha(100);
        } else {
            mBgPaint.setAlpha(25);
        }

        mTextPaint.setColor(Color.BLACK);
        mTextPaint.setTextSize(24f); // hard coded for now
        mTextPaint.setTextAlign(Paint.Align.CENTER);
        mText = text;

        mTextPaint.getTextBounds(mText, 0, mText.length(), mTextBounds);
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
        canvas.drawRect(mBounds, mFramePaint);
        canvas.drawText(mText, mBounds.centerX(), mBounds.centerY() - mTextBounds.centerY(), mTextPaint);
    }
}
