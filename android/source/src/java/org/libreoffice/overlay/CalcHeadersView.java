package org.libreoffice.overlay;

import android.content.Context;
import android.graphics.Canvas;
import android.util.AttributeSet;
import android.view.ContextMenu;
import android.view.MotionEvent;
import android.view.View;
import android.view.View;

import org.libreoffice.canvas.CalcHeaderCell;
import org.mozilla.gecko.gfx.LayerView;

import java.util.ArrayList;

public class CalcHeadersView extends View implements View.OnTouchListener {

    private boolean mInitialized;
    private LayerView mLayerView;
    private boolean mIsRow;
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
        for (int i = 1; i < mLabels.size(); i++) {
            if (mIsRow) {
                new CalcHeaderCell(0f, mDimens.get(i-1), getWidth(),
                        mDimens.get(i) - mDimens.get(i-1), mLabels.get(i)).onDraw(canvas);
            } else {
                new CalcHeaderCell(mDimens.get(i-1), 0f,
                        mDimens.get(i) - mDimens.get(i-1), getHeight(), mLabels.get(i)).onDraw(canvas);
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
