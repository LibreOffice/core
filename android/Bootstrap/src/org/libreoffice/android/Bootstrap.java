// -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-

// Version: MPL 1.1 / GPLv3+ / LGPLv3+
//
// The contents of this file are subject to the Mozilla Public License Version
// 1.1 (the "License"); you may not use this file except in compliance with
// the License or as specified alternatively below. You may obtain a copy of
// the License at http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
// for the specific language governing rights and limitations under the
// License.
//
// Major Contributor(s):
// Copyright (C) 2011 Tor Lillqvist <tml@iki.fi> (initial developer)
// Copyright (C) 2011 SUSE Linux http://suse.com (initial developer's employer)
//
// All Rights Reserved.
//
// For minor contributions see the git repository.
//
// Alternatively, the contents of this file may be used under the terms of
// either the GNU General Public License Version 3 or later (the "GPLv3+"), or
// the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
// in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
// instead of those above.

package org.libreoffice.android;

import android.app.Activity;
import android.app.NativeActivity;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;

import fi.iki.tml.CommandLine;

import java.io.File;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Scanner;

// We extend NativeActivity so that we can get at the intent of the
// activity and its extra parameters, that we use to tell us what
// actual LibreOffice "program" to run. I.e. something that on desktop
// OSes would be a program, but for Android is actually built as a
// shared object, with a "lo_main" function.

public class Bootstrap extends NativeActivity
{
    private static String TAG = "lo-bootstrap";

    // Native methods in this class are all implemented in
    // sal/android/lo-bootstrap.c as the lo-bootstrap library is loaded with
    // System.loadLibrary() and Android's JNI works only to such libraries, it
    // seems.

    private static native boolean setup(String dataDir,
                                        String apkFile,
                                        String[] ld_library_path);

    public static native boolean setup(int lo_main_ptr,
                                       Object lo_main_argument,
                                       int lo_main_delay);

    // This is not just a wrapper for the C library dlopen(), but also
    // loads recursively dependent libraries.
    public static native int dlopen(String library);

    // This is just a wrapper for the C library dlsym().
    public static native int dlsym(int handle, String symbol);

    // To be called after you are sure libgnustl_shared.so
    // has been loaded
    static native void patch_libgnustl_shared();

    // Extracts files in the .apk that need to be extraced into the app's tree
    static native void extract_files();

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

    // A wrapper for InitUCBHelper() in the ucbhelper library
    // (contentbroker.cxx), also this called indirectly through the lo-bootstrap library
    public static native void initUCBHelper();

    // A wrapper for createWindowFoo() in the vcl library
    public static native int createWindowFoo();

    // A method that starts a thread to redirect stdout and stderr writes to
    // the Android logging mechanism, or stops the redirection.
    public static native boolean redirect_stdio(boolean state);

    // The DIB returned by css.awt.XBitmap.getDIB is in BGR_888 form, at least
    // for Writer documents. We need it in Android's Bitmap.Config.ARGB_888
    // format, which actually is RGBA_888, whee... At least in Android 4.0.3,
    // at least on my device. No idea if it is always like that or not, the
    // documentation sucks.
    public static native void twiddle_BGR_to_RGBA(byte[] source, int offset, int width, int height, ByteBuffer destination);

    public static native void force_full_alpha_array(byte[] array, int offset, int length);

    public static native void force_full_alpha_bb(ByteBuffer buffer, int offset, int length);

    public static native long new_byte_buffer_wrapper(ByteBuffer bbuffer);

    public static native void delete_byte_buffer_wrapper(long bbw);

    static boolean setup_done = false;

    // This setup() method is called 1) in apps that use *this* class as their activity from onCreate(),
    // and 2) should be called from other kinds of LO code using apps.
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

        String llp = System.getenv("LD_LIBRARY_PATH");
        if (llp == null)
            llp = "/vendor/lib:/system/lib";

        String[] llpa = llp.split(":");

        if (!setup(dataDir, activity.getApplication().getPackageResourcePath(), llpa))
            return;

        // We build LO code against the shared GNU C++ library
        dlopen("libgnustl_shared.so");
        // and need to patch it.
        patch_libgnustl_shared();

        // Extract files from the .apk that can't be used mmapped directly from it
        extract_files();

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

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        setup(this);

        String mainLibrary = getIntent().getStringExtra("lo-main-library");

        if (mainLibrary == null)
            mainLibrary = "libcppunittester";

        mainLibrary += ".so";

        Log.i(TAG, String.format("mainLibrary=%s", mainLibrary));

        // Get "command line" to pass to the LO "program"
        String cmdLine = getIntent().getStringExtra("lo-main-cmdline");

        if (cmdLine == null) {
            String indirectFile = getIntent().getStringExtra("lo-main-indirect-cmdline");
            if (indirectFile != null) {
                try {
                    // Somewhat stupid but short way to read a file into a string
                    cmdLine = new Scanner(new File(indirectFile), "UTF-8").useDelimiter("\\A").next().trim();
                }
                catch (java.io.FileNotFoundException e) {
                    Log.i(TAG, String.format("Could not read %s: %s",indirectFile, e.toString()));
                }
            }

            if (cmdLine == null)
                cmdLine = "";
        }

        Log.i(TAG, String.format("cmdLine=%s", cmdLine));

        String[] argv = CommandLine.split(cmdLine);

        // Handle env var assignments in the command line.
        while (argv.length > 0 &&
               argv[0].matches("[A-Z_]+=.*")) {
            putenv(argv[0]);
            argv = Arrays.copyOfRange(argv, 1, argv.length);
        }

        // argv[0] will be replaced by android_main() in lo-bootstrap.c by the
        // pathname of the mainLibrary.
        String[] newargv = new String[argv.length + 1];
        newargv[0] = "dummy-program-name";
        System.arraycopy(argv, 0, newargv, 1, argv.length);
        argv = newargv;

        // Load the LO "program" here and look up lo_main
        int loLib = dlopen(mainLibrary);

        if (loLib == 0) {
            Log.i(TAG, String.format("Error: could not load %s", mainLibrary));
            mainLibrary = "libmergedlo.so";
            loLib = dlopen(mainLibrary);
            if (loLib == 0) {
                Log.i(TAG, String.format("Error: could not load fallback %s", mainLibrary));
                return;
            }
        }

        int lo_main = dlsym(loLib, "lo_main");
        if (lo_main == 0) {
            Log.i(TAG, String.format("No lo_main in %s", mainLibrary));
            return;
        }

        // Get extra libraries to load early, so that it's easier to debug
        // them even with a buggy ndk-gdb that doesn't grok debugging
        // information from libraries loaded after it has been attached to the
        // process.
        String extraLibs = getIntent().getStringExtra("lo-extra-libs");
        if (extraLibs != null) {
            for (String lib : extraLibs.split(":")) {
                dlopen(lib);
            }
        }

        // Start a strace on ourself if requested.

        // Note that the started strace will have its stdout and
        // stderr connected to /dev/null, so you definitely want to
        // specify an -o option in the lo-strace extra. Also, strace
        // will trace only *this* thread, which is not the one that
        // eventually will run android_main() and lo_main(), so you
        // also want the -f option.
        String strace_args = getIntent().getStringExtra("lo-strace");
        if (strace_args != null)
            system("/system/xbin/strace -p " + getpid() + " " + (strace_args != "yes" ? strace_args : "" ) + " &");

        int delay = 0;
        String sdelay = getIntent().getStringExtra("lo-main-delay");
        if (sdelay != null)
            delay = Integer.parseInt(sdelay);

        // Tell lo-bootstrap.c the stuff it needs to know
        if (!setup(lo_main, argv, delay))
            return;

        // Finally, call our super-class, NativeActivity's onCreate(),
        // which eventually calls the ANativeActivity_onCreate() in
        // android_native_app_glue.c, which starts a thread in which
        // android_main() from lo-bootstrap.c is called.

        // android_main() calls the lo_main() defined in sal/main.h
        // through the function pointer passed to setup() above, with
        // the argc and argv also saved from the setup() call.
        super.onCreate(savedInstanceState);
    }

    // This is used to load the 'lo-bootstrap' library on application
    // startup. The library has already been unpacked into
    // /data/data/<app name>/lib/liblo-bootstrap.so at installation
    // time by the package manager.
    static {
        System.loadLibrary("lo-bootstrap");
        System.loadLibrary("gnustl_shared");
        System.loadLibrary("libotouchlo");
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
