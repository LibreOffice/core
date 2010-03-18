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

package com.sun.star.script.framework.provider;

import java.util.StringTokenizer;

import java.lang.reflect.Method;

import com.sun.star.frame.XModel;
import com.sun.star.uno.IQueryInterface;

public class PathUtils {

    public static String FILE_URL_PREFIX;
    public static String BOOTSTRAP_NAME;
    private static boolean m_windows = false;

    static {
        // detect if windows platform
        if (System.getProperty("os.name").startsWith("Windows"))
            m_windows = true;

        FILE_URL_PREFIX = m_windows ? "file:///" : "file://";
        BOOTSTRAP_NAME = m_windows ? "bootstrap.ini" : "bootstraprc";
    }
    public static String getOidForModel( XModel xModel )
    {
        String oid = new String("");
        if ( xModel != null )
        {
            try
            {
                Method getOid = IQueryInterface.class.getMethod("getOid", (java.lang.Class[])null);
                if ( getOid != null )
                {
                    oid = (String)getOid.invoke( xModel, new Object[0] );
                }

            }
            catch ( Exception ignore )
            {
            }
        }
        return oid;
    }
    static  public String make_url( String baseUrl, String url )
    {
        StringBuffer buff = new StringBuffer( baseUrl.length() + url.length() );        buff.append( baseUrl );
        StringTokenizer t = new StringTokenizer( url, "/");
        while ( t.hasMoreElements() )
        {
           if ( buff.charAt( buff.length() - 1 ) != '/' )
           {
               buff.append('/');
           }
           buff.append( java.net.URLEncoder.encode( (String)t.nextElement() ) );        }
        return buff.toString();
    }

    private PathUtils() {
    }
}
