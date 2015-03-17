/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.PointF;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;
import android.widget.RelativeLayout;

import org.json.JSONObject;
import org.libreoffice.LOKitShell;
import org.libreoffice.R;
import org.mozilla.gecko.gfx.ImmutableViewportMetrics;
import org.mozilla.gecko.gfx.LayerView;

/**
 * Custom image view used for showing the text selection handles.
 */
public class TextSelectionHandle extends ImageView implements View.OnTouchListener {
    private static final String LOGTAG = TextSelectionHandle.class.getSimpleName();
    private long mLastTime = 0;

    // Minimum time lapsed between 2 handle updates
    private static final long MINIMUM_HANDLE_UPDATE_TIME = 50 * 1000000;

    public enum HandleType { START, MIDDLE, END };

    private final HandleType mHandleType;
    private final int mWidth;
    private final int mHeight;
    private final int mShadow;

    private int mLeft;
    private int mTop;
    private boolean mIsRTL;
    private PointF mPoint;
    private PointF mReposition;
    private int mTouchStartX;
    private int mTouchStartY;

    private RelativeLayout.LayoutParams mLayoutParams;

    private static final int IMAGE_LEVEL_LTR = 0;
    private static final int IMAGE_LEVEL_RTL = 1;

    public TextSelectionHandle(Context context, AttributeSet attrs) {
        super(context, attrs);
        setOnTouchListener(this);

        TypedArray array = context.obtainStyledAttributes(attrs, R.styleable.TextSelectionHandle);
        int handleType = array.getInt(R.styleable.TextSelectionHandle_handleType, 0x01);

        switch (handleType) {
            case 1:
                mHandleType = HandleType.START;
                break;
            case 2:
                mHandleType = HandleType.MIDDLE;
                break;
            case 3:
                mHandleType = HandleType.END;
                break;
            default:
                throw new RuntimeException("Unknown text handle id");
        }

        mIsRTL = false;
        mPoint = new PointF(0.0f, 0.0f);

        mWidth = getResources().getDimensionPixelSize(R.dimen.text_selection_handle_width);
        mHeight = getResources().getDimensionPixelSize(R.dimen.text_selection_handle_height);
        mShadow = getResources().getDimensionPixelSize(R.dimen.text_selection_handle_shadow);
    }

    public boolean onTouch(View v, MotionEvent event) {
        switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN: {
                mTouchStartX = Math.round(event.getX());
                mTouchStartY = Math.round(event.getY());
                break;
            }
            case MotionEvent.ACTION_UP: {
                mTouchStartX = 0;
                mTouchStartY = 0;
                break;
            }
            case MotionEvent.ACTION_MOVE: {
                long currentTime = System.nanoTime();
                if (currentTime - mLastTime > MINIMUM_HANDLE_UPDATE_TIME) {
                    mLastTime = currentTime;
                    move(event.getX(), event.getY());
                }
                break;
            }
        }
        return true;
    }

    private void move(float newX, float newY) {
        LayerView layerView = LOKitShell.getLayerView();
        if (layerView == null) {
            Log.e(LOGTAG, "Can't move selection because layerView is null");
            return;
        }

        float newLeft = mLeft + newX - mTouchStartX;
        float newTop  = mTop + newY - mTouchStartY;

        // Send x coordinate on the right side of the start handle, left side of the end handle.
        float left = (float) newLeft + adjustLeftForHandle();

        PointF documentPoint = new PointF(left, newTop);
        documentPoint = layerView.getLayerClient().convertViewPointToLayerPoint(documentPoint);
        documentPoint.x += mReposition.x;
        documentPoint.y += mReposition.y;

        LOKitShell.sendChangeHandlePositionEvent(mHandleType, documentPoint);
    }

    /**
     * Calculate the position just under (and centered horizontally) rectangle from the input rectangle.
     *
     * @param rectangle - input rectangle
     * @return position just under the selection
     */
    private PointF positionUnderSelection(RectF rectangle) {
        return new PointF(rectangle.centerX(), rectangle.bottom);
    }

    void positionFromGecko(RectF position,  boolean rtl) {
        LayerView layerView = LOKitShell.getLayerView();
        if (layerView == null) {
            Log.e(LOGTAG, "Can't position handle because layerView is null");
            return;
        }

        mPoint = positionUnderSelection(position);
        mReposition = new PointF(position.left - mPoint.x, position.top - mPoint.y);

        if (mIsRTL != rtl) {
            mIsRTL = rtl;
            setImageLevel(mIsRTL ? IMAGE_LEVEL_RTL : IMAGE_LEVEL_LTR);
        }

        ImmutableViewportMetrics metrics = layerView.getViewportMetrics();
        repositionWithViewport(metrics.viewportRectLeft, metrics.viewportRectTop, metrics.zoomFactor);
    }

    void repositionWithViewport(float x, float y, float zoom) {
        PointF viewPoint = new PointF(mPoint.x * zoom - x,
                                      mPoint.y * zoom - y);

        mLeft = Math.round(viewPoint.x) - (int) adjustLeftForHandle();
        mTop = Math.round(viewPoint.y);

        setLayoutPosition();
    }

    private float adjustLeftForHandle() {
        if (mHandleType.equals(HandleType.START))
            return mIsRTL ? mShadow : mWidth - mShadow;
        else if (mHandleType.equals(HandleType.MIDDLE))
            return (float) ((mWidth - mShadow) / 2);
        else
            return mIsRTL ? mWidth - mShadow : mShadow;
    }

    private void setLayoutPosition() {
        if (mLayoutParams == null) {
            mLayoutParams = (RelativeLayout.LayoutParams) getLayoutParams();
            // Set negative right/bottom margins so that the handles can be dragged outside of
            // the content area (if they are dragged to the left/top, the dyanmic margins set
            // below will take care of that).
            mLayoutParams.rightMargin = 0 - mWidth;
            mLayoutParams.bottomMargin = 0 - mHeight;
        }

        mLayoutParams.leftMargin = mLeft;
        mLayoutParams.topMargin = mTop;
        setLayoutParams(mLayoutParams);
    }
}
