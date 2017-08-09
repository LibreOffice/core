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
