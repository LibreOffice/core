/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
     * (<code>jurt.jar</code>, <code>ridl.jar</code>, etc.) can be found; must
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
                new PrivilegedExceptionAction() {
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
