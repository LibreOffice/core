/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
                URL url = ((URLClassLoader) cl).findResource(
                    System.mapLibraryName(libname));
                if (url != null) {
                    try {
                        System.load(
                            ((File) File.class.getConstructor(
                                new Class[] {
                                    ClassLoader.getSystemClassLoader().
                                    loadClass("java.net.URI") }).
                             newInstance(
                                 new Object[] {
                                     URL.class.getMethod("toURI", new Class[0]).
                                     invoke(url, (java.lang.Object[])null) })).
                            getAbsolutePath());
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
