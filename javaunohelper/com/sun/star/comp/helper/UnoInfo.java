/*************************************************************************
 *
 *  $RCSfile: UnoInfo.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 15:15:51 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
     * Gets the UNO jar files.
     *
     * @return the UNO jar files
     */
    public static URL[] getJars() {

        final String JUHJAR = "juh.jar";

        String[] jarFileNames = new String[] {
            "jurt.jar",
            "unoil.jar",
            "ridl.jar",
            "juh.jar",
            "unoinfo.jar" };

        URL[] jars = new URL[jarFileNames.length];
        URLClassLoader cl = (URLClassLoader) UnoInfo.class.getClassLoader();
        URL[] urls = cl.getURLs();
        for ( int i = 0; i < urls.length; i++ ) {
            String url = urls[i].toString();
            int index = url.indexOf( JUHJAR );
            if ( index >= 0 ) {
                String base = url.substring( 0, index );
                for ( int j = 0; j < jarFileNames.length; j++ ) {
                    try {
                        jars[j] = new URL( base + jarFileNames[j] );
                    } catch ( MalformedURLException e ) {
                        return null;
                    }
                }
                break;
            }
        }

        return jars;
    }
}
