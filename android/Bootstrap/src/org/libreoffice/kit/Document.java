/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.kit;

import java.nio.ByteBuffer;

public class Document {
    public static final int PART_MODE_SLIDE = 0;
    public static final int PART_MODE_NOTES = 1;

    /**
     * Document types
     */
    public static final int DOCTYPE_TEXT = 0;
    public static final int DOCTYPE_SPREADSHEET = 1;
    public static final int DOCTYPE_PRESENTATION = 2;
    public static final int DOCTYPE_DRAWING = 3;
    public static final int DOCTYPE_OTHER = 4;

    /**
     * Mouse event types
     */
    public static final int MOUSE_EVENT_BUTTON_DOWN = 0;
    public static final int MOUSE_EVENT_BUTTON_UP = 1;
    public static final int MOUSE_EVENT_MOVE = 2;

    /**
     * Key event types
     */
    public static final int KEY_EVENT_PRESS = 0;
    public static final int KEY_EVENT_RELEASE = 1;

    /**
     * State change types
     */
    public static final int BOLD = 0;
    public static final int ITALIC = 1;
    public static final int UNDERLINE = 2;
    public static final int STRIKEOUT = 3;

    public static final int ALIGN_LEFT= 4;
    public static final int ALIGN_CENTER = 5;
    public static final int ALIGN_RIGHT= 6;
    public static final int ALIGN_JUSTIFY= 7;

    /**
     * Callback message types
     */
    public static final int CALLBACK_INVALIDATE_TILES = 0;
    public static final int CALLBACK_INVALIDATE_VISIBLE_CURSOR = 1;
    public static final int CALLBACK_TEXT_SELECTION = 2;
    public static final int CALLBACK_TEXT_SELECTION_START = 3;
    public static final int CALLBACK_TEXT_SELECTION_END = 4;
    public static final int CALLBACK_CURSOR_VISIBLE = 5;
    public static final int CALLBACK_GRAPHIC_SELECTION = 6;
    public static final int CALLBACK_HYPERLINK_CLICKED = 7;
    public static final int CALLBACK_STATE_CHANGED = 8;
    public static final int CALLBACK_STATUS_INTICATOR_START = 9;
    public static final int CALLBACK_STATUS_INTICATOR_SET_VALUE = 10;
    public static final int CALLBACK_STATUS_INTICATOR_FINISH = 11;
    public static final int CALLBACK_SEARCH_NOT_FOUND = 12;
    public static final int CALLBACK_DOCUMENT_SIZE_CHANGED = 13;
    public static final int CALLBACK_SET_PART = 14;
    public static final int CALLBACK_SEARCH_RESULT_SELECTION = 15;

    /**
     * Set text selection types
     */
    public static final int SET_TEXT_SELECTION_START = 0;
    public static final int SET_TEXT_SELECTION_END = 1;
    public static final int SET_TEXT_SELECTION_RESET = 2;

    /**
     * Set graphic selection types
     */
    public static final int SET_GRAPHIC_SELECTION_START = 0;
    public static final int SET_GRAPHIC_SELECTION_END = 1;

    /**
     * Mouse button type
     */
    public static final int MOUSE_BUTTON_LEFT = 1;
    public static final int MOUSE_BUTTON_MIDDLE = 2;
    public static final int MOUSE_BUTTON_RIGHT = 4;

    public static final int KEYBOARD_MODIFIER_NONE = 0x0000;
    public static final int KEYBOARD_MODIFIER_SHIFT = 0x1000;
    public static final int KEYBOARD_MODIFIER_MOD1 = 0x2000;
    public static final int KEYBOARD_MODIFIER_MOD2 = 0x4000;
    public static final int KEYBOARD_MODIFIER_MOD3 = 0x8000;

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
     * Callback triggered through JNI to indicate that a new signal
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

    public native String getPartPageRectangles();

    public native long getDocumentHeight();

    public native long getDocumentWidth();

    private native int getDocumentTypeNative();

    public native void setClientZoom(int nTilePixelWidth, int nTilePixelHeight, int nTileTwipWidth, int nTileTwipHeight);

    private native void saveAs(String url, String format, String options);

    private native void paintTileNative(ByteBuffer buffer, int canvasWidth, int canvasHeight, int tilePositionX, int tilePositionY, int tileWidth, int tileHeight);

    public int getDocumentType() {
        return getDocumentTypeNative();
    }

    public void paintTile(ByteBuffer buffer, int canvasWidth, int canvasHeight, int tilePositionX, int tilePositionY, int tileWidth, int tileHeight) {
        paintTileNative(buffer, canvasWidth, canvasHeight, tilePositionX, tilePositionY, tileWidth, tileHeight);
    }

    public native void initializeForRendering();

    /**
     * Post a key event to LibreOffice.
     * @param type - type of key event
     * @param charCode - the Unicode character generated by this event or 0.
     * @param keyCode - the integer code representing the key of the event (non-zero for control keys).
     */
    public native void postKeyEvent(int type, int charCode, int keyCode);

    /**
     * Post a mouse event to LOK
     * @param type - mouse event type
     * @param x - x coordinate
     * @param y - y coordinate
     * @param count - number of events
     */
    public native void postMouseEvent(int type, int x, int y, int count, int button, int modifier);

    /**
     * Post a .uno: command to LOK
     * @param command - the command, like ".uno:Bold"
     * @param arguments
     */
    public native void postUnoCommand(String command, String arguments);

    /**
     * Change text selection.
     * @param type - text selection type
     * @param x - x coordinate
     * @param y - y coordinate
     */
    public native void setTextSelection(int type, int x, int y);

    /**
     * Change graphic selection.
     * @param type - graphic selection type
     * @param x - x coordinate
     * @param y - y coordinate
     */
    public native void setGraphicSelection(int type, int x, int y);

    /**
     * Reset current (any kind of) selection.
     */
    public native void resetSelection();

    public native String getCommandValues(String command);

    /**
     * Callback to retrieve messages from LOK
     */
    public interface MessageCallback {
        /**
         * Invoked when a message is retrieved from LOK
         * @param signalNumber - signal type / number
         * @param payload - retrieved for the signal
         */
        void messageRetrieved(int signalNumber, String payload);
    }

}
