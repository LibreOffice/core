/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.PointF;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
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
public class TextCursorView extends View implements View.OnTouchListener {
    private static final String LOGTAG = TextCursorView.class.getSimpleName();
    private static final float CURSOR_WIDTH = 2f;
    private static final int CURSOR_BLINK_TIME = 500;

    private boolean mInitialized = false;
    private RectF mCursorPosition = new RectF();
    private RectF mCursorScaledPosition = new RectF();
    private Paint mCursorPaint = new Paint();
    private int mCursorAlpha = 0;
    private boolean mCursorVisible;

    private List<RectF> mSelections = new ArrayList<RectF>();
    private List<RectF> mScaledSelections = new ArrayList<RectF>();
    private Paint mSelectionPaint = new Paint();
    private boolean mSelectionsVisible;

    private RectF mGraphicSelection = new RectF();
    private RectF mGraphicScaledSelection = new RectF();
    private Paint mGraphicSelectionPaint = new Paint();
    private Paint mGraphicHandleFillPaint = new Paint();
    private float mRadius = 20.0f;
    private boolean mGraphicSelectionVisible;
    private PointF mTouchStart = new PointF();
    private PointF mDeltaPoint = new PointF();
    private boolean mGraphicSelectionMove = false;
    private LayerView mLayerView;

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

    /**
     * Initialize the selection and cursor view.
     */
    private void initialize() {
        if (!mInitialized) {
            setOnTouchListener(this);

            mCursorPaint.setColor(Color.BLACK);
            mCursorPaint.setAlpha(0xFF);
            mCursorVisible = false;

            mSelectionPaint.setColor(Color.BLUE);
            mSelectionPaint.setAlpha(50);
            mSelectionsVisible = false;

            mGraphicSelectionPaint.setStyle(Paint.Style.STROKE);
            mGraphicSelectionPaint.setColor(Color.BLACK);
            mGraphicSelectionPaint.setStrokeWidth(2);

            mGraphicHandleFillPaint.setStyle(Paint.Style.FILL);
            mGraphicHandleFillPaint.setColor(Color.WHITE);
            mGraphicSelectionVisible = false;

            postDelayed(cursorAnimation, CURSOR_BLINK_TIME);

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

    public void changeGraphicSelection(RectF rectangle) {
        LayerView layerView = LOKitShell.getLayerView();
        if (layerView == null) {
            Log.e(LOGTAG, "Can't position selections because layerView is null");
            return;
        }

        mGraphicSelection = rectangle;

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

        mGraphicScaledSelection = RectUtils.scale(mGraphicSelection, zoom);
        mGraphicScaledSelection.offset(-x, -y);
        invalidate();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        if (mCursorVisible) {
            canvas.drawRect(mCursorScaledPosition, mCursorPaint);
        }
        if (mSelectionsVisible) {
            for (RectF selection : mScaledSelections) {
                canvas.drawRect(selection, mSelectionPaint);
            }
        }
        if (mGraphicSelectionVisible) {
            canvas.drawRect(mGraphicScaledSelection, mGraphicSelectionPaint);
            canvas.drawCircle(mGraphicScaledSelection.left, mGraphicScaledSelection.top, mRadius, mGraphicHandleFillPaint);
            canvas.drawCircle(mGraphicScaledSelection.left, mGraphicScaledSelection.top, mRadius, mGraphicSelectionPaint);

            canvas.drawCircle(mGraphicScaledSelection.right, mGraphicScaledSelection.top, mRadius, mGraphicHandleFillPaint);
            canvas.drawCircle(mGraphicScaledSelection.right, mGraphicScaledSelection.top, mRadius, mGraphicSelectionPaint);

            canvas.drawCircle(mGraphicScaledSelection.left, mGraphicScaledSelection.bottom, mRadius, mGraphicHandleFillPaint);
            canvas.drawCircle(mGraphicScaledSelection.left, mGraphicScaledSelection.bottom, mRadius, mGraphicSelectionPaint);

            canvas.drawCircle(mGraphicScaledSelection.right, mGraphicScaledSelection.bottom, mRadius, mGraphicHandleFillPaint);
            canvas.drawCircle(mGraphicScaledSelection.right, mGraphicScaledSelection.bottom, mRadius, mGraphicSelectionPaint);

            canvas.drawCircle(mGraphicScaledSelection.left, mGraphicScaledSelection.centerY(), mRadius, mGraphicHandleFillPaint);
            canvas.drawCircle(mGraphicScaledSelection.left, mGraphicScaledSelection.centerY(), mRadius, mGraphicSelectionPaint);

            canvas.drawCircle(mGraphicScaledSelection.right, mGraphicScaledSelection.centerY(), mRadius, mGraphicHandleFillPaint);
            canvas.drawCircle(mGraphicScaledSelection.right, mGraphicScaledSelection.centerY(), mRadius, mGraphicSelectionPaint);

            canvas.drawCircle(mGraphicScaledSelection.centerX(), mGraphicScaledSelection.top, mRadius, mGraphicHandleFillPaint);
            canvas.drawCircle(mGraphicScaledSelection.centerX(), mGraphicScaledSelection.top, mRadius, mGraphicSelectionPaint);

            canvas.drawCircle(mGraphicScaledSelection.centerX(), mGraphicScaledSelection.bottom, mRadius, mGraphicHandleFillPaint);
            canvas.drawCircle(mGraphicScaledSelection.centerX(), mGraphicScaledSelection.bottom, mRadius, mGraphicSelectionPaint);

            if (mGraphicSelectionMove) {
                RectF one = new RectF(mGraphicScaledSelection);
                one.offset(mDeltaPoint.x, mDeltaPoint.y);
                canvas.drawRect(one, mGraphicSelectionPaint);
            }
        }
    }

    private Runnable cursorAnimation = new Runnable() {
        public void run() {
            if (mCursorVisible) {
                mCursorPaint.setAlpha(mCursorPaint.getAlpha() == 0 ? 0xFF : 0);
                invalidate();
            }
            postDelayed(cursorAnimation, CURSOR_BLINK_TIME);
        }
    };

    public void showCursor() {
        mCursorVisible = true;
        invalidate();
    }

    public void hideCursor() {
        mCursorVisible = false;
        invalidate();
    }

    public void showSelections() {
        mSelectionsVisible = true;
        invalidate();
    }

    public void hideSelections() {
        mSelectionsVisible = false;
        invalidate();
    }

    public void showGraphicSelection() {
        mGraphicSelectionVisible = true;
        invalidate();
    }

    public void hideGraphicSelection() {
        mGraphicSelectionVisible = false;
        invalidate();
    }

    @Override
    public boolean onTouch(View view, MotionEvent event) {
        if (mLayerView == null) {
            mLayerView = LOKitShell.getLayerView();
            if (mLayerView == null) {
                return false;
            }
        }

        switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN: {
                mTouchStart.x = event.getX();
                mTouchStart.y = event.getY();
                if (mGraphicSelectionVisible && mGraphicScaledSelection.contains(mTouchStart.x, mTouchStart.y)) {
                    mGraphicSelectionMove = true;
                    PointF documentPoint = mLayerView.getLayerClient().convertViewPointToLayerPoint(mTouchStart);
                    Log.i(LOGTAG, "Down: " + documentPoint);
                    LOKitShell.sendTouchEvent("GraphicSelectionStart", documentPoint);
                    return true;
                }
            }
            case MotionEvent.ACTION_UP: {
                if (mGraphicSelectionMove) {
                    mGraphicSelectionMove = false;
                    mTouchStart.offset(mDeltaPoint.x, mDeltaPoint.y);
                    PointF documentPoint = mLayerView.getLayerClient().convertViewPointToLayerPoint(mTouchStart);
                    Log.i(LOGTAG, "Up: " + documentPoint);
                    LOKitShell.sendTouchEvent("GraphicSelectionEnd", documentPoint);
                    mTouchStart.x = 0.0f;
                    mTouchStart.y = 0.0f;
                    mDeltaPoint.x = 0.0f;
                    mDeltaPoint.y = 0.0f;
                    invalidate();
                    return true;
                }
            }
            case MotionEvent.ACTION_MOVE: {
                if (mGraphicSelectionMove) {
                    mDeltaPoint.x = event.getX() - mTouchStart.x;
                    mDeltaPoint.y = event.getY() - mTouchStart.y;
                    invalidate();
                    return true;
                }
            }
        }
        return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
