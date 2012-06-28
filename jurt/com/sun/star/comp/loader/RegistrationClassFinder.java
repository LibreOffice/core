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
    public static Class<?> find(String locationUrl)
        throws ClassNotFoundException, IOException
    {
        synchronized (map) {
            Class<?> c = (Class<?>) WeakMap.getValue(map.get(locationUrl));
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
        Class<?> c = cl2.loadClass(name);
        synchronized (map) {
            Class<?> c2 = (Class<?>) WeakMap.getValue(map.get(locationUrl));
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
