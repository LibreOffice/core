/*************************************************************************
 *
 *  $RCSfile: PathUtils.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 14:01:29 $
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
