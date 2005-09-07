/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RegistrationClassFinder.java,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:51:52 $
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

package com.sun.star.comp.loader;

import com.sun.star.lib.unoloader.UnoClassLoader;
import com.sun.star.lib.util.WeakMap;
import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.StringTokenizer;
import java.util.jar.Attributes;

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
        Attributes attr = UnoClassLoader.getJarMainAttributes(url);
        String name = attr == null
            ? null : attr.getValue("RegistrationClassName");
        if (name == null) {
            return null;
        }
        ClassLoader cl1 = RegistrationClassFinder.class.getClassLoader();
        ClassLoader cl2;
        if (cl1 instanceof UnoClassLoader) {
            cl2 = ((UnoClassLoader) cl1).getClassLoader(url, attr);
        } else {
            cl2 = URLClassLoader.newInstance(new URL[] { url }, cl1);
        }
        Class c = cl2.loadClass(name);
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
