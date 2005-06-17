/*************************************************************************
 *
 *  $RCSfile: UnoClassLoader.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-06-17 09:44:36 $
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
 *  Copyright: 2005 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
 * @since UDK 3.2.1
 */
public final class UnoClassLoader extends URLClassLoader {
    /**
     * Instantiates the root UNO class loader.
     *
     * @param base a base URL relative to which the URE JARs
     * (<code>java_uno.jar</code>, <code>juh.jar</code>, <code>jurt.jar</code>,
     * <code>ridl.jar</code>) can be found; must not be <code>null</code>.
     *
     * @param parent the parent class loader for delegation.
     *
     * @throws MalformedURLException if the given <code>base</code> URL is
     * malformed.
     */
    public UnoClassLoader(URL base, ClassLoader parent)
        throws MalformedURLException
    {
        //TODO  Get rid of java_uno.jar here:
        super(
            new URL[] {
                new URL(base, "java_uno.jar"), new URL(base, "juh.jar"),
                new URL(base, "jurt.jar"), new URL(base, "ridl.jar") },
            parent);
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
}
