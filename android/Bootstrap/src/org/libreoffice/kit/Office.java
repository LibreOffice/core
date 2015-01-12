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

public class Office {
    public static final int KEY_PRESS = 0;
    public static final int KEY_RELEASE = 1;

    private ByteBuffer handle;

    public Office(ByteBuffer handle) {
        this.handle = handle;
    }

    public native String getError();

    private native ByteBuffer documentLoadNative(String url);

    public Document documentLoad(String url) {
        ByteBuffer documentHandle = documentLoadNative(url);
        Document document = null;
        if (documentHandle != null) {
            document = new Document(documentHandle);
        }
        return document;
    }

    /**
     * Post a key event to LibreOffice.
     * @param type - type of key event
     * @param code - key event code
     */
    public native void postKeyEvent(int type, int code);

    public native void destroy();
    public native void destroyAndExit();
}
