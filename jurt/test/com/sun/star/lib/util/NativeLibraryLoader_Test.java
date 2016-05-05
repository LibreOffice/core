/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
import java.net.MalformedURLException;
import java.net.URL;
import org.junit.Test;
import static org.junit.Assert.*;

public final class NativeLibraryLoader_Test {
    @Test public void testEncoded() throws MalformedURLException {
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
                @Override
                public URL getResource(String name) {
                    return url;
                }
            },
            "dummy");
        assertEquals("Files are equal", file1, file2);
    }

    @Test public void testUnencoded() throws MalformedURLException {
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
                @Override
                public URL getResource(String name) {
                    return url;
                }
            },
            "dummy");
        assertEquals("Files are equal", file1, file2);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
