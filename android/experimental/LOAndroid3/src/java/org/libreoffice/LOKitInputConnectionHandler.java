/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice;

import android.view.KeyEvent;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;

import org.mozilla.gecko.gfx.InputConnectionHandler;

public class LOKitInputConnectionHandler implements InputConnectionHandler {
    private static String LOGTAG = LOKitInputConnectionHandler.class.getSimpleName();

    @Override
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        return null;
    }

    @Override
    public boolean onKeyPreIme(int keyCode, KeyEvent event) {
        LOKitShell.sendKeyEvent(event);
        return false;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        LOKitShell.sendKeyEvent(event);
        return false;
    }

    @Override
    public boolean onKeyLongPress(int keyCode, KeyEvent event) {
        return false;
    }

    @Override
    public boolean onKeyMultiple(int keyCode, int repeatCount, KeyEvent event) {
        LOKitShell.sendKeyEvent(event);
        return false;
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        LOKitShell.sendKeyEvent(event);
        return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
