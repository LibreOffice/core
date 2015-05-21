package org.libreoffice.canvas;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.RectF;

/**
 * Handles the cursor drawing on the canvas.
 */
public class Cursor extends CommonCanvasElement {
    private static final float CURSOR_WIDTH = 2f;
    private final Paint mCursorPaint = new Paint();
    public RectF mPosition = new RectF();
    public RectF mScaledPosition = new RectF();
    public int mAlpha = 0;

    /**
     * Construct the cursor and set the default values.
     */
    public Cursor() {
        mCursorPaint.setColor(Color.BLACK);
        mCursorPaint.setAlpha(0xFF);
    }

    /**
     * Hit test for cursor, always false.
     */
    @Override
    public boolean onHitTest(float x, float y) {
        return false;
    }

    /**
     * Draw the cursor.
     */
    @Override
    public void onDraw(Canvas canvas) {
        canvas.drawRect(mScaledPosition, mCursorPaint);
    }

    /**
     * Reposition the cursor on screen.
     */
    public void reposition(RectF rect) {
        mScaledPosition = rect;
        mScaledPosition.right = mScaledPosition.left + CURSOR_WIDTH;
    }

    /**
     * Cycle the alpha color of the cursor, makes the
     */
    public void cycleAlpha() {
        mCursorPaint.setAlpha(mCursorPaint.getAlpha() == 0 ? 0xFF : 0);
    }
}
