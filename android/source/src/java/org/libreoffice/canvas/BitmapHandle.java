package org.libreoffice.canvas;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;
import android.support.v4.content.ContextCompat;

/**
 * Bitmap handle canvas element is used to show a handle on the screen.
 * The handle visual comes from the bitmap, which must be provided in time
 * of construction.
 */
public abstract class BitmapHandle extends CommonCanvasElement {
    public final RectF mDocumentPosition;
    private final Bitmap mBitmap;
    final RectF mScreenPosition;

    BitmapHandle(Bitmap bitmap) {
        mBitmap = bitmap;
        mScreenPosition = new RectF(0, 0, mBitmap.getWidth(), mBitmap.getHeight());
        mDocumentPosition = new RectF();
    }

    /**
     * Return a bitmap for a drawable id.
     */
    static Bitmap getBitmapForDrawable(Context context, int drawableId) {
        Drawable drawable = ContextCompat.getDrawable(context, drawableId);

        return ImageUtils.getBitmapForDrawable(drawable);
    }

    /**
     * Draw the bitmap handle to the canvas.
     * @param canvas - the canvas
     */
    @Override
    public void onDraw(Canvas canvas) {
        canvas.drawBitmap(mBitmap, mScreenPosition.left, mScreenPosition.top, null);
    }

    /**
     * Test if the bitmap has been hit.
     * @param x - x coordinate
     * @param y - y coordinate
     * @return true if the bitmap has been hit
     */
    @Override
    public boolean onHitTest(float x, float y) {
        return mScreenPosition.contains(x, y);
    }

    /**
     * Change the position of the handle.
     * @param x - x coordinate
     * @param y - y coordinate
     */
    public void reposition(float x, float y) {
        mScreenPosition.offsetTo(x, y);
    }
}
