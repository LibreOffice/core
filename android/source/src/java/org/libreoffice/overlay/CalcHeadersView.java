package org.libreoffice.overlay;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.PointF;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

import org.libreoffice.canvas.CalcHeaderCell;
import org.mozilla.gecko.gfx.ImmutableViewportMetrics;
import org.mozilla.gecko.gfx.LayerView;

import java.util.ArrayList;

public class CalcHeadersView extends View implements View.OnTouchListener {

    private boolean mInitialized;
    private LayerView mLayerView;
    private boolean mIsRow; // true if this is for row headers, false for column
    private ArrayList<String> mLabels;
    private ArrayList<Float> mDimens;

    public CalcHeadersView(Context context) {
        super(context);
    }

    public CalcHeadersView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public CalcHeadersView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public void initialize(LayerView layerView, boolean isRow) {
        if (!mInitialized) {
            setOnTouchListener(this);
            mLayerView = layerView;
            mIsRow = isRow;

            mInitialized = true;
        }
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        if (mInitialized && mDimens != null && mLabels != null) {
            updateHeaders(canvas);
        }
    }

    private void updateHeaders(Canvas canvas) {
        ImmutableViewportMetrics metrics = mLayerView.getViewportMetrics();
        float zoom = metrics.getZoomFactor();
        PointF origin = metrics.getOrigin();
        boolean inRangeOfVisibleHeaders = false; // a helper variable for skipping unnecessary onDraw()'s
        float top,bottom,left,right;
        for (int i = 1; i < mLabels.size(); i++) {
            if (mIsRow) {
                top = -origin.y + zoom*mDimens.get(i-1);
                bottom = -origin.y + zoom*mDimens.get(i);
                if (top <= getHeight() && bottom >= 0) {
                    inRangeOfVisibleHeaders = true;
                    new CalcHeaderCell(0f, top, getWidth(), bottom - top, mLabels.get(i)).onDraw(canvas);
                } else {
                    if (inRangeOfVisibleHeaders) {
                        break;
                    }
                }
            } else {
                left = -origin.x + zoom*mDimens.get(i-1);
                right = -origin.x + zoom*mDimens.get(i);
                if (left <= getWidth() && right >= 0) {
                    new CalcHeaderCell(left, 0f, right - left, getHeight(), mLabels.get(i)).onDraw(canvas);
                } else {
                    if (inRangeOfVisibleHeaders) {
                        break;
                    }
                }
            }
        }
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        return false;
    }

    public void setHeaders(ArrayList<String> labels, ArrayList<Float> dimens) {
        mLabels = labels;
        mDimens = dimens;
    }
}
