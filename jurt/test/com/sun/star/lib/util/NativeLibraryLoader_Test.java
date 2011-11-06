/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
