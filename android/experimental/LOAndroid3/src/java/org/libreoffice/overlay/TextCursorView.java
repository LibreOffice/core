/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.overlay;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PointF;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

import org.libreoffice.LOKitShell;
import org.libreoffice.canvas.GraphicSelection;
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

    private GraphicSelection mGraphicSelection;

    private boolean mGraphicSelectionVisible;
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

            mGraphicSelection = new GraphicSelection();

            mGraphicSelectionVisible = false;

            postDelayed(cursorAnimation, CURSOR_BLINK_TIME);

            mInitialized = true;
        }
    }

    /**
     * Change the cursor position.
     * @param position - new position of the cursor
     */
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

    /**
     * Change the text selection rectangles.
     * @param selectionRects - list of text selection rectangles
     */
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

    /**
     * Change the graphic selection rectangle.
     * @param rectangle - new graphic selection rectangle
     */
    public void changeGraphicSelection(RectF rectangle) {
        LayerView layerView = LOKitShell.getLayerView();
        if (layerView == null) {
            Log.e(LOGTAG, "Can't position selections because layerView is null");
            return;
        }

        mGraphicSelection.mRectangle = rectangle;

        ImmutableViewportMetrics metrics = layerView.getViewportMetrics();
        repositionWithViewport(metrics.viewportRectLeft, metrics.viewportRectTop, metrics.zoomFactor);
    }

    public void repositionWithViewport(float x, float y, float zoom) {
        mCursorScaledPosition = convertPosition(mCursorPosition, x, y, zoom);
        mCursorScaledPosition.right = mCursorScaledPosition.left + CURSOR_WIDTH;

        mScaledSelections.clear();
        for (RectF selection : mSelections) {
            RectF scaledSelection = convertPosition(selection, x, y, zoom);
            mScaledSelections.add(scaledSelection);
        }

        RectF scaledGraphicSelection = convertPosition(mGraphicSelection.mRectangle, x, y, zoom);
        mGraphicSelection.reposition(scaledGraphicSelection);

        invalidate();
    }

    private RectF convertPosition(RectF cursorPosition, float x, float y, float zoom) {
        RectF cursor = RectUtils.scale(cursorPosition, zoom);
        cursor.offset(-x, -y);
        return cursor;
    }

    /**
     * Drawing on canvas.
     */
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
            mGraphicSelection.draw(canvas);
        }
    }

    /**
     * Cursor animation function. Switch the alpha between opaque and fully transparent.
     */
    private Runnable cursorAnimation = new Runnable() {
        public void run() {
            if (mCursorVisible) {
                mCursorPaint.setAlpha(mCursorPaint.getAlpha() == 0 ? 0xFF : 0);
                invalidate();
            }
            postDelayed(cursorAnimation, CURSOR_BLINK_TIME);
        }
    };

    /**
     * Show the cursor on the view.
     */
    public void showCursor() {
        mCursorVisible = true;
        invalidate();
    }

    /**
     * Hide the cursor.
     */
    public void hideCursor() {
        mCursorVisible = false;
        invalidate();
    }

    /**
     * Show text selection rectangles.
     */
    public void showSelections() {
        mSelectionsVisible = true;
        invalidate();
    }

    /**
     * Hide text selection rectangles.
     */
    public void hideSelections() {
        mSelectionsVisible = false;
        invalidate();
    }

    /**
     * Show the graphic selection on the view.
     */
    public void showGraphicSelection() {
        mGraphicSelectionVisible = true;
        mGraphicSelectionMove = false;
        mGraphicSelection.reset();
        invalidate();
    }

    /**
     * Hide the graphic selection.
     */
    public void hideGraphicSelection() {
        mGraphicSelectionVisible = false;
        invalidate();
    }

    /**
     * Handle the triggered touch event.
     */
    @Override
    public boolean onTouch(View view, MotionEvent event) {
        switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN: {
                if (mGraphicSelectionVisible) {
                    // Check if inside graphic selection was hit
                    PointF startPosition = new PointF(event.getX(), event.getY());
                    if (mGraphicSelection.contains(startPosition.x, startPosition.y)) {
                        mGraphicSelectionMove = true;
                        mGraphicSelection.dragStart(startPosition);
                        invalidate();
                        return true;
                    }
                    return false;
                }
            }
            case MotionEvent.ACTION_UP: {
                if (mGraphicSelectionVisible && mGraphicSelectionMove) {
                    mGraphicSelectionMove = false;
                    mGraphicSelection.dragEnd(new PointF(event.getX(), event.getY()));
                    invalidate();
                    return true;
                }
            }
            case MotionEvent.ACTION_MOVE: {
                if (mGraphicSelectionVisible && mGraphicSelectionMove) {
                    mGraphicSelection.dragging(new PointF(event.getX(), event.getY()));
                    invalidate();
                    return true;
                }
            }
        }
        return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
