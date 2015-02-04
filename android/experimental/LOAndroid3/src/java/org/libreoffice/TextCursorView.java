package org.libreoffice;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.PointF;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.RelativeLayout;

import org.mozilla.gecko.gfx.ImmutableViewportMetrics;
import org.mozilla.gecko.gfx.LayerView;
import org.mozilla.gecko.gfx.RectUtils;

/**
 * Text cursor view responsible to show the cursor drawable on the screen.
 */
public class TextCursorView extends ImageView {
    private static final String LOGTAG = TextCursorView.class.getSimpleName();

    private RectF mPosition;
    private RelativeLayout.LayoutParams mLayoutParams;

    private int mLeft;
    private int mTop;

    private int mWidth;
    private int mHeight;

    public TextCursorView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public void changePosition(RectF position) {
        LayerView layerView = LOKitShell.getLayerView();
        if (layerView == null) {
            Log.e(LOGTAG, "Can't position handle because layerView is null");
            return;
        }

        mPosition = position;

        mWidth = Math.round(position.width());
        mHeight = Math.round(position.height());

        ImmutableViewportMetrics metrics = layerView.getViewportMetrics();
        repositionWithViewport(metrics.viewportRectLeft, metrics.viewportRectTop, metrics.zoomFactor);
    }

    public void repositionWithViewport(float x, float y, float zoom) {
        RectF scaled = RectUtils.scale(mPosition, zoom);

        mLeft = Math.round(scaled.centerX() - x);
        mTop = Math.round(scaled.centerY() - y);

        setScaleY(scaled.height());

        setLayoutPosition();
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