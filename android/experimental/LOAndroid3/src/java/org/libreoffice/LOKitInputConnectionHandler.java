package org.libreoffice;

import android.view.KeyEvent;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;

import org.mozilla.gecko.gfx.InputConnectionHandler;

public class LOKitInputConnectionHandler implements InputConnectionHandler {
    @Override
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        return null;
    }

    @Override
    public boolean onKeyPreIme(int keyCode, KeyEvent event) {
        return false;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        LOKitShell.sendKeyPressEvent(event);
        return true;
    }

    @Override
    public boolean onKeyLongPress(int keyCode, KeyEvent event) {
        return false;
    }

    @Override
    public boolean onKeyMultiple(int keyCode, int repeatCount, KeyEvent event) {
        return false;
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        LOKitShell.sendKeyReleaseEvent(event);
        return false;
    }
}
