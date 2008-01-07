/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NativeLibraries.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-07 09:45:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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
            loadLibrary("dbtools680mi");
        }
        loadLibrary("hsqldb2");
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
                                     invoke(url, null) })).
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
