/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice;


import android.graphics.Bitmap;
import android.graphics.PointF;
import android.view.KeyEvent;

import org.mozilla.gecko.gfx.CairoImage;
import org.mozilla.gecko.gfx.IntSize;

/**
 * Provides the tiles and other document information.
 */
public interface TileProvider {
    /**
     * Returns the page width in pixels.
     */
    int getPageWidth();

    /**
     * Returns the page height in pixels.
     */
    int getPageHeight();

    boolean isReady();

    CairoImage createTile(float x, float y, IntSize tileSize, float zoom);

    /**
     * Rerender and overwrite tile's image buffer directly
     */
    void rerenderTile(CairoImage image, float x, float y, IntSize tileSize, float zoom);

    /**
     * Change the document part to the one specified by the partIndex input parameter.
     *
     * @param partIndex - part index to change to
     */
    void changePart(int partIndex);

    /**
     * Get the current document part number.
     *
     * @return
     */
    int getCurrentPartNumber();

    /**
     * Get the total number of parts.
     */
    int getPartsCount();

    Bitmap thumbnail(int size);

    /**
     * Closes the document.
     */
    void close();

    /**
     * Returns true if the current open document is a text document.
     */
    boolean isTextDocument();

    /**
     * Returns true if the current open document is a spreadsheet.
     */
    boolean isSpreadsheet();

    /**
     * Trigger a key event.
     *
     * @param keyEvent - contains information about key event
     */
    void sendKeyEvent(KeyEvent keyEvent);

    /**
     * Trigger a mouse button down event.
     *
     * @param documentCoordinate - coordinate relative to the document where the mouse button should be triggered
     * @param numberOfClicks     - number of clicks (1 - single click, 2 - double click)
     */
    void mouseButtonDown(PointF documentCoordinate, int numberOfClicks, float zoomFactor);


    /**
     * Trigger a swipe left event.
     */
    void onSwipeLeft();

    /**
     * Trigger a swipe left event.
     */
    void onSwipeRight();

    /**
     * Trigger a mouse button up event.
     *
     * @param documentCoordinate - coordinate relative to the document where the mouse button should be triggered
     * @param numberOfClicks     - number of clicks (1 - single click, 2 - double click)
     */
    void mouseButtonUp(PointF documentCoordinate, int numberOfClicks, float zoomFactor);

    /**
     * Post a UNO command to LOK.
     *
     * @param command - the .uno: command, like ".uno:Bold"
     */
    void postUnoCommand(String command, String arguments);

    /**
     * Send text selection start coordinate.
     * @param documentCoordinate
     */
    void setTextSelectionStart(PointF documentCoordinate);

    /**
     * Send text selection end coordinate.
     * @param documentCoordinate
     */
    void setTextSelectionEnd(PointF documentCoordinate);

    /**
     * Send text selection reset coordinate.
     * @param documentCoordinate
     */
    void setTextSelectionReset(PointF documentCoordinate);

    /**
     * Send a request to change start the change of graphic selection.
     */
    void setGraphicSelectionStart(PointF documentCoordinate);

    /**
     * Send a request to change end the change of graphic selection..
     */
    void setGraphicSelectionEnd(PointF documentCoordinate);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
