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

        String fileUrl = dir.toURI().toURL().toString();
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

        String fileUrl = dir.toURI().toURL().toString();
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
