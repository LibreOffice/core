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
package com.sun.star.lib.util;

import java.io.File;
import java.net.URL;
import java.net.URLClassLoader;

/**
 * Helper functions to locate and load native files.
 *
 * <p>The methods in this class are designed to find the requested resources in
 * as many cases as possible.  They search various places, roughly from most
 * specific to most general.  This works well if a component is known to bring
 * with it a certain resource, and that resource has to be found.  However, it
 * might not work very well in cases where you want to check whether a
 * component brings with it a certain resource or not: a similarly named
 * resource from another component might be found by the eager search
 * algorithm.</p>
 */
public final class NativeLibraryLoader {
    /**
     * Load a system library, using a given class loader to locate the library.
     *
     * <p>This is similar to <code>System.loadLibrary</code>.</p>
     *
     * @param loader a class loader; may be null.
     * @param libname the library name; how this name is mapped to a system
     * library name is system dependent.
     */
    public static void loadLibrary(ClassLoader loader, String libname) {
        String sysname = System.mapLibraryName(libname);
        // At least Oracle's 1.7.0_51 now maps to .dylib rather than .jnilib:
        if (System.getProperty("os.name").startsWith("Mac")
            && sysname.endsWith(".dylib"))
        {
            sysname
                = sysname.substring(0, sysname.length() - "dylib".length())
                + "jnilib";
        }
        File path = getResource(loader, sysname);
        if (path == null) {
            // If the library cannot be found as a class loader resource, try
            // the global System.loadLibrary as a last resort:
            System.loadLibrary(libname);
        } else {
            System.load(path.getAbsolutePath());
        }
    }

    /**
     * Locate a system resource, using a given class loader.
     *
     * <p>This is similar to <code>ClassLoader.getResource</code>, but only works
     * for local resources (local files), and adds additional functionality for
     * <code>URLClassLoaders</code>.</p>
     *
     * @param loader a class loader; may be null.
     * @param name a resource name (that is, the name of a file).
     * @return a File locating the resource, or null if the resource was not
     * found.
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
        if (!(loader instanceof URLClassLoader)) {
            return null;
        }
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
                        // On OS X, dir is now the Resources dir,
                        // we want to look in Frameworks
                        if (System.getProperty("os.name").startsWith("Mac")
                            && dir.getName().equals("Resources")) {
                            dir = dir.getParentFile();
                            path = new File(dir, "Frameworks/" + name);
                            if (path.exists()) {
                                return path;
                            }
                            // In case of ENABLE_MACOSX_MACLIKE_APP_STRUCTURE,
                            // dir is now the Resources dir, we want to look in Frameworks
                            if (System.getProperty("os.name").startsWith("Mac")
                                && dir.getName().equals("Resources")) {
                                dir = dir.getParentFile();
                                path = new File(dir, "Frameworks/" + name);
                                if (path.exists()) {
                                    return path;
                                }
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
