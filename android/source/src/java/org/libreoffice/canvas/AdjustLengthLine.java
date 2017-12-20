package org.libreoffice.canvas;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PointF;
import android.graphics.RectF;

import org.json.JSONException;
import org.json.JSONObject;
import org.libreoffice.LOEvent;
import org.libreoffice.LOKitShell;
import org.libreoffice.LibreOfficeMainActivity;
import org.libreoffice.overlay.CalcHeadersView;
import org.mozilla.gecko.gfx.ImmutableViewportMetrics;

import static org.libreoffice.SearchController.addProperty;
import static org.libreoffice.UnitConverter.pixelToTwip;
import static org.libreoffice.UnitConverter.twipsToHMM;

public class AdjustLengthLine extends CommonCanvasElement {

    private static final float STROKE_WIDTH = 4f;
    private static final float TOUCH_VICINITY_RADIUS = 24f;

    private LibreOfficeMainActivity mContext;
    private CalcHeadersView mCalcHeadersView;
    private boolean mIsRow;
    private PointF mScreenPosition;
    private float mWidth;
    private float mHeight;
    private Paint mPaint;
    private PointF mStartScreenPosition;
    private int mIndex;

    public AdjustLengthLine(LibreOfficeMainActivity context, CalcHeadersView view, boolean isRow, float width, float height) {
        super();
        mContext = context;
        mCalcHeadersView = view;
        mIsRow = isRow;
        mWidth = width;
        mHeight = height;
        mPaint = new Paint();
        mPaint.setColor(Color.BLACK);
        mPaint.setStrokeWidth(STROKE_WIDTH);
    }

    @Override
    public boolean onHitTest(float x, float y) {
        if (mIsRow) {
            return mScreenPosition.y - TOUCH_VICINITY_RADIUS < y &&
                    y < mScreenPosition.y + TOUCH_VICINITY_RADIUS;
        } else {
            return mScreenPosition.x - TOUCH_VICINITY_RADIUS < x &&
                    x < mScreenPosition.x + TOUCH_VICINITY_RADIUS;
        }
    }

    @Override
    public void onDraw(Canvas canvas) {
        if (mIsRow) {
            canvas.drawLine(0f, mScreenPosition.y, mWidth, mScreenPosition.y, mPaint);
        } else {
            canvas.drawLine(mScreenPosition.x, 0f, mScreenPosition.x, mHeight, mPaint);
        }
    }

    public void dragStart(PointF point) {
    }

    public void dragging(PointF point) {
        mScreenPosition = point;
    }

    public void dragEnd(PointF point) {
        ImmutableViewportMetrics viewportMetrics = mContext.getLayerClient().getViewportMetrics();
        float zoom = viewportMetrics.zoomFactor;

        PointF documentDistance = new PointF(pixelToTwip((point.x-mStartScreenPosition.x)/zoom, LOKitShell.getDpi(mContext)),
                pixelToTwip((point.y-mStartScreenPosition.y)/zoom, LOKitShell.getDpi(mContext)));

        try {
            JSONObject rootJson = new JSONObject();
            if (mIsRow) {
                addProperty(rootJson, "Row", "long", String.valueOf(mIndex));
                addProperty(rootJson, "RowHeight", "unsigned short", String.valueOf(Math.round(documentDistance.y > 0 ? twipsToHMM(documentDistance.y) : 0)));
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:RowHeight", rootJson.toString()));
            } else {
                addProperty(rootJson, "Column", "long", String.valueOf(mIndex));
                addProperty(rootJson, "ColumnWidth", "unsigned short", String.valueOf(documentDistance.x > 0 ? twipsToHMM(documentDistance.x) : 0));
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:ColumnWidth", rootJson.toString()));
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public void setScreenRect(RectF position) {
        mScreenPosition = new PointF(position.right, position.bottom);
        mStartScreenPosition = new PointF(position.left, position.top);
        mIndex = 1 + mCalcHeadersView.getIndexFromPointOfTouch(new PointF(position.centerX(), position.centerY()));
    }
}
