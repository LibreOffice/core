/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NativeLibraryLoader_Test.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:16:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package com.sun.star.lib.util;

import complexlib.ComplexTestCase;
import java.io.File;
import java.net.MalformedURLException;
import java.net.URL;

public final class NativeLibraryLoader_Test extends ComplexTestCase {
    public String[] getTestMethodNames() {
        return new String[] { "testEncoded", "testUnencoded" };
    }

    public void testEncoded() throws MalformedURLException {
        File dir = new File(System.getProperty("user.dir"));
        File subdir = new File(dir, "with space");
        File file1 = new File(subdir, "file");

        String fileUrl = dir.toURL().toString();
        if (!fileUrl.endsWith("/")) {
            fileUrl += "/";
        }
        fileUrl += "with%20space/file";
        final URL url = new URL(fileUrl);

        File file2 = NativeLibraryLoader.getResource(
            new ClassLoader() {
                public URL getResource(String name) {
                    return url;
                }
            },
            "dummy");
        assure("Files are equal", file2.equals(file1));
    }

    public void testUnencoded() throws MalformedURLException {
        File dir = new File(System.getProperty("user.dir"));
        File subdir = new File(dir, "with space");
        File file1 = new File(subdir, "file");

        String fileUrl = dir.toURL().toString();
        if (!fileUrl.endsWith("/")) {
            fileUrl += "/";
        }
        fileUrl += "with space/file";
        final URL url = new URL(fileUrl);

        File file2 = NativeLibraryLoader.getResource(
            new ClassLoader() {
                public URL getResource(String name) {
                    return url;
                }
            },
            "dummy");
        assure("Files are equal", file2.equals(file1));
    }
}
