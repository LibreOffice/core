/*************************************************************************
 *
 *  $RCSfile: NativeLibraryLoader.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-01-31 15:51:51 $
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
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.net.URL;
import java.net.URLClassLoader;
import java.net.URLDecoder;
import java.net.URLEncoder;

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

    // java.net.URLEncoder.encode(String, String) and java.net.URI are only
    // available since Java 1.4:
    private static Method urlEncoderEncode;
    private static Constructor uriConstructor;
    private static Constructor fileConstructor;
    static {
        try {
            urlEncoderEncode = URLEncoder.class.getMethod(
                "encode", new Class[] { String.class, String.class });
            Class uriClass = Class.forName("java.net.URI");
            uriConstructor = uriClass.getConstructor(
                new Class[] { String.class });
            fileConstructor = File.class.getConstructor(
                new Class[] { uriClass });
        } catch (ClassNotFoundException e) {
        } catch (NoSuchMethodException e) {
        }
    }

    private static File mapUrlToFile(URL url) {
        if (url == null) {
            return null;
        } else if (fileConstructor == null) {
            // If java.net.URI is not available, hope that the following works
            // well:  First, check that the given URL has a certain form.
            // Second, use the URLDecoder to decode the URL path (taking care
            // not to change any plus signs to spaces), hoping that the used
            // default encoding is the proper one for file URLs.  Third, create
            // a File from the decoded path.
            return url.getProtocol().equalsIgnoreCase("file")
                && url.getAuthority() == null && url.getQuery() == null
                && url.getRef() == null
                ? new File(URLDecoder.decode(
                               StringHelper.replace(url.getPath(), '+', "%2B")))
                : null;
        } else {
            // If java.net.URI is avaliable, do
            //   URI uri = new URI(encodedUrl);
            //   try {
            //       return new File(uri);
            //   } catch (IllegalArgumentException e) {
            //       return null;
            //   }
            // where encodedUrl is url.toString(), but since that may contain
            // unsafe characters (e.g., space, " "), it is encoded, as otherwise
            // the URI constructor might throw java.net.URISyntaxException (in
            // Java 1.5, URL.toURI might be used instead).
            String encodedUrl = encode(url.toString());
            try {
                Object uri = uriConstructor.newInstance(
                    new Object[] { encodedUrl });
                try {
                    return (File) fileConstructor.newInstance(
                        new Object[] { uri });
                } catch (InvocationTargetException e) {
                    if (e.getTargetException() instanceof
                        IllegalArgumentException) {
                        return null;
                    } else {
                        throw e;
                    }
                }
            } catch (InstantiationException e) {
                throw new RuntimeException("This cannot happen: " + e);
            } catch (IllegalAccessException e) {
                throw new RuntimeException("This cannot happen: " + e);
            } catch (InvocationTargetException e) {
                if (e.getTargetException() instanceof Error) {
                    throw (Error) e.getTargetException();
                } else if (e.getTargetException() instanceof RuntimeException) {
                    throw (RuntimeException) e.getTargetException();
                } else {
                    throw new RuntimeException("This cannot happen: " + e);
                }
            }
        }
    }

    private static String encode(String url) {
        StringBuffer buf = new StringBuffer();
        for (int i = 0; i < url.length(); ++i) {
            char c = url.charAt(i);
            // The RFC 2732 <uric> characters: !$&'()*+,-./:;=?@[]_~ plus digits
            // and letters; additionally, do not encode % again.
            if (c >= 'a' && c <= 'z' || c >= '?' && c <= '['
                || c >= '$' && c <= ';' || c == '!' || c == '=' || c == ']'
                || c == '_' || c == '~')
            {
                buf.append(c);
            } else if (c == ' ') {
                buf.append("%20");
            } else {
                String enc;
                try {
                    enc = (String) urlEncoderEncode.invoke(
                        null, new Object[] { Character.toString(c), "UTF-8" });
                } catch (IllegalAccessException e) {
                    throw new RuntimeException("This cannot happen: " + e);
                } catch (InvocationTargetException e) {
                    throw new RuntimeException("This cannot happen: " + e);
                }
                buf.append(enc);
            }
        }
        return buf.toString();
    }
}
