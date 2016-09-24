package org.libreoffice.canvas;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;

/**
 * Bitmap handle canvas element is used to show a handle on the screen.
 * The handle visual comes from the bitmap, which must be provided in time
 * of construction.
 */
public abstract class BitmapHandle extends CommonCanvasElement {
    public final RectF mDocumentPosition;
    protected final Bitmap mBitmap;
    protected final RectF mScreenPosition;

    public BitmapHandle(Bitmap bitmap) {
        mBitmap = bitmap;
        mScreenPosition = new RectF(0, 0, mBitmap.getWidth(), mBitmap.getHeight());
        mDocumentPosition = new RectF();
    }

    /**
     * Return a bitmap for a drawable id.
     */
    protected static Bitmap getBitmapForDrawable(Context context, int drawableId) {
        Drawable drawable = context.getResources().getDrawable(drawableId);

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
     * @return
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
