/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.kit;

import android.util.Log;

import java.nio.ByteBuffer;

public class Document {
    public static final int PART_MODE_DEFAULT = 0;
    public static final int PART_MODE_SLIDE = 1;
    public static final int PART_MODE_NOTES = 2;
    public static final int PART_MODE_SLIDENOTES = 3;
    public static final int PART_MODE_EMBEDDEDOBJ = 4;

    public static final int DOCTYPE_TEXT = 0;
    public static final int DOCTYPE_SPREADSHEET = 1;
    public static final int DOCTYPE_PRESENTATION = 2;
    public static final int DOCTYPE_DRAWING = 3;
    public static final int DOCTYPE_OTHER = 4;

    private final ByteBuffer handle;
    private MessageCallback messageCallback = null;

    public Document(ByteBuffer handle) {
        this.handle = handle;
        bindMessageCallback();
    }

    public void setMessageCallback(MessageCallback messageCallback) {
        this.messageCallback = messageCallback;
    }

    /**
     * Callback triggered through JNI to indicate that a new singal
     * from LibreOfficeKit was retrieved.
     */
    private void messageRetrieved(int signalNumber, String payload) {
        if (messageCallback != null) {
            messageCallback.messageRetrieved(signalNumber, payload);
        }
    }

    /**
     * Bind the signal callback in LOK.
     */
    private native void bindMessageCallback();

    public native void destroy();

    public native int getPart();

    public native void setPart(int partIndex);

    public native int getParts();

    public native String getPartName(int partIndex);

    public native void setPartMode(int partMode);

    public native long getDocumentHeight();

    public native long getDocumentWidth();

    private native int getDocumentTypeNative();

    private native void saveAs(String url, String format, String options);

    private native void paintTileNative(ByteBuffer buffer, int canvasWidth, int canvasHeight, int tilePositionX, int tilePositionY, int tileWidth, int tileHeight);

    public int getDocumentType() {
        return getDocumentTypeNative();
    }

    public void paintTile(ByteBuffer buffer, int canvasWidth, int canvasHeight, int tilePositionX, int tilePositionY, int tileWidth, int tileHeight) {
        paintTileNative(buffer, canvasWidth, canvasHeight, tilePositionX, tilePositionY, tileWidth, tileHeight);
    }

    public native void initializeForRendering();

    public interface MessageCallback {
        void messageRetrieved(int signalNumber, String payload);
    }

}
