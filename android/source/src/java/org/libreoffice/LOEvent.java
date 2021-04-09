/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice;

import android.graphics.PointF;
import android.graphics.RectF;
import android.view.KeyEvent;

import org.libreoffice.canvas.SelectionHandle;
import org.mozilla.gecko.gfx.ComposedTileLayer;

/**
 * Events and data that is queued and processed by LOKitThread.
 */
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
    public static final int UNO_COMMAND = 14;
    public static final int LOAD_NEW = 16;
    public static final int SAVE_AS = 17;
    public static final int UPDATE_PART_PAGE_RECT = 18;
    public static final int UPDATE_ZOOM_CONSTRAINTS = 19;
    public static final int UPDATE_CALC_HEADERS = 20;
    public static final int REFRESH = 21;
    public static final int PAGE_SIZE_CHANGED = 22;
    public static final int UNO_COMMAND_NOTIFY = 23;
    public static final int SAVE_COPY_AS = 24;


    public final int mType;
    public int mPriority = 0;
    private String mTypeString;

    public ThumbnailCreator.ThumbnailCreationTask mTask;
    public int mPartIndex;
    public String mString;
    public String filePath;
    public String fileType;
    public ComposedTileLayer mComposedTileLayer;
    public String mTouchType;
    public PointF mDocumentCoordinate;
    public KeyEvent mKeyEvent;
    public RectF mInvalidationRect;
    public SelectionHandle.HandleType mHandleType;
    public String mValue;
    public int mPageWidth;
    public int mPageHeight;
    public boolean mNotify;

    public LOEvent(int type) {
        mType = type;
    }

    public LOEvent(int type, ComposedTileLayer composedTileLayer) {
        mType = type;
        mTypeString = "Tile Reevaluation";
        mComposedTileLayer = composedTileLayer;
    }

    public LOEvent(int type, String someString) {
        mType = type;
        mTypeString = "String";
        mString = someString;
        mValue = null;
    }

    public LOEvent(int type, String someString, boolean notify) {
        mType = type;
        mTypeString = "String";
        mString = someString;
        mValue = null;
        mNotify = notify;
    }

    public LOEvent(int type, String someString, String value, boolean notify) {
        mType = type;
        mTypeString = "String";
        mString = someString;
        mValue = value;
        mNotify = notify;
    }

    public LOEvent(int type, String key, String value) {
        mType = type;
        mTypeString = "key / value";
        mString = key;
        mValue = value;
    }

    public LOEvent(String filePath, int type) {
        mType = type;
        mTypeString = "Load";
        this.filePath = filePath;
    }

    public LOEvent(String filePath, String fileType, int type) {
        mType = type;
        mTypeString = "Load New/Save As";
        this.filePath = filePath;
        this.fileType = fileType;
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

    public LOEvent(int type, SelectionHandle.HandleType handleType, PointF documentCoordinate) {
        mType = type;
        mHandleType = handleType;
        mDocumentCoordinate = documentCoordinate;
    }

    public LOEvent(int type, int pageWidth, int pageHeight){
        mType = type;
        mPageWidth = pageWidth;
        mPageHeight = pageHeight;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
