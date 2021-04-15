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
     * Save the current document under the given path.
     * @param takeOwnership Whether to take ownership of the new file,
     *                      i.e. whether the current document is changed to the
     *                      newly saved document (takeOwnership = true),
     *                      as compared to just saving a copy of the current document
     *                      or exporting to a different file format.
     *                      Must be 'false' when using this method for export to e.g. PNG or PDF.
     */
    void saveDocumentAs(String filePath, String format, boolean takeOwnership);

    /**
     * Saves the current document under the given path,
     * using the default file format.
     * @param takeOwnership (s. documentation for
     *                      'saveDocumentAs(String filePath, String format, boolean takeOwnership)')
     */
    void saveDocumentAs(String filePath, boolean takeOwnership);

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
     * Returns true if the current open document is a drawing.
     */
    boolean isDrawing();

    /**
     * Returns true if the current open document is a text document.
     */
    boolean isTextDocument();

    /**
     * Returns true if the current open document is a spreadsheet.
     */
    boolean isSpreadsheet();

    /**
     * Returns true if the current open document is a presentation
     */
    boolean isPresentation();

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
     * This is the actual reference to the function in LOK, used for getting notified when uno:save event finishes
     * @param command
     * @param arguments
     * @param notifyWhenFinished
     */
    void postUnoCommand(String command, String arguments, boolean notifyWhenFinished);

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
     * get selected text
     * @param mimeType
     */
    String getTextSelection(String mimeType);

    /**
     * copy
     * @param mimeType
     * @param data
     * @return
     */
    boolean paste(String mimeType, String data);
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
     * Send a request to change end the change of graphic selection...
     */
    void setGraphicSelectionEnd(PointF documentCoordinate);

    /**
     * Set the new page size of the document when changed
     */
    void setDocumentSize(int pageWidth, int pageHeight);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
