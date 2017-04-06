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
import android.graphics.Rect;
import android.text.TextPaint;
import android.util.Log;

public class PageNumberRect extends CommonCanvasElement {
    private static String LOGTAG = PageNumberRect.class.getSimpleName();
    private String mPageNumberString;
    private TextPaint mPageNumberRectPaint = new TextPaint();
    private Paint mBgPaint = new Paint();
    private Rect mTextBounds = new Rect();
    private float mBgMargin = 5f;

    public PageNumberRect() {
        mBgPaint.setColor(Color.BLACK);
        mBgPaint.setAlpha(100);
        mPageNumberRectPaint.setColor(Color.WHITE);
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
        canvas.drawRect(canvas.getWidth()*0.1f - mBgMargin,
                canvas.getHeight()*0.1f - mTextBounds.height() - mBgMargin,
                mTextBounds.width() + canvas.getWidth()*0.1f + mBgMargin,
                canvas.getHeight()*0.1f + mBgMargin,
                mBgPaint);
        canvas.drawText(mPageNumberString, canvas.getWidth()*0.1f, canvas.getHeight()*0.1f, mPageNumberRectPaint);
        Log.d(LOGTAG, "onDraw called!");
    }

    public void setPageNumberString (String pageNumberString) {
        mPageNumberString = pageNumberString;
        mPageNumberRectPaint.getTextBounds(mPageNumberString, 0, mPageNumberString.length(), mTextBounds);
    }
}
