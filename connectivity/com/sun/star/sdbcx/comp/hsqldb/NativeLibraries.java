/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package com.sun.star.sdbcx.comp.hsqldb;

import java.io.File;
import java.net.URL;
import java.net.URLClassLoader;

final class NativeLibraries {
    public static void load() {
        if (System.getProperty( "os.name" ).startsWith("Windows")) {
            loadLibrary("msvcr71");
            loadLibrary("uwinapi");
            loadLibrary("sal3");
            loadLibrary("dbtoolsmi");
        }
        loadLibrary("hsqldb");
    }

    private static void loadLibrary(String libname) {
        // At least on Mac OS X Tiger, System.loadLibrary("hsqldb2") does not
        // find the hsqldb2 library one directory above sdbc_hsqldb.jar, even
        // though ".." is on the jar's Class-Path; however, the alternative
        // code (needing Java 1.5, which is given for Mac OS X Tiger) works
        // there:
        try {
            System.loadLibrary(libname);
        } catch (UnsatisfiedLinkError e) {
            ClassLoader cl = NativeLibraries.class.getClassLoader();
            if (cl instanceof URLClassLoader) {
                String sysname = System.mapLibraryName(libname);
                // At least Oracle's 1.7.0_51 now maps to .dylib rather than
                // .jnilib:
                if (System.getProperty("os.name").startsWith("Mac")
                    && sysname.endsWith(".dylib"))
                {
                    sysname
                        = sysname.substring(
                            0, sysname.length() - "dylib".length())
                        + "jnilib";
                }
                URL url = ((URLClassLoader) cl).findResource(sysname);
                if (url != null) {
                    try {
                        System.load(new File(url.toURI()).getAbsolutePath());
                    } catch (Throwable t) {
                        throw new UnsatisfiedLinkError(
                            e.toString()+ " - " + t.toString());
                    }
                }
            }
        }
    }

    private NativeLibraries() {}
}
