/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: JarInputStream.java,v $
 * $Revision: 1.4 $
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

import java.io.InputStream;
import java.io.IOException;

import java.util.Hashtable;

import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;


class JarInputStream {
//    JarVerifierStream jvs = null;
    ZipInputStream jvs = null;

    ZipEntry ent = null;

    // mapping of names to objects that have been loaded from the jar
    Hashtable nameToObject = new Hashtable();

    public JarInputStream(InputStream inputStream) throws IOException {
//      try {
            jvs = new ZipInputStream(inputStream);
//      jvs = new JarVerifierStream(inputStream);
//      }
//      catch(JarException e) {
//          throw new IOException("JarInputStream");
//      }
    }


    public int available() throws IOException {
        return jvs.available();
    }


    public JarEntry getNextJarEntry() throws IOException {
        ent = jvs.getNextEntry();

//          if(ent != null && isManifest(ent.getName()))
//              ent = jvs.getNextEntry();

        return (ent != null) ? new JarEntry(ent, null/*jvs*/) : null;
    }

    public int read(byte bytes[]) throws IOException {
        return jvs.read(bytes,0, bytes.length);
    }

    public void close() throws IOException {
        jvs.close();
    }


    static boolean isManifest(String name) {
        boolean isManifest = false;

        name = name.toUpperCase();

        /* ignore leading slash */
        if (name.startsWith("/"))
            name = name.substring(1);

        /* a manifest file or signature file at all? */
        if (name.startsWith("META-INF/")) {
            /* the manifest? */
            isManifest = name.equalsIgnoreCase("META-INF/MANIFEST.MF");
            isManifest = isManifest || name.endsWith("DSA");
            isManifest = isManifest || name.endsWith("PK7");
            isManifest = isManifest || name.endsWith("PGP");
            isManifest = isManifest || name.endsWith("SF");
        }

        return isManifest;
    }
}

