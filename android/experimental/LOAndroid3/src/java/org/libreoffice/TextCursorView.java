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

/**
 * Text cursor view responsible to show the cursor drawable on the screen.
 */
public class TextCursorView extends View {
    private static final String LOGTAG = TextCursorView.class.getSimpleName();

    private boolean mCursorAnimationEnabled = false;
    private RectF mCursorPosition = new RectF();
    private PointF mCursorScaledPosition = new PointF();

    private float mCursorHeight = 0f;
    private float mCursorWidth = 2f;

    private int mCursorAlpha = 0;

    public TextCursorView(Context context) {
        super(context);
        startCursorAnimation();
    }

    public TextCursorView(Context context, AttributeSet attrs) {
        super(context, attrs);
        startCursorAnimation();
    }

    public TextCursorView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        startCursorAnimation();
    }

    private void startCursorAnimation() {
        if (!mCursorAnimationEnabled) {
            mCursorAnimationEnabled = true;
            postDelayed(cursorAnimation, 500);
        }
    }

    public void changePosition(RectF position) {
        LayerView layerView = LOKitShell.getLayerView();
        if (layerView == null) {
            Log.e(LOGTAG, "Can't position handle because layerView is null");
            return;
        }

        mCursorPosition = position;
        ImmutableViewportMetrics metrics = layerView.getViewportMetrics();
        repositionWithViewport(metrics.viewportRectLeft, metrics.viewportRectTop, metrics.zoomFactor);
    }

    public void repositionWithViewport(float x, float y, float zoom) {
        RectF scaledRectangle = RectUtils.scale(mCursorPosition, zoom);

        mCursorScaledPosition = new PointF(scaledRectangle.left - x, scaledRectangle.top - y);
        mCursorHeight = scaledRectangle.height();

        invalidate();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        Paint paint = new Paint();
        paint.setColor(Color.BLACK);
        paint.setAlpha(mCursorAlpha);
        RectF cursorRect = new RectF(
                mCursorScaledPosition.x,
                mCursorScaledPosition.y,
                mCursorScaledPosition.x + mCursorWidth,
                mCursorScaledPosition.y + mCursorHeight);
        canvas.drawRect(cursorRect, paint);
    }

    private Runnable cursorAnimation = new Runnable() {
        public void run() {
            mCursorAlpha = mCursorAlpha == 0 ? 0xFF : 0;
            invalidate();
            postDelayed(cursorAnimation, 500);
        }
    };
}