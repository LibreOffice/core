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

public class CalcHeadersView extends View implements View.OnTouchListener {

    private boolean mInitialized;
    private LayerView mLayerView;
    private boolean mIsRow;

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
        if (mInitialized) {
            updateHeaders(canvas);
        }
    }

    private void updateHeaders(Canvas canvas) {
        for (int i = 0; i < 10; i++) {
            if (mIsRow) {
                new CalcHeaderCell(i*30f, 0f, 30f, getHeight(), "" + i).onDraw(canvas);
            } else {
                new CalcHeaderCell(0f, i*30f, getWidth(), 30f, "" + i).onDraw(canvas);
            }
        }
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        return false;
    }
}
