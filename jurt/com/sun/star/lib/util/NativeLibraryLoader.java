/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: NativeLibraryLoader.java,v $
 * $Revision: 1.10 $
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
package com.sun.star.lib.util;

import java.io.File;
import java.net.URL;
import java.net.URLClassLoader;

/** Helper functions to locate and load native files.

    The methods in this class are designed to find the requested resources in as
    many cases as possible.  They search various places, roughly from most
    specific to most general.  This works well if a component is known to bring
    with it a certain resource, and that resource has to be found.  However, it
    might not work very well in cases where you want to check whether a
    component brings with it a certain resource or not: a similarly named
    resource from another component might be found by the eager search
    algorithm.
 */
public final class NativeLibraryLoader {
    /** Load a system library, using a given class loader to locate the library.

        This is similar to System.loadLibrary.

        @param loader a class loader; may be null

        @param libname the library name; how this name is mapped to a system
        library name is system dependent
     */
    public static void loadLibrary(ClassLoader loader, String libname) {
        File path = getResource(loader, System.mapLibraryName(libname));
        if (path == null) {
            // If the library cannot be found as a class loader resource, try
            // the global System.loadLibrary as a last resort:
            System.loadLibrary(libname);
        } else {
            System.load(path.getAbsolutePath());
        }
    }

    /** Locate a system resource, using a given class loader.

        This is similar to ClassLoader.getResource, but only works for local
        resources (local files), and adds additional functionality for
        URLClassLoaders.

        @param loader a class loader; may be null

        @param name a resource name (that is, the name of a file)

        @return a File locating the resource, or null if the resource was not
        found
     */
    public static File getResource(ClassLoader loader, String name) {
        if (loader != null) {
            File path = UrlToFileMapper.mapUrlToFile(loader.getResource(name));
            if (path != null) {
                return path;
            }
        }
        // URLClassLoaders work on lists of URLs, which are typically URLs
        // locating JAR files (scheme://auth/dir1/dir2/some.jar).  The following
        // code looks for resource name beside the JAR file
        // (scheme://auth/dir1/dir2/name) and one directory up
        // (scheme://auth/dir1/name).  The second step is important in a typical
        // OOo installation, where the JAR files are in the program/classes
        // directory while the shared libraries are in the program directory.
        if (loader instanceof URLClassLoader) {
            URL[] urls = ((URLClassLoader) loader).getURLs();
            for (int i = 0; i < urls.length; ++i) {
                File path = UrlToFileMapper.mapUrlToFile(urls[i]);
                if (path != null) {
                    File dir = path.isDirectory() ? path : path.getParentFile();
                    if (dir != null) {
                        path = new File(dir, name);
                        if (path.exists()) {
                            return path;
                        }
                        dir = dir.getParentFile();
                        if (dir != null) {
                            path = new File(dir, name);
                            if (path.exists()) {
                                return path;
                            }
                        }
                    }
                }
            }
        }
        return null;
    }

    private NativeLibraryLoader() {} // do not instantiate
}
