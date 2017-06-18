package org.libreoffice.overlay;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.PointF;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

import org.json.JSONException;
import org.json.JSONObject;
import org.libreoffice.LOEvent;
import org.libreoffice.LOKitShell;
import org.libreoffice.canvas.CalcHeaderCell;
import org.mozilla.gecko.gfx.ImmutableViewportMetrics;
import org.mozilla.gecko.gfx.LayerView;

import java.util.ArrayList;
import java.util.Collections;

public class CalcHeadersView extends View implements View.OnTouchListener {
    private static final String LOGTAG = CalcHeadersView.class.getSimpleName();

    private boolean mInitialized;
    private LayerView mLayerView;
    private boolean mIsRow; // true if this is for row headers, false for column
    private ArrayList<String> mLabels;
    private ArrayList<Float> mDimens;
    private RectF mCellCursorRect;
    private PointF pointOfTouch;
    private boolean mPendingRowOrColumnSelectionToShowUp;

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

        // Draw headers
        boolean inRangeOfVisibleHeaders = false; // a helper variable for skipping unnecessary onDraw()'s
        float top,bottom,left,right;
        for (int i = 1; i < mLabels.size(); i++) {
            if (mIsRow) {
                top = -origin.y + zoom*mDimens.get(i-1);
                bottom = -origin.y + zoom*mDimens.get(i);
                if (top <= getHeight() && bottom >= 0) {
                    inRangeOfVisibleHeaders = true;
                    if (mCellCursorRect != null && bottom > mCellCursorRect.top - origin.y && top < mCellCursorRect.bottom - origin.y) {
                        // if cell is within current selected portion
                        new CalcHeaderCell(0f, top, getWidth(), bottom - top, mLabels.get(i), true).onDraw(canvas);
                    } else {
                        new CalcHeaderCell(0f, top, getWidth(), bottom - top, mLabels.get(i), false).onDraw(canvas);
                    }
                } else {
                    if (inRangeOfVisibleHeaders) {
                        break;
                    }
                }
            } else {
                left = -origin.x + zoom*mDimens.get(i-1);
                right = -origin.x + zoom*mDimens.get(i);
                if (left <= getWidth() && right >= 0) {
                    if (mCellCursorRect != null && right > mCellCursorRect.left - origin.x && left < mCellCursorRect.right - origin.x) {
                        // if cell is within current selected portion
                        new CalcHeaderCell(left, 0f, right - left, getHeight(), mLabels.get(i), true).onDraw(canvas);
                    } else {
                        new CalcHeaderCell(left, 0f, right - left, getHeight(), mLabels.get(i), false).onDraw(canvas);
                    }
                } else {
                    if (inRangeOfVisibleHeaders) {
                        break;
                    }
                }
            }
        }
    }

    /**
     * Handle the triggered touch event.
     */
    @Override
    public boolean onTouch(View view, MotionEvent event) {
        PointF point = new PointF(event.getX(), event.getY());
        switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                pointOfTouch = point;
                return true;
            case MotionEvent.ACTION_UP:
                if (pointOfTouch != null) {
                    highlightRowOrColumn();
                }
        }
        return false;
    }

    /**
     * Handle a single tap event on a header cell.
     * Selects whole row/column.
     */
    private void highlightRowOrColumn() {
        int searchedIndex, index;
        ImmutableViewportMetrics metrics = mLayerView.getViewportMetrics();
        float zoom = metrics.getZoomFactor();
        PointF origin = metrics.getOrigin();
        if (mIsRow) {
            searchedIndex = Collections.binarySearch(mDimens, (pointOfTouch.y+origin.y)/zoom);
        } else {
            searchedIndex = Collections.binarySearch(mDimens, (pointOfTouch.x+origin.x)/zoom);
        }
        // converting searched index to real index on headers
        if (searchedIndex < 0) {
            index = - searchedIndex - 2;
        } else {
            index = searchedIndex;
        }
        try {
            JSONObject rootJson = new JSONObject();
            addProperty(rootJson, "Modifier", "unsigned short", "0");
            if (mIsRow) {
                addProperty(rootJson, "Row", "unsigned short", String.valueOf(index));
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:SelectRow", rootJson.toString()));
            } else {
                addProperty(rootJson, "Col", "unsigned short", String.valueOf(index));
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:SelectColumn", rootJson.toString()));
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
        // At this point, InvalidationHandler.java will have received two callbacks.
        // One is for text selection (first) and the other for cell selection (second).
        // The second will override the first on headers which is not wanted.
        // setPendingRowOrColumnSelectionToShowUp(true) will skip the second call.
        setPendingRowOrColumnSelectionToShowUp(true);
        pointOfTouch = null;
    }

    public void setPendingRowOrColumnSelectionToShowUp(boolean b) {
        mPendingRowOrColumnSelectionToShowUp = b;
    }

    public boolean pendingRowOrColumnSelectionToShowUp() {
        return mPendingRowOrColumnSelectionToShowUp;
    }

    private void addProperty(JSONObject json, String parentValue, String type, String value) throws JSONException {
        JSONObject child = new JSONObject();
        child.put("type", type);
        child.put("value", value);
        json.put(parentValue, child);
    }

    public void setHeaders(ArrayList<String> labels, ArrayList<Float> dimens) {
        mLabels = labels;
        mDimens = dimens;
    }

    public void setHeaderSelection(RectF cellCursorRect) {
        mCellCursorRect = cellCursorRect;
    }
}
