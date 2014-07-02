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

    private final long handle;

    private native void setPartNative(long handle, int part);
    private native int getNumberOfPartsNative(long handle);
    private native int getDocumentTypeNative(long handle);
    private native void paintTileNative(long handle, ByteBuffer buffer, int canvasWidth, int canvasHeight, int tilePositionX, int tilePositionY, int tileWidth, int tileHeight);
    private native long getDocumentHeightNative(long handle);
    private native long getDocumentWidthNative(long handle);

    public Document(long handle) {
        this.handle = handle;
    }

    public boolean saveAs(String url) {
        return false;
    }

    public int getDocumentType() {
        return getDocumentTypeNative(handle);
    }

    public int getNumberOfParts() {
        return getNumberOfPartsNative(handle);
    }

    public void setPart(int part) {
        setPartNative(handle, part);
    }

    public void paintTile(ByteBuffer buffer, int canvasWidth, int canvasHeight, int tilePositionX, int tilePositionY, int tileWidth, int tileHeight) {
        paintTileNative(handle, buffer, canvasWidth, canvasHeight, tilePositionX, tilePositionY, tileWidth, tileHeight);
    }

    public long getDocumentWidth() {
        return getDocumentWidthNative(handle);
    }

    public long getDocumentHeight() {
        return getDocumentHeightNative(handle);
    }

}
