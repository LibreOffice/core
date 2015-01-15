package org.libreoffice;

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
    public static final int TILE_RERENDER = 9;
    public static final int TOUCH = 10;

    public final int mType;
    public int mPriority = 0;

    public ThumbnailCreator.ThumbnailCreationTask mTask;
    public String mTypeString;
    public int mPartIndex;
    public String mFilename;
    public TileIdentifier mTileId;
    public ComposedTileLayer mComposedTileLayer;
    public boolean mForceRedraw;
    public SubTile mTile;
    public String mTouchType;
    public MotionEvent mMotionEvent;

    public LOEvent(int type) {
        mType = type;
    }

    public LOEvent(int type, int widthPixels, int heightPixels) {
        mType = type;
        mTypeString = "Size Changed: " + widthPixels + " " + heightPixels;
    }

    public LOEvent(int type, ComposedTileLayer composedTileLayer, TileIdentifier tileId, boolean forceRedraw) {
        mType = type;
        mTypeString = "Tile Request";
        mComposedTileLayer = composedTileLayer;
        mTileId = tileId;
        mForceRedraw = forceRedraw;
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
    }

    public LOEvent(int type, ComposedTileLayer composedTileLayer, SubTile tile) {
        mType = type;
        mTypeString = "Tile Rerender";
        mComposedTileLayer = composedTileLayer;
        mTile = tile;
    }

    public LOEvent(int type, String touchType, MotionEvent motionEvent) {
        mType = type;
        mTypeString = "Touch";
        mTouchType = touchType;
        mMotionEvent = motionEvent;
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
