// -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

package org.libreoffice.android;

import android.app.Activity;
import android.content.pm.ApplicationInfo;
import android.util.Log;

import java.io.InputStream;

// final because subclassing would be meaningless.
public final class Bootstrap
{
    // private constructor because instantiating would be meaningless
    private Bootstrap()
    {
    }

    private static String TAG = "lo-bootstrap";

    // Native methods in this class are all implemented in
    // sal/android/lo-bootstrap.c as the lo-bootstrap library is loaded with
    // System.loadLibrary() and Android's JNI works only to such libraries, it
    // seems.

    private static native boolean setup(String dataDir,
                                        String cacheDir,
                                        String apkFile);

    // Wrapper for getpid()
    public static native int getpid();

    // Wrapper for system()
    public static native void system(String cmdline);

    // Wrapper for putenv()
    public static native void putenv(String string);

    // A wrapper for InitVCL() in libvcl (svmain.cxx), called indirectly
    // through the lo-bootstrap library
    public static native void initVCL();

    // A wrapper for osl_setCommandArgs(). Before calling
    // osl_setCommandArgs(), argv[0] is prefixed with the parent directory of
    // where the lo-bootstrap library is.
    public static native void setCommandArgs(String[] argv);

    // A method that starts a thread to redirect stdout and stderr writes to
    // the Android logging mechanism, or stops the redirection.
    public static native void redirect_stdio(boolean state);

    static boolean setup_done = false;

    // This setup() method should be called from the upper Java level of
    // LO-based apps.
    public static synchronized void setup(Activity activity)
    {
        if (setup_done)
            return;

        setup_done = true;

        String dataDir = null;

        ApplicationInfo ai = activity.getApplicationInfo();
        dataDir = ai.dataDir;
        Log.i(TAG, String.format("dataDir=%s\n", dataDir));

        redirect_stdio(true);

        if (!setup(dataDir,
                   activity.getApplication().getCacheDir().getAbsolutePath(),
                   activity.getApplication().getPackageResourcePath()))
            return;

        // If we notice that a fonts.conf file was extracted, automatically
        // set the FONTCONFIG_FILE env var.
        InputStream i;
        try {
            i = activity.getAssets().open("unpack/etc/fonts/fonts.conf");
        }
        catch (java.io.IOException e) {
            i = null;
        }
        putenv("OOO_DISABLE_RECOVERY=1");
        if (i != null)
            putenv("FONTCONFIG_FILE=" + dataDir + "/etc/fonts/fonts.conf");

        // TMPDIR is used by osl_getTempDirURL()
        putenv("TMPDIR=" + activity.getCacheDir().getAbsolutePath());
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
