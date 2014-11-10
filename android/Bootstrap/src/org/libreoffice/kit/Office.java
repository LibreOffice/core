/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.kit;

public class Office {

    private long handle;

    public Office(long handle) {
        this.handle = handle;
    }

    public native String getError();

    private native long documentLoadNative(String url);

    public Document documentLoad(String url) {
        long documentHandle = documentLoadNative(url);
        Document document = null;
        if (documentHandle > 0) {
            document = new Document(documentHandle);
        }
        return document;
    }

    public native void destroy();
    public native void destroyAndExit();
}
