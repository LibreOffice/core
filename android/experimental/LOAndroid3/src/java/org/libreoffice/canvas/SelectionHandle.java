package org.libreoffice.canvas;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.PointF;
import android.graphics.RectF;
import android.util.Log;

import org.libreoffice.LOKitShell;
import org.mozilla.gecko.gfx.ImmutableViewportMetrics;

public abstract class SelectionHandle extends CommonCanvasElement {
    private static final long MINIMUM_HANDLE_UPDATE_TIME = 50 * 1000000;

    public final RectF mDocumentPosition;
    protected final Bitmap mBitmap;
    protected final RectF mScreenPosition;
    private final PointF mDragStartPoint = new PointF();
    private long mLastTime = 0;
    private final PointF mDragDocumentPosition = new PointF();

    public SelectionHandle(Bitmap bitmap) {
        mBitmap = bitmap;
        mScreenPosition = new RectF(0, 0, mBitmap.getWidth(), mBitmap.getHeight());
        mDocumentPosition = new RectF();
    }

    protected static Bitmap getBitmapForDrawable(Context context, int drawableId) {
        BitmapFactory.Options options = new BitmapFactory.Options();
        return BitmapFactory.decodeResource(context.getResources(), drawableId, options);
    }

    @Override
    public void onDraw(Canvas canvas) {
        canvas.drawBitmap(mBitmap, mScreenPosition.left, mScreenPosition.top, null);
    }

    @Override
    public boolean onHitTest(float x, float y) {
        return mScreenPosition.contains(x, y);
    }

    public void reposition(float x, float y) {
        mScreenPosition.offsetTo(x, y);
    }

    public void dragStart(PointF point) {
        mDragStartPoint.x = point.x;
        mDragStartPoint.y = point.y;
        mDragDocumentPosition.x = mDocumentPosition.left;
        mDragDocumentPosition.y = mDocumentPosition.top;
    }

    public void dragEnd(PointF point) {
        //move(point.x, point.y);
    }

    public void dragging(PointF point) {
        long currentTime = System.nanoTime();
        if (currentTime - mLastTime > MINIMUM_HANDLE_UPDATE_TIME) {
            mLastTime = currentTime;
            move(point.x, point.y);
        }
    }

    private void move(float newX, float newY) {
        ImmutableViewportMetrics viewportMetrics = LOKitShell.getLayerView().getLayerClient().getViewportMetrics();
        float zoom = viewportMetrics.zoomFactor;

        float deltaX = (newX - mDragStartPoint.x) / zoom;
        float deltaY = (newY - mDragStartPoint.y) / zoom;

        PointF documentPoint = new PointF(mDragDocumentPosition.x + deltaX, mDragDocumentPosition.y + deltaY);

        LOKitShell.sendChangeHandlePositionEvent(getHandleType(), documentPoint);
    }

    public abstract HandleType getHandleType();

    public enum HandleType { START, MIDDLE, END }
}
