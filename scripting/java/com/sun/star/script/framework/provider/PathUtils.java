/*************************************************************************
 *
 *  $RCSfile: PathUtils.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dfoster $ $Date: 2003-10-09 14:37:47 $
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

import com.sun.star.uno.XComponentContext;

import com.sun.star.script.framework.log.LogUtils;

import com.sun.star.util.XMacroExpander;
import com.sun.star.uno.Type;
import com.sun.star.uno.AnyConverter;
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

    private PathUtils() {
    }

    public static String replaceWindowsPathSeparators( String path ) {
        if ( m_windows ) {
            path = path.replace( File.separatorChar, '/' );
        }
        return path;
    }

    public static InputStream getScriptFileStream( String pathURL, XComponentContext ctxt ) throws MalformedURLException, IOException
    {
        URL url = createScriptURL( pathURL, ctxt );
        return url.openStream();
    }

    public static URL createScriptURL( String pathURL, XComponentContext ctxt ) throws MalformedURLException
    {
        URL url = null;
        if ( pathURL.startsWith( UCBStreamHandler.UCBSCHEME ) )
        {
            URLStreamHandler handler = new UCBStreamHandler( ctxt );
            pathURL += UCBStreamHandler.separator;
            url  = new URL(null, pathURL, handler);
        }
        else
        {
            url =  new URL( pathURL );
        }

        return url;
    }

    private static String createPathURLRelativeToParcel( String parcelURI, String relativePart  )
    {
        LogUtils.DEBUG("**** createPathURLRelativeToParcel() parcel = " + parcelURI + " path = " + relativePart );
        String path = null;
        String dummyPath =  m_windows ? "z:" : "/dummy";


        File f = new File ( dummyPath, relativePart );
        try
        {
            path = f.getCanonicalPath();
            LogUtils.DEBUG("createPathURLRelativeToParcel() canonical path = " + path );
            // strip off dummy path prefix
            path = path.substring( dummyPath.length() );
            path = replaceWindowsPathSeparators( path );
            // strip off leading "/" if present, parcel always has
            // trailing "/" and "//" causes problems with ucb on some platforms
            if ( path.startsWith("/") )
            {
                path = path.substring( 1 );
            }
            LogUtils.DEBUG("createPathURLRelativeToParcel() relative portion of path = " + path );
        }
        catch (IOException ioe)
        {
            LogUtils.DEBUG("Caught exception " + ioe );
            return null;
        }
        if ( path == null )
        {
            return null;
        }
        path = parcelURI + path;

        if ( !path.endsWith(".jar") && !path.endsWith("/"))
        {
            path += "/";
        }

        LogUtils.DEBUG("createPathURLRelativeToParcel() path url = " + path );
        return path;
    }

    public static Vector buildClasspath(String parcelURI, String classpath)
        throws MalformedURLException
    {
        // ParcelURI should always end with Seperator ( it's a directory )
        if ( !parcelURI.endsWith("/") )
        {
            parcelURI += "/";
        }

        Vector result = new Vector();
        StringTokenizer stk = new StringTokenizer(classpath, ":");

        while (stk.hasMoreElements()) {
            String path = "";
            String relativeClasspath =  (String)stk.nextElement();
            path = createPathURLRelativeToParcel( parcelURI, relativeClasspath );
            if ( path != null )
            {
                result.add(path);
            }
        }

        if (result.size() == 0) {
            result.add(parcelURI);
        }

        LogUtils.DEBUG("buildClassPath() returning vector of size "
            + result.size() );

        return result;
    }
    /**
    *  Returns a path with ./ and ../ relative path directives evaluated.
    *
    * @return    path with ./ and ../ relative path directives evaluated.
    *            The path passed in must not start with "../", null is returned     *            in this case.
    */
    static private String processRelativePath( String path )
    {
        Vector pathComponents = new Vector();
        boolean isJarPath = false;
        if ( path.endsWith( ".jar" ) )
        {
            isJarPath = true;
        }
        StringTokenizer stk =  new StringTokenizer( path, "/" );
        while( stk.hasMoreElements() )
        {
            String pathPart = (String)stk.nextElement();

            if ( pathPart.equals( ".." ) )
            {
                if ( pathComponents.isEmpty() )
                {
                    //System.err.println( "Illegal path, path when evaluated will point beyond start of path ->" + path );
                    return null;
                }
                pathComponents.removeElementAt( pathComponents.size() - 1 );
            }
            else
            {
                if ( !pathPart.equals(".") )
                {
                    pathComponents.add( pathPart );
                }
            }
        }

        String returnPath = "";
        for ( int i = 0; i < pathComponents.size(); i++ )
        {
            if ( i == 0 )
            {
               returnPath =  ( String )pathComponents.elementAt( 0 );
            }
            else
            {
                returnPath = returnPath + "/" + ( String )pathComponents.elementAt( i );
            }
        }

        if ( returnPath.length() > 0 && !isJarPath )
        {
            returnPath += "/";
        }
        return returnPath;
    }
    public static String toScriptLocation(XComponentContext ctxt, String path) {
        String sharePath = getSharePath(ctxt);
        String userPath = getUserPath(ctxt);

        LogUtils.DEBUG("toScriptLocation, path: " + path);
        LogUtils.DEBUG("toScriptLocation, share path: " + sharePath);
        LogUtils.DEBUG("toScriptLocation, user path: " + userPath);

        try {
            path = URLDecoder.decode(path, "UTF-8");
        }
        catch (java.io.UnsupportedEncodingException ignore) {
            // ignore
        }

        if (m_windows) {
            path = replaceWindowsPathSeparators(path);
        }

        if (path.indexOf(sharePath) != -1) {
            return "share";
        }
        else if (path.indexOf(userPath) != -1) {
            return "user";
        }
        return path;
    }

    private static String getOfficeURL(XComponentContext ctxt, String name) {

        String result = null;

        try {
            Object serviceObj = ctxt.getValueByName(
                "/singletons/com.sun.star.util.theMacroExpander");

            LogUtils.DEBUG("got serviceObj");
            XMacroExpander me = (XMacroExpander) AnyConverter.toObject(
                new Type(XMacroExpander.class), serviceObj);

            result = me.expandMacros(name);
        }
        catch (com.sun.star.lang.IllegalArgumentException iae) {
            LogUtils.DEBUG("Caught IllegalArgumentException trying to " +
                "expand dir: " + name);
        }

        return result;
    }

    public static String getShareURL(XComponentContext ctxt) {
        return getOfficeURL(ctxt,
            "${$SYSBINDIR/" + BOOTSTRAP_NAME +
            "::BaseInstallation}/share");
    }

    public static String getSharePath(XComponentContext ctxt) {
        String s = getShareURL(ctxt);

        if (s.startsWith(FILE_URL_PREFIX)) {
            s = s.substring(FILE_URL_PREFIX.length());
        }

        return s;
    }

    public static String getUserURL(XComponentContext ctxt) {
        return getOfficeURL(ctxt,
            "${$SYSBINDIR/" + BOOTSTRAP_NAME +
            "::UserInstallation}/user");
    }

    public static String getUserPath(XComponentContext ctxt) {
        String s = getUserURL(ctxt);

        if (s.startsWith(FILE_URL_PREFIX)) {
            s = s.substring(FILE_URL_PREFIX.length());
        }

        return s;
    }
}
