package org.libreoffice.canvas;

import android.graphics.Bitmap;
import android.graphics.PointF;

import org.libreoffice.LOKitShell;
import org.mozilla.gecko.gfx.ImmutableViewportMetrics;

/**
 * Selection handle is a common class for "start", "middle" and "end" types
 * of selection handles.
 */
public abstract class SelectionHandle extends BitmapHandle {
    private static final long MINIMUM_HANDLE_UPDATE_TIME = 50 * 1000000;

    private final PointF mDragStartPoint = new PointF();
    private final PointF mDragDocumentPosition = new PointF();
    private long mLastTime = 0;

    public SelectionHandle(Bitmap bitmap) {
        super(bitmap);
    }

    /**
     * Start of a touch and drag action on the handle.
     */
    public void dragStart(PointF point) {
        mDragStartPoint.x = point.x;
        mDragStartPoint.y = point.y;
        mDragDocumentPosition.x = mDocumentPosition.left;
        mDragDocumentPosition.y = mDocumentPosition.top;
    }

    /**
     * End of a touch and drag action on the handle.
     */
    public void dragEnd(PointF point) {
    }

    /**
     * Handle has been dragged.
     */
    public void dragging(PointF point) {
        long currentTime = System.nanoTime();
        if (currentTime - mLastTime > MINIMUM_HANDLE_UPDATE_TIME) {
            mLastTime = currentTime;
            signalHandleMove(point.x, point.y);
        }
    }

    /**
     * Signal to move the handle to a new position to LO.
     */
    private void signalHandleMove(float newX, float newY) {
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
