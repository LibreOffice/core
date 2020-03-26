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
import java.security.AccessController;
import java.security.PrivilegedActionException;
import java.security.PrivilegedExceptionAction;

/**
 * A helper class for executing UNO JARs.
 *
 * <p><em>This class is not yet stable.</em></p>
 *
 * @since UDK 3.2.0
 */
public final class UnoLoader {
    /**
     * Executes a UNO JAR.
     *
     * @param base a base URL relative to which the URE JARs
     * (<code>libreoffice.jar</code>, etc.) can be found; must
     * not be <code>null</code>.
     *
     * @param jar the URL of a UNO JAR that specifies a Main-Class; must not be
     * <code>null</code>.
     *
     * @param arguments any arguments passed to the <code>main</code> method of
     * the specified Main-Class of the given JAR <code>jar</code>; must not be
     * <code>null</code>.
     *
     * @throws IOException if the given <code>base</code> URL is malformed, or
     * if there are any problems processing the given JAR <code>jar</code>.
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
    public static void execute(final URL base, URL jar, String[] arguments)
        throws IOException, ClassNotFoundException, NoSuchMethodException,
        InvocationTargetException
    {
        UnoClassLoader cl;
        try {
            cl = (UnoClassLoader) AccessController.doPrivileged(
                new PrivilegedExceptionAction<Object>() {
                    public Object run() throws MalformedURLException {
                        return new UnoClassLoader(
                            base, null, UnoLoader.class.getClassLoader());
                    }
                });
        } catch (PrivilegedActionException e) {
            throw (MalformedURLException) e.getException();
        }
        cl.execute(jar, arguments);
    }

    private UnoLoader() {}
}
