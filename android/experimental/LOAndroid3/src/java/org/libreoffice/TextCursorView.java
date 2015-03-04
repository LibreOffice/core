package org.libreoffice;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PointF;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.widget.RelativeLayout;

import org.mozilla.gecko.gfx.ImmutableViewportMetrics;
import org.mozilla.gecko.gfx.LayerView;
import org.mozilla.gecko.gfx.RectUtils;

import java.util.ArrayList;
import java.util.List;

/**
 * Text cursor view responsible to show the cursor drawable on the screen.
 */
public class TextCursorView extends View {
    private static final String LOGTAG = TextCursorView.class.getSimpleName();
    private static final float CURSOR_WIDTH = 2f;

    private boolean mInitialized = false;
    private RectF mCursorPosition = new RectF();
    private RectF mCursorScaledPosition = new RectF();
    private int mCursorAlpha = 0;

    private List<RectF> mSelections = new ArrayList<RectF>();
    private List<RectF> mScaledSelections = new ArrayList<RectF>();
    private Paint mCursorPaint = new Paint();
    private Paint mSelectionPaint = new Paint();

    private boolean mCursorVisible;

    public TextCursorView(Context context) {
        super(context);
        initialize();
    }

    public TextCursorView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initialize();
    }

    public TextCursorView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initialize();
    }

    private void initialize() {
        if (!mInitialized) {
            postDelayed(cursorAnimation, 500);

            mCursorPaint.setColor(Color.BLACK);
            mCursorPaint.setAlpha(0);

            mCursorVisible = false;

            mSelectionPaint.setColor(Color.BLUE);
            mSelectionPaint.setAlpha(50);

            mInitialized = true;
        }
    }

    public void changeCursorPosition(RectF position) {
        LayerView layerView = LOKitShell.getLayerView();
        if (layerView == null) {
            Log.e(LOGTAG, "Can't position cursor because layerView is null");
            return;
        }

        mCursorPosition = position;
        ImmutableViewportMetrics metrics = layerView.getViewportMetrics();
        repositionWithViewport(metrics.viewportRectLeft, metrics.viewportRectTop, metrics.zoomFactor);
    }

    public void changeSelections(List<RectF> selectionRects) {
        LayerView layerView = LOKitShell.getLayerView();
        if (layerView == null) {
            Log.e(LOGTAG, "Can't position selections because layerView is null");
            return;
        }

        mSelections = selectionRects;

        ImmutableViewportMetrics metrics = layerView.getViewportMetrics();
        repositionWithViewport(metrics.viewportRectLeft, metrics.viewportRectTop, metrics.zoomFactor);
    }

    public void repositionWithViewport(float x, float y, float zoom) {
        mCursorScaledPosition = RectUtils.scale(mCursorPosition, zoom);
        mCursorScaledPosition.offset(-x, -y);
        mCursorScaledPosition.right = mCursorScaledPosition.left + CURSOR_WIDTH;

        mScaledSelections.clear();
        for (RectF selection : mSelections) {
            RectF scaledSelection = RectUtils.scale(selection, zoom);
            scaledSelection.offset(-x, -y);
            mScaledSelections.add(scaledSelection);
        }
        invalidate();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        canvas.drawRect(mCursorScaledPosition, mCursorPaint);

        for (RectF selection : mScaledSelections) {
            canvas.drawRect(selection, mSelectionPaint);
        }
    }

    private Runnable cursorAnimation = new Runnable() {
        public void run() {
            if (mCursorVisible) {
                mCursorPaint.setAlpha(mCursorPaint.getAlpha() == 0 ? 0xFF : 0);
                invalidate();
            }
            postDelayed(cursorAnimation, 500);
        }
    };

    public void showCursor() {
        mCursorVisible = true;
        mCursorPaint.setAlpha(0xFF);
        invalidate();
    }

    public void hideCursor() {
        mCursorVisible = false;
        mCursorPaint.setAlpha(0);
        invalidate();
    }
}