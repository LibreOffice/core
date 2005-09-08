/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UnoLoader.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 13:26:10 $
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

package com.sun.star.lib.unoloader;

import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.net.URL;

/**
 * A helper class for executing UNO JARs.
 *
 * <p><em>This class is not yet stable.</em></p>
 *
 * @since UDK 3.2.1
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
    public static void execute(URL base, URL jar, String[] arguments)
        throws IOException, ClassNotFoundException, NoSuchMethodException,
        InvocationTargetException
    {
        new UnoClassLoader(base, UnoLoader.class.getClassLoader()).execute(
            jar, arguments);
    }

    private UnoLoader() {}
}
