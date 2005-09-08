/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: JarEntry.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:06:10 $
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

