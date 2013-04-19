// -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Native functions that are used by "desktop" approach apps. That means apps
// that have a "real" LibreOffice "main loop" running (headless).

package org.libreoffice.android;

import android.graphics.Bitmap;

public final class AppSupport
{
    /* In desktop */
    public static native void runMain();

    /* In vcl */
    public static native void renderVCL(Bitmap bitmap);
    public static native void registerForDamageCallback(Class destinationClass);
    public static native void setViewSize(int width, int height);
    public static native void key(char c);
    public static native void touch(int action, int x, int y);
    public static native void zoom(float scale, int x, int y);
    public static native void scroll(int x, int y);
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
