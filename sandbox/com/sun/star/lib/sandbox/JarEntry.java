/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: JarEntry.java,v $
 * $Revision: 1.3 $
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

package com.sun.star.lib.sandbox;

import java.security.Identity;

import java.util.zip.ZipEntry;

import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

import sun.tools.jar.JarVerifierStream;

public class JarEntry {
    private static final boolean DEBUG = false;

    private ZipEntry zipEntry = null;
    private JarVerifierStream jvs = null;

    public JarEntry(ZipEntry zipEntry, JarVerifierStream jvs) {
        this.zipEntry = zipEntry;
        this.jvs = jvs;
    }

    public Identity[] getIdentities() {
        Identity identities[] = null;

        if(jvs != null) {
            Class classes[] = new Class[1];
            classes[0] = String.class;

            try {
                Method method = JarVerifierStream.class.getMethod("getIdentities", classes);

                Object objects[] = new Object[1];
                objects[0] = zipEntry.getName();

                identities = (Identity[])method.invoke(jvs, objects);
            }
            catch(IllegalAccessException illegalAccessException) {
                if(DEBUG)System.err.println("#### JarEntry.getIdentities:" + illegalAccessException);
            }
            catch(IllegalArgumentException illegalArgumentException) {
                if(DEBUG)System.err.println("#### JarEntry.getIdentities:" + illegalArgumentException);
            }
            catch(InvocationTargetException invocationTargetException ) {
                if(DEBUG)System.err.println("#### JarEntry.getIdentities:" + invocationTargetException);
            }
            catch(NoSuchMethodException noSuchMethodException) {
                if(DEBUG)System.err.println("#### JarEntry.getIdentities:" + noSuchMethodException);
            }
            catch(SecurityException securityException) {
                if(DEBUG)System.err.println("#### JarEntry.getIdentities:" + securityException);
            }
        }
          return identities;
    }

    public String getName() {
        return zipEntry.getName();
    }

    public long getSize() {
        return zipEntry.getSize();
    }
}

