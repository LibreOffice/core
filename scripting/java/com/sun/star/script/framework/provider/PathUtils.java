/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PathUtils.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:00:49 $
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

package com.sun.star.script.framework.provider;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

import java.util.Vector;
import java.util.StringTokenizer;

import java.net.URLStreamHandler;
import java.net.URL;
import java.net.URLDecoder;
import java.net.MalformedURLException;

import java.lang.reflect.Method;

import com.sun.star.uno.XComponentContext;

import com.sun.star.script.framework.log.LogUtils;


import com.sun.star.util.XMacroExpander;
import com.sun.star.uno.Type;
import com.sun.star.uno.AnyConverter;
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
                Method getOid = IQueryInterface.class.getMethod("getOid", null);
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
