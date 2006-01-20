/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UnoInfo.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-01-20 10:12:10 $
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
