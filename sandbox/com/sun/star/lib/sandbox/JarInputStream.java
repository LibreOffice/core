/*************************************************************************
 *
 *  $RCSfile: JarInputStream.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:24:28 $
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

package com.sun.star.lib.sandbox;

import java.io.InputStream;
import java.io.IOException;

import java.util.Hashtable;

import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import sun.tools.jar.JarVerifierStream;
import sun.tools.jar.JarException;

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

