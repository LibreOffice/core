/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.kit;

import android.app.Activity;
import android.content.pm.ApplicationInfo;
import android.util.Log;

import java.io.InputStream;

// final because subclassing would be meaningless.
public final class LibreOfficeKit
{
    private long handle;

    public static void loadStatic() {

    }

    // private constructor because instantiating would be meaningless
    private LibreOfficeKit()
    {
    }

    private static String TAG = "LibreOfficeKit";

    // Native methods in this class are all implemented in
    // sal/android/lo-bootstrap.c as the lo-bootstrap library is loaded with
    // System.loadLibrary() and Android's JNI works only to such libraries, it
    // seems.

    private static native boolean initializeNative(String dataDir, String cacheDir, String apkFile);

    public static native long getLibreOfficeKitHandle();

    // Wrapper for putenv()
    public static native void putenv(String string);

    // A method that starts a thread to redirect stdout and stderr writes to
    // the Android logging mechanism, or stops the redirection.
    public static native void redirectStdio(boolean state);

    static boolean initializeDone = false;

    // This init() method should be called from the upper Java level of
    // LO-based apps.
    public static synchronized void init(Activity activity)
    {
        if (initializeDone)
            return;

        String dataDir = null;

        ApplicationInfo applicationInfo = activity.getApplicationInfo();
        dataDir = applicationInfo.dataDir;
        Log.i(TAG, String.format("Initializing LibreOfficeKit, dataDir=%s\n", dataDir));

        redirectStdio(true);

        String cacheDir = activity.getApplication().getCacheDir().getAbsolutePath();
        String apkFile = activity.getApplication().getPackageResourcePath();

        // If we notice that a fonts.conf file was extracted, automatically
        // set the FONTCONFIG_FILE env var.
        InputStream inputStream = null;
        try {
            inputStream = activity.getAssets().open("unpack/etc/fonts/fonts.conf");
        } catch (java.io.IOException exception) {
        }

        putenv("OOO_DISABLE_RECOVERY=1");

        if (inputStream != null) {
            putenv("FONTCONFIG_FILE=" + dataDir + "/etc/fonts/fonts.conf");
        }

        // TMPDIR is used by osl_getTempDirURL()
        putenv("TMPDIR=" + activity.getCacheDir().getAbsolutePath());

        if (!initializeNative(dataDir, cacheDir, apkFile)) {
            return;
        }

        initializeDone = true;
    }

    // Now with static loading we always have all native code in one native
    // library which we always call liblo-native-code.so, regardless of the
    // app. The library has already been unpacked into /data/data/<app
    // name>/lib at installation time by the package manager.
    static {
        System.loadLibrary("lo-native-code");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
