/*************************************************************************
 *
 *  $RCSfile: RegistrationClassFinder.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dbo $ $Date: 2000-11-16 09:09:13 $
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
package com.sun.star.comp.loader;

import com.sun.star.lib.sandbox.ClassContext;
import com.sun.star.lib.sandbox.ClassContextProxy;
import com.sun.star.lib.sandbox.Resource;
import com.sun.star.lib.sandbox.ResourceProxy;

public class RegistrationClassFinder {
    final static boolean DEBUG = false;

       protected ClassContext   m_context       = null;
    protected String        m_locationUrl   = null;
    protected String        m_manifest      = null;
    protected String        m_className     = null;

    public RegistrationClassFinder( String locationUrl )
        throws  java.io.IOException,
                java.net.MalformedURLException
    {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".<init>:" + locationUrl);

        m_locationUrl = locationUrl;

        if(locationUrl.endsWith(".jar")) {
            m_context = ClassContextProxy.create(new java.net.URL(m_locationUrl), null, null);
            m_manifest = locationUrl.substring(0, locationUrl.lastIndexOf('/') + 1) + "META-INF/MANIFEST.MF";
        }
    }

    private static String s_accessPath[];
    private static boolean s_bInit = false;

    private final static boolean checkAccessPath( java.net.URL url )
    {
        // init access path
        if (! s_bInit)
        {
            String accessPath[] = null;

            String env = System.getProperty( "com.sun.star.comp.loader.CPLD_ACCESSPATH" );
            if (env != null)
            {
                int nPos = 0;
                java.util.StringTokenizer tokens = new java.util.StringTokenizer( env, ";" );
                accessPath = new String[ tokens.countTokens() ];

                try
                {
                    while (tokens.hasMoreTokens())
                    {
                        try
                        {
                            accessPath[ nPos ] =
                                (new java.io.File( tokens.nextToken() )).getCanonicalPath();
                            ++nPos;
                        }
                        catch (java.io.IOException exc)
                        {
                        }
                    }
                }
                catch (java.util.NoSuchElementException exc)
                {
                }

                if (nPos != accessPath.length)
                {
                    // realloc accessPath to nPos
                    String ar[] = new String[ nPos ];
                    System.arraycopy( accessPath, 0, ar, 0, nPos );
                    accessPath = ar;
                }
            }

            s_accessPath = accessPath;
            s_bInit = true;

            if (DEBUG && s_accessPath != null)
            {
                System.err.print( "> CPLD_ACCESSPATH: " );
                for ( int nPos = 0; nPos < s_accessPath.length; ++nPos )
                {
                    System.err.print( "\"" + s_accessPath[ nPos ] + "\" " );
                }
                System.err.println();
            }
        }

        if (s_accessPath == null)
            return true; // no CPLD_ACCESSPATH set

        if (! url.getProtocol().equals( "file" ))
        {
            if (DEBUG)
                System.err.println( "> \"" + url.toExternalForm() + "\" is no file url!" );
            return false;
        }

        String surl;

        try
        {
            surl = (new java.io.File( url.getFile() )).getCanonicalPath();
        }
        catch (java.io.IOException exc)
        {
            if (DEBUG)
                System.err.println( "> \"" + url.toExternalForm() + "\" cannot be resolved!" );
            return false;
        }

        if (DEBUG)
            System.err.print( "> java loader looking up: \"" + surl + "\"..." );

        // check if jar is in access path
        for ( int nPos = 0; nPos < s_accessPath.length; ++nPos )
        {
            String path = s_accessPath[ nPos ];
            if (0 == surl.indexOf( path ) &&
                surl.length() > path.length() &&
                (path.charAt( path.length() -1 ) == java.io.File.separatorChar ||
                 surl.charAt( path.length() ) == java.io.File.separatorChar)) // dir boundary
            {
                if (DEBUG)
                    System.err.println( "succeeded!" );
                return true;
            }
        }

        if (DEBUG)
            System.err.println( "failed!" );

        return false;
    }

    public Class getRegistrationClass()
        throws  java.io.IOException,
                java.lang.ClassNotFoundException,
                java.net.MalformedURLException
    {
        Class ret = null;

        if (m_context != null) {
            String className = null;

            java.net.URL url = new java.net.URL(m_locationUrl);
            if (! checkAccessPath( url ))
                throw new ClassNotFoundException( "jar access failed!" );
              Resource resource = ResourceProxy.load(url, null);
              resource.loadJar(url);

              java.io.InputStream inManifest = ResourceProxy.load(new java.net.URL(m_manifest), null).getInputStream();
            java.io.BufferedReader manifestReader = new java.io.BufferedReader(new java.io.InputStreamReader(inManifest));

            String line;
            while ((line = manifestReader.readLine()) != null) {
                if (line.startsWith("RegistrationClassName: ")) {
                    className = line.substring("RegistrationClassName: ".length() );
                    break;
                }
            }

            if (className != null) {
                ret = m_context.loadClass( className );
            }
        }
        else
            ret = Class.forName(m_locationUrl);

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".getRegistrationClass:" + ret);

        return ret;
    }

    public Class loadClass( String className )
        throws  java.lang.ClassNotFoundException,
                java.io.IOException,
                java.net.MalformedURLException
    {
        Class ret = null;

        if (m_context != null) {
            java.net.URL url = new java.net.URL( m_locationUrl );
            if (! checkAccessPath( url ))
                throw new ClassNotFoundException( "jar access failed!" );
             Resource resource = ResourceProxy.load(url, null);
             resource.loadJar(url);

            ret = m_context.loadClass(className);
        } else {
            ret = Class.forName(className);
        }

        return ret;
    }
}
