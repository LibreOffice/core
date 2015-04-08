package org.libreoffice.canvas;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.RectF;

public class Cursor extends CommonCanvasElement {
    private static final float CURSOR_WIDTH = 2f;
    private final Paint mCursorPaint = new Paint();
    public RectF mPosition = new RectF();
    public RectF mScaledPosition = new RectF();
    public int mAlpha = 0;

    public Cursor() {
        mCursorPaint.setColor(Color.BLACK);
        mCursorPaint.setAlpha(0xFF);
    }

    @Override
    public boolean onHitTest(float x, float y) {
        return false;
    }

    @Override
    public void onDraw(Canvas canvas) {
        canvas.drawRect(mScaledPosition, mCursorPaint);
    }

    public void reposition(RectF rect) {
        mScaledPosition = rect;
        mScaledPosition.right = mScaledPosition.left + CURSOR_WIDTH;
    }

    public void cycleAlpha() {
        mCursorPaint.setAlpha(mCursorPaint.getAlpha() == 0 ? 0xFF : 0);
    }
}
