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

package com.sun.star.lib.unoloader;

import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.jar.Attributes;
import java.util.jar.Manifest;
import java.util.jar.JarInputStream;

/**
 * The root UNO class loader.
 *
 * <p>This class loader is able to load all published URE classes, including the
 * classes representing the published URE UNO types.  For consistency, it is
 * important that within each Java UNO environment there is one instance of this
 * class loader that is the defining class loader of all published URE classes
 * (and hence of all unpublished URE classes, too) and of all classes
 * representing UNO types (the published URE UNO types, any unpublished URE UNO
 * types, and any additional UNO types introduced by components; for the latter,
 * {@link #addURL} may be necessary).</p>
 *
 * <p><em>This is an internal, unstable class of the Uno Runtime Environment; it
 * should not be used by client code.</em></p>
 *
 * @since UDK 3.2.0
 */
public final class UnoClassLoader extends URLClassLoader {
    /**
     * Instantiates the root UNO class loader.
     *
     * @param base a base URL relative to which the URE JARs
     * (<code>java_uno.jar</code> and <code>libreoffice.jar</code>)
     * can be found; must not be <code>null</code>.
     *
     * @param classPath an array of URLs that form the class path of this class
     * loader; may be <code>null</code>, which is the same as an empty array.
     * The URLs are interpreted in the same way as the arguments of a {@link
     * URLClassLoader}.
     *
     * @param parent the parent class loader for delegation.
     *
     * @throws MalformedURLException if the given <code>base</code> URL is
     * malformed.
     */
    public UnoClassLoader(URL base, URL[] classPath, ClassLoader parent)
        throws MalformedURLException
    {
        super(createUrls(base, classPath), parent);
    }

    /**
     * Obtains a class loader for a UNO JAR.
     *
     * @param jar the URL of a UNO JAR; must not be <code>null</code>.
     *
     * @param mainAttributes represents the main section of the manifest of the
     * given JAR <code>jar</code>; <code>null</code> if the given JAR does not
     * have a manifest.  (This redundant parameter is there for performance
     * reasons, as typically the caller of this method already has this
     * information available.)
     *
     * @return an appropriate class loader; will never be <code>null</code>.
     *
     * @throws MalformedURLException if the given <code>jar</code> URL or any of
     * the UNO-Type-Path URLs specified in the given JAR are malformed.
     */
    public ClassLoader getClassLoader(URL jar, Attributes mainAttributes)
        throws MalformedURLException
    {
        String path = mainAttributes == null ?
            null : mainAttributes.getValue("UNO-Type-Path");
        if (path == null) {
            path = "<>";
        }
        for (int i = 0; i < path.length();) {
            while (i < path.length() && path.charAt(i) == ' ') {
                ++i;
            }
            if (i < path.length()) {
                String url;
                if (path.charAt(i) == '<') {
                    int j = path.indexOf('>', i + 1);
                    if (j < 0) {
                        url = path.substring(i + 1);
                        i = path.length();
                    } else {
                        url = path.substring(i + 1, j);
                        i = j + 1;
                    }
                } else {
                    int j = path.indexOf(' ', i + 1);
                    if (j < 0) {
                        j = path.length();
                    }
                    url = path.substring(i, j);
                    i = j;
                }
                addURL(new URL(jar, url));
            }
        }
        return URLClassLoader.newInstance(new URL[] { jar }, this);
    }

    /**
     * Executes a UNO JAR.
     *
     * @param jar the URL of a UNO JAR that specifies a Main-Class; must not be
     * <code>null</code>.
     *
     * @param arguments any arguments passed to the <code>main</code> method of
     * the specified Main-Class of the given JAR <code>jar</code>; must not be
     * <code>null</code>.
     *
     * @throws IOException if there are any problems processing the given JAR
     * <code>jar</code>.
     *
     * @throws ClassNotFoundException if the given JAR <code>jar</code> does not
     * specify a Main-Class, or if the specified Main-Class cannot be found.
     *
     * @throws NoSuchMethodException if the specified Main-Class of the given
     * JAR <code>jar</code> does not have an appropriate <code>main</code>
     * method.
     *
     * @throws InvocationTargetException if an exception occurs while executing
     * the <code>main</code> method of the specified Main-Class of the given JAR
     * <code>jar</code>.
     */
    public void execute(URL jar, String[] arguments)
        throws IOException, ClassNotFoundException, NoSuchMethodException,
        InvocationTargetException
    {
        Attributes attr = getJarMainAttributes(jar);
        String name = attr == null
            ? null : attr.getValue(Attributes.Name.MAIN_CLASS);
        if (name == null) {
            throw new ClassNotFoundException(
                jar + " does not specify a main class");
        }
        try {
            getClassLoader(jar, attr).loadClass(name.replace('/', '.')).
                getMethod("main", new Class[] { String[].class }).
                invoke(null, new Object[] { arguments });
        } catch (IllegalAccessException e) {
            throw new RuntimeException("impossible " + e);
        }
    }

    /**
     * Obtains the main section of the manifest of a JAR.
     *
     * @param jar the URL of a JAR; must not be <code>null</code>.
     *
     * @return the representation of the main section of the manifest of the
     * given JAR <code>jar</code>, or <code>null</code> if the given JAR does
     * not have a manifest.
     *
     * @throws IOException if there are any problems processing the given JAR
     * <code>jar</code>.
     */
    public static Attributes getJarMainAttributes(URL jar) throws IOException {
        JarInputStream s = new JarInputStream(jar.openStream());
        Manifest mf;
        try {
            mf = s.getManifest();
        } finally {
            s.close();
        }
        return mf == null ? null : mf.getMainAttributes();
    }

    private static URL[] createUrls(URL base, URL[] classPath)
        throws MalformedURLException
    {
        final int JARS = 2;
        URL[] urls = new URL[JARS + (classPath == null ? 0 : classPath.length)];
        urls[0] = new URL(base, "java_uno.jar"); //TODO get rid of it here
        urls[1] = new URL(base, "libreoffice.jar");
        if (classPath != null) {
            System.arraycopy(classPath, 0, urls, JARS, classPath.length);
        }
        return urls;
    }
}
