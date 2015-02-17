package org.libreoffice;

import android.graphics.PointF;
import android.graphics.RectF;
import android.view.KeyEvent;
import android.view.MotionEvent;

import org.mozilla.gecko.gfx.ComposedTileLayer;
import org.mozilla.gecko.gfx.IntSize;
import org.mozilla.gecko.gfx.SubTile;

public class LOEvent implements Comparable<LOEvent> {
    public static final int SIZE_CHANGED = 1;
    public static final int TILE_SIZE = 2;
    public static final int CHANGE_PART = 3;
    public static final int LOAD = 4;
    public static final int CLOSE = 5;
    public static final int REDRAW = 6;
    public static final int TILE_REQUEST = 7;
    public static final int THUMBNAIL = 8;
    public static final int TILE_INVALIDATION = 9;
    public static final int TOUCH = 10;
    public static final int KEY_EVENT = 11;

    public final int mType;
    public int mPriority = 0;

    public ThumbnailCreator.ThumbnailCreationTask mTask;
    public String mTypeString;
    public int mPartIndex;
    public String mFilename;
    public SubTile mTile;
    public ComposedTileLayer mComposedTileLayer;
    public String mTouchType;
    public MotionEvent mMotionEvent;
    public PointF mDocumentTouchCoordinate;
    public String mKeyEventType;
    public KeyEvent mKeyEvent;
    public RectF mInvalidationRect;

    public LOEvent(int type) {
        mType = type;
    }

    public LOEvent(int type, int widthPixels, int heightPixels) {
        mType = type;
        mTypeString = "Size Changed: " + widthPixels + " " + heightPixels;
    }

    public LOEvent(int type, ComposedTileLayer composedTileLayer, SubTile tile) {
        mType = type;
        mTypeString = "Tile Request";
        mComposedTileLayer = composedTileLayer;
        mTile = tile;
    }

    public LOEvent(int type, String filename) {
        mType = type;
        mTypeString = "Filename";
        mFilename = filename;
    }

    public LOEvent(int type, IntSize tileSize) {
        mType = type;
        mTypeString = "Tile size";
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

    public LOEvent(int type, String touchType, MotionEvent motionEvent, PointF documentTouchCoordinate) {
        mType = type;
        mTypeString = "Touch";
        mTouchType = touchType;
        mMotionEvent = motionEvent;
        mDocumentTouchCoordinate = documentTouchCoordinate;
    }

    public LOEvent(int type, String keyEventType, KeyEvent keyEvent) {
        mType = type;
        mTypeString = "Key Event";
        mKeyEventType = keyEventType;
        mKeyEvent = keyEvent;
    }

    public LOEvent(int type, RectF rect) {
        mType = type;
        mTypeString = "Tile Invalidation";
        mInvalidationRect = rect;
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
