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

/**
 * Implementation of InputConnectionHandler. When a key event happens it is
 * directed to this class which is then directed further to LOKitThread.
 */
public class LOKitInputConnectionHandler implements InputConnectionHandler {
    private static String LOGTAG = LOKitInputConnectionHandler.class.getSimpleName();

    @Override
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        return null;
    }

    /**
     * When key pre-Ime happens.
     */
    @Override
    public boolean onKeyPreIme(int keyCode, KeyEvent event) {
        LOKitShell.sendKeyEvent(event);
        return false;
    }

    /**
     * When key down event happens.
     */
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        LOKitShell.sendKeyEvent(event);
        return false;
    }

    /**
     * When key long press event happens.
     */
    @Override
    public boolean onKeyLongPress(int keyCode, KeyEvent event) {
        return false;
    }

    /**
     * When key multiple event happens. Key multiple event is triggered when
     * non-ascii characters are entered on soft keyboard.
     */
    @Override
    public boolean onKeyMultiple(int keyCode, int repeatCount, KeyEvent event) {
        LOKitShell.sendKeyEvent(event);
        return false;
    }

    /**
     * When key up event happens.
     */
    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        LOKitShell.sendKeyEvent(event);
        return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
