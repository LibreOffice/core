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
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;

import java.io.File;
import java.util.HashMap;

public class Bootstrap extends Activity
{
    private static String TAG = "lo-bootstrap";
    private String dataDir;

    // A native method to list the DT_NEEDED names in a ELF shared object
    public static native String[] dlneeds(String library);

    // A native method to call dlopen(library, RTLD_LOCAL)
    public static native int dlopen(String library);

    // A native method to call dlsym(handle, symbol)
    public static native int dlsym(int handle, String symbol);

    // A native method to call (*function)(argument)
    public static native int dlcall(int function, Object argument);

    // Already loaded libraries
    private HashMap<String, Integer> presentLibs = new HashMap<String, Integer>();

    private int loadLibrary(String library)
    {
        // We should *not* try to just dlopen() the bare library name
        // first, as the stupid dynamic linker remembers for each
        // library basename if loading it has failed. Thus if you try
        // loading it once, and it fails because of missing needed
        // libraries, and your load those, and then try again, it
        // fails with an infurtating message "failed to load
        // previously" in the log.

        // We *must* first dlopen() all needed libraries,
        // recursively. It shouldn't matter if we dlopen() a library
        // that already is loaded, dlopen() just returns the same
        // value then.

        Integer handle;

        if ((handle = presentLibs.get(library)) != null)
            return handle;

        String fullName = null;
        boolean found = false;
        String[] libraryLocations = { "/system/lib/", dataDir + "/lib/" };
        for (String dir : libraryLocations ) {
            fullName = dir + library;
            if (new File(fullName).exists()) {
                found = true;
                break;
            }
        }
        if (!found) {
            Log.i(TAG, String.format("Library not found: %s\n", library));
            return 0;
        }

        String[] needs = dlneeds(fullName);
        if (needs == null)
            return 0;

        for (String neededLibrary : needs) {
            if (loadLibrary(neededLibrary) == 0)
                return 0;
        }
        if ((handle = dlopen(fullName)) == 0)
            return 0;

        presentLibs.put(library, handle);
        return handle;
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        try {
            ApplicationInfo ai = this.getPackageManager().getApplicationInfo
                ("org.libreoffice.android",
                 PackageManager.GET_META_DATA);
            Log.i(TAG, String.format("sourceDir=%s\n", ai.sourceDir));
            dataDir = ai.dataDir;
            Log.i(TAG, String.format("dataDir=%s\n", dataDir));
        }
        catch (PackageManager.NameNotFoundException e) {
            return;
        }

        String mainLibrary = getIntent().getStringExtra("lo-main-library");

        if (mainLibrary != null) {
            int loLib = loadLibrary(mainLibrary + ".so");

            if (loLib == 0)
                return;

            // Get "command line" to pass to the LO "program"
            String cmdLine = getIntent().getStringExtra("lo-main-cmdline");
            String[] argv;
            if (cmdLine != null)
                argv = cmdLine.split(" *");
            else
                argv = new String[0];
            int loLibMain = dlsym(loLib, "lo_main");
            if (loLibMain != 0)
                dlcall(loLibMain, argv);
        }
    }

    /* This is used to load the 'lo-bootstrap' library on application
     * startup. The library has already been unpacked into
     * /data/data/<app name>/lib/liblo-bootstrap.so at
     * installation time by the package manager.
     */
    static {
        System.loadLibrary("lo-bootstrap");
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
