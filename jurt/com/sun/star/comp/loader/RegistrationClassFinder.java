/*************************************************************************
 *
 *  $RCSfile: RegistrationClassFinder.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 14:43:52 $
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

package com.sun.star.comp.loader;

import com.sun.star.lib.util.WeakMap;
import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.StringTokenizer;
import java.util.jar.JarInputStream;
import java.util.jar.Manifest;

final class RegistrationClassFinder {
    public static Class find(String locationUrl)
        throws ClassNotFoundException, IOException
    {
        synchronized (map) {
            Class c = (Class) WeakMap.getValue(map.get(locationUrl));
            if (c != null) {
                return c;
            }
        }
        URL url = new URL(locationUrl);
        checkAccess(url);
        String name = null;
        Manifest mf = new JarInputStream(url.openStream()).getManifest();
        if (mf != null) {
            name = mf.getMainAttributes().getValue("RegistrationClassName");
        }
        if (name == null) {
            return null;
        }
        Class c = new URLClassLoader(new URL[] { url }).loadClass(name);
        synchronized (map) {
            Class c2 = (Class) WeakMap.getValue(map.get(locationUrl));
            if (c2 != null) {
                return c2;
            }
            map.put(locationUrl, c);
        }
        return c;
    }

    private RegistrationClassFinder() {} // do not instantiate

    private static void checkAccess(URL url) throws ClassNotFoundException {
        // The system property com.sun.star.comp.loader.CPLD_ACCESSPATH was
        // introduced as a hack to restrict which UNO components can be
        // instantiated.  It seems to be unused nowadays, and should probably be
        // replaced by the native Java security features, anyway.
        if (accessPath != null) {
            if (!url.getProtocol().equals("file")) {
                throw new ClassNotFoundException(
                    "Access restriction: <" + url + "> is not a file URL");
            }
            String p;
            try {
                p = new File(url.getFile()).getCanonicalPath();
            } catch (IOException e) {
                throw new ClassNotFoundException(
                    "Access restriction: <" + url + "> is bad: " + e);
            }
            for (int i = 0; i < accessPath.length; ++i) {
                String p2 = accessPath[i];
                if (p.startsWith(p2) && p.length() > p2.length()
                    && (p2.charAt(p2.length() - 1) == File.separatorChar
                        || p.charAt(p2.length()) == File.separatorChar))
                {
                    return;
                }
            }
            throw new ClassNotFoundException(
                "Access restriction: <" + url + "> is restricted");
        }
    }

    private static final WeakMap map = new WeakMap();

    private static final String[] accessPath;
    static {
        String[] ap = null;
        String p = System.getProperty(
            "com.sun.star.comp.loader.CPLD_ACCESSPATH");
        if (p != null) {
            StringTokenizer t = new StringTokenizer(p, ";");
            ap = new String[t.countTokens()];
            int i = 0;
            while (t.hasMoreTokens()) {
                try {
                    ap[i] = new File(t.nextToken()).getCanonicalPath();
                    ++i;
                } catch (IOException e) {}
            }
            if (i != ap.length) {
                String[] ap2 = new String[i];
                System.arraycopy(ap, 0, ap2, 0, i);
                ap = ap2;
            }
        }
        accessPath = ap;
    }
}
