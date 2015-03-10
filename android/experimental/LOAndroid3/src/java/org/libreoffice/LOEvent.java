package org.libreoffice;

import android.graphics.PointF;
import android.graphics.RectF;
import android.view.KeyEvent;
import android.view.MotionEvent;

import org.mozilla.gecko.TextSelectionHandle;
import org.mozilla.gecko.gfx.ComposedTileLayer;
import org.mozilla.gecko.gfx.IntSize;
import org.mozilla.gecko.gfx.SubTile;

public class LOEvent implements Comparable<LOEvent> {
    public static final int SIZE_CHANGED = 1;
    public static final int CHANGE_PART = 2;
    public static final int LOAD = 3;
    public static final int CLOSE = 4;
    public static final int TILE_REEVALUATION_REQUEST = 5;
    public static final int THUMBNAIL = 6;
    public static final int TILE_INVALIDATION = 7;
    public static final int TOUCH = 8;
    public static final int KEY_EVENT = 9;
    public static final int CHANGE_HANDLE_POSITION = 10;
    public static final int SWIPE_RIGHT = 11;
    public static final int SWIPE_LEFT = 12;
    public static final int NAVIGATION_CLICK = 13;

    public final int mType;
    public int mPriority = 0;
    public String mTypeString;

    public ThumbnailCreator.ThumbnailCreationTask mTask;
    public int mPartIndex;
    public String mFilename;
    public ComposedTileLayer mComposedTileLayer;
    public String mTouchType;
    public PointF mDocumentCoordinate;
    public KeyEvent mKeyEvent;
    public RectF mInvalidationRect;
    public TextSelectionHandle.HandleType mHandleType;

    public LOEvent(int type) {
        mType = type;
    }

    public LOEvent(int type, ComposedTileLayer composedTileLayer) {
        mType = type;
        mTypeString = "Tile Reevaluation";
        mComposedTileLayer = composedTileLayer;
    }

    public LOEvent(int type, String filename) {
        mType = type;
        mTypeString = "Filename";
        mFilename = filename;
    }

    public LOEvent(int type, int partIndex) {
        mType = type;
        mPartIndex = partIndex;
        mTypeString = "Change part";
    }

    public LOEvent(int type, ThumbnailCreator.ThumbnailCreationTask task) {
        mType = type;
        mTask = task;
        mTypeString = "Thumbnail";
    }

    public LOEvent(int type, String touchType, PointF documentTouchCoordinate) {
        mType = type;
        mTypeString = "Touch";
        mTouchType = touchType;
        mDocumentCoordinate = documentTouchCoordinate;
    }

    public LOEvent(int type, KeyEvent keyEvent) {
        mType = type;
        mTypeString = "Key Event";
        mKeyEvent = keyEvent;
    }

    public LOEvent(int type, RectF rect) {
        mType = type;
        mTypeString = "Tile Invalidation";
        mInvalidationRect = rect;
    }

    public LOEvent(int type, TextSelectionHandle.HandleType handleType, PointF documentCoordinate) {
        mType = type;
        mHandleType = handleType;
        mDocumentCoordinate = documentCoordinate;
    }

    public String getTypeString() {
        if (mTypeString == null) {
            return "Event type: " + mType;
        }
        return mTypeString;
    }

    @Override
    public int compareTo(LOEvent another) {
        return mPriority - another.mPriority;
    }

}
