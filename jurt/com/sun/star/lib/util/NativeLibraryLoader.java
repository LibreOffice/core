/*************************************************************************
 *
 *  $RCSfile: NativeLibraryLoader.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 14:34:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package com.sun.star.lib.util;

import java.io.File;
import java.net.URL;
import java.net.URLClassLoader;

/** Helper functions to locate and load native files.
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
            throw new UnsatisfiedLinkError(
                "Don't know how to load native library " + libname);
        }
        System.load(path.getAbsolutePath());
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
            File path = mapUrlToFile(loader.getResource(name));
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
                File path = mapUrlToFile(urls[i]);
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

    private static File mapUrlToFile(URL url) {
        if (url != null && url.getProtocol().equalsIgnoreCase("file")) {
            String f = url.getFile();
            return new File(f == null ? "" : f);
        }
        return null;
    }
}
