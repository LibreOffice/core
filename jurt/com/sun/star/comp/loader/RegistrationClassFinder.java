/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
import java.io.IOException;
import java.net.URL;
import java.net.URLClassLoader;
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

    private static final WeakMap map = new WeakMap();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
