/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: UnoInfo.java,v $
 * $Revision: 1.7 $
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

package com.sun.star.comp.helper;

import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;

/**
 * UnoInfo offers functionality to obtain the UNO jar files.
 */
public final class UnoInfo {

    /**
     * do not instantiate
     */
    private UnoInfo() {}

    /**
     * Gets the URL base.
     *
     * @return the URL base
     */
    private static String getBase() {

        final String JUHJAR = "/juh.jar";

        String base = null;

        URLClassLoader cl = (URLClassLoader) UnoInfo.class.getClassLoader();
        URL[] urls = cl.getURLs();
        for ( int i = 0; i < urls.length; i++ ) {
            String url = urls[i].toString();
            if ( url.endsWith( JUHJAR ) )
            {
                int index = url.lastIndexOf( JUHJAR );
                if ( index >= 0 ) {
                    base = url.substring( 0, index + 1 );
                    break;
                }
            }
        }

        return base;
    }

    /**
     * Gets a list of URLs for the given jar files.
     *
     * @return the list of URLs
     */
    private static URL[] getURLs( String[] jarFileNames ) {

        URL[] jars = new URL[jarFileNames.length];
        String base = getBase();
        for ( int i = 0; i < jarFileNames.length; i++ ) {
            try {
                jars[i] = new URL( base + jarFileNames[i] );
            } catch ( MalformedURLException e ) {
                return null;
            }
        }

        return jars;
    }

    /**
     * Gets the UNO jar files.
     *
     * @return the UNO jar files
     */
    public static URL[] getJars() {

        String[] jarFileNames = new String[] {
            "jurt.jar",
            "unoil.jar",
            "ridl.jar",
            "juh.jar" };

        return getURLs( jarFileNames );
    }

    /**
     * Gets the extra UNO types.
     *
     * @return the extra UNO types
     */
    public static URL[] getExtraTypes() {

        String[] jarFileNames = new String[] {
            "unoil.jar" };

        return getURLs( jarFileNames );
    }
}
