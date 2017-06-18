package org.libreoffice.canvas;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PointF;
import android.graphics.RectF;

import org.libreoffice.LOKitShell;
import org.libreoffice.LibreOfficeMainActivity;
import org.mozilla.gecko.gfx.ImmutableViewportMetrics;

/**
 * CalcSelectionBox is the selection frame for the current highlighted area/cells
 * in Calc.
 */

public class CalcSelectionBox extends CommonCanvasElement {
    private static final long MINIMUM_HANDLE_UPDATE_TIME = 50 * 1000000;
    private static final float CIRCLE_HANDLE_RADIUS = 8f;

    public RectF mDocumentPosition;

    private LibreOfficeMainActivity mContext;
    private RectF mScreenPosition;
    private long mLastTime = 0;
    private Paint mPaint;
    private Paint mCirclePaint;

    public CalcSelectionBox(LibreOfficeMainActivity context) {
        mContext = context;
        mScreenPosition = new RectF();
        mDocumentPosition = new RectF();
        mPaint = new Paint();
        mPaint.setStyle(Paint.Style.STROKE);
        mPaint.setColor(Color.BLACK);
        mPaint.setStrokeWidth(2f);
        mCirclePaint = new Paint();
        mCirclePaint.setColor(Color.BLACK);
        mCirclePaint.setStyle(Paint.Style.FILL);
    }

    /**
     * Start of a touch and drag action on the box.
     */
    public void dragStart(PointF point) {}

    /**
     * End of a touch and drag action on the box.
     */
    public void dragEnd(PointF point) {}

    /**
     * Box has been dragged.
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
        ImmutableViewportMetrics viewportMetrics = mContext.getLayerClient().getViewportMetrics();
        float zoom = viewportMetrics.zoomFactor;
        PointF origin = viewportMetrics.getOrigin();

        PointF documentPoint = new PointF((newX+origin.x)/zoom , (newY+origin.y)/zoom);

        if (documentPoint.x < mDocumentPosition.left || documentPoint.y < mDocumentPosition.top) {
            LOKitShell.sendChangeHandlePositionEvent(SelectionHandle.HandleType.START, documentPoint);
        } else if (documentPoint.x > mDocumentPosition.right || documentPoint.y > mDocumentPosition.bottom){
            LOKitShell.sendChangeHandlePositionEvent(SelectionHandle.HandleType.END, documentPoint);
        }
    }

    @Override
    public boolean onHitTest(float x, float y) {
        return mScreenPosition.contains(x, y);
    }

    @Override
    public void onDraw(Canvas canvas) {
        canvas.drawRect(mScreenPosition, mPaint);
        canvas.drawCircle(mScreenPosition.left, mScreenPosition.top, CIRCLE_HANDLE_RADIUS, mCirclePaint);
        canvas.drawCircle(mScreenPosition.right, mScreenPosition.bottom, CIRCLE_HANDLE_RADIUS, mCirclePaint);
    }

    public void reposition(RectF rect) {
        mScreenPosition = rect;
    }

    @Override
    public boolean contains(float x, float y) {
        // test if in range of the box or the circular handles
        boolean inRange =  new RectF(mScreenPosition.left - CIRCLE_HANDLE_RADIUS,
                mScreenPosition.top - CIRCLE_HANDLE_RADIUS,
                mScreenPosition.left + CIRCLE_HANDLE_RADIUS,
                mScreenPosition.top + CIRCLE_HANDLE_RADIUS).contains(x, y)
                || new RectF(mScreenPosition.right - CIRCLE_HANDLE_RADIUS,
                mScreenPosition.bottom - CIRCLE_HANDLE_RADIUS,
                mScreenPosition.right + CIRCLE_HANDLE_RADIUS,
                mScreenPosition.bottom + CIRCLE_HANDLE_RADIUS).contains(x, y)
                || onHitTest(x, y);
        return inRange && isVisible();
    }
}
