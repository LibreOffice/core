/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
            "ridl.jar",
            "juh.jar",
        "unoil.jar" };

        return getURLs( jarFileNames );
    }

    /**
     * Gets the extra UNO types.
     *
     * @return the extra UNO types
     */
    public static URL[] getExtraTypes() {
        return new URL[0];
    }
}
