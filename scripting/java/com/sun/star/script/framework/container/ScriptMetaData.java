/*************************************************************************
 *
 *  $RCSfile: ScriptMetaData.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-19 08:22:23 $
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

package com.sun.star.script.framework.container;
import java.net.URLEncoder;
import java.net.URLDecoder;
import java.net.URLDecoder;
import java.net.URL;

import java.util.Vector;
import java.util.StringTokenizer;
import java.net.URLStreamHandler;

import java.io.File;
import java.io.InputStream;
import java.io.FileOutputStream;
import java.io.OutputStream;

import com.sun.star.script.framework.log.LogUtils;

import com.sun.star.script.framework.provider.PathUtils;


import com.sun.star.script.framework.container.ScriptEntry;
import com.sun.star.script.framework.container.Parcel;

import com.sun.star.script.framework.io.XStorageHelper;
import com.sun.star.script.framework.io.XStorageStreamHandler;
import com.sun.star.script.framework.io.XOutputStreamWrapper;


import com.sun.star.embed.XStorage;
import com.sun.star.embed.ElementModes;
import com.sun.star.io.XStream;


public class ScriptMetaData extends ScriptEntry implements Cloneable {
    private boolean hasSource = false;
    private String locationPlaceHolder = "";
    private String source;
    private Parcel parent;


    static private String VNDPREFIX = XStorageStreamHandler.XSTORAGESCHEME + "://";

    public ScriptMetaData( Parcel parent, ScriptEntry entry,
                           String source )
    {
        super( entry );
        this.parent = parent;
        if ( source != null )
        {
            this.hasSource = true;
            this.source = source;
        }

    }

    public boolean hasSource()
    {
        return hasSource;
    }
    public String getSource()
    {

        if ( source !=null && hasSource )
        {
            return source;
        }
        else
        {
            return null;
        }
    }

    public byte[] getSourceBytes()
    {
        if ( source !=null && hasSource )
        {
            return source.getBytes();
        }
        else
        {
            return null;
        }

    }
    public Object clone() throws CloneNotSupportedException {
        return super.clone();
    }

    public boolean equals(ScriptMetaData other) {
        if (super.equals(other) &&
            hasSource == other.hasSource() )
        {
            return true;
        }
        return false;
    }

    public String getScriptFullURL()
    {
        String url = "vnd.sun.star.script:" + parent.getName() + "." + getLanguageName() +
            "?" + "language=" + getLanguage() +
            "&location=" + getParcelLocation();
         return url;
    }

    public String getShortFormScriptURL()
    {
        String url = "vnd.sun.star.script:" + parent.getName() + "." + getLanguageName() +
            "?" + "language=" + getLanguage() +
            "&location=" + getLocationPlaceHolder();
        return url;
    }

    // TODO probably should be private should not be necessary
    // to be exposed at all

    public String getLocationPlaceHolder()
    {
        String placeHolder = parent.getParcelLocationMacro();
        return placeHolder;
    }

    // TODO probably should be private should not be necessary
    // to be exposed at all only used in lang providers at the moment
    // to generate URL for script, editors should use a model of script
    // source and not interact with the URL
    // Also if it is to remain needs to be renamed to getParcelLocationURL

    // return  URL string  to parcel
    public String getParcelLocation()
    {
        String temp = parent.getPathToParcel();
        if ( getLocationPlaceHolder().equals("document") )
        {
           temp = VNDPREFIX + temp;
        }
        else
        {
            temp = PathUtils.FILE_URL_PREFIX + temp;
        }

        return temp;
    }


    public String toString()
    {
        return "\nParcelLocation = " + getParcelLocation() + "\nLocationPlaceHolder = " + locationPlaceHolder + super.toString();
    }

    public URL[] getClassPath() throws java.net.MalformedURLException
    {
    try
    {
        String classpath = (String)getLanguageProperties().get("classpath");
        Vector paths = null;

        if ( classpath == null )
        {
            classpath = "";
        }

        String parcelPath = getParcelLocation();
        // make sure path ends with /
        if ( !parcelPath.endsWith("/") )
        {
            parcelPath += "/";
        }

        Vector classPathVec =  new Vector();
        StringTokenizer stk = new StringTokenizer(classpath, ":");
        while (  stk.hasMoreElements() )
        {
            String relativeClasspath =  (String)stk.nextElement();
            String pathToProcess  = parcelPath + relativeClasspath;
            URL url = createURL( pathToProcess );
            if ( url != null )
            {
                classPathVec.add (  url  );
            }

        }
        if ( classPathVec.size() == 0)
        {
            URL url = createURL( parcelPath );
            if ( url != null )
            {
                classPathVec.add(url);
            }
        }

        return  (URL[])classPathVec.toArray( new URL[0]);
    }
    catch ( Exception e )
    {
        LogUtils.DEBUG("Failed to build class path " + e.toString() );
        LogUtils.DEBUG( LogUtils.getTrace( e ) );
        return new URL[0];
    }

    }
    private URL createURL( String path ) throws java.net.MalformedURLException
    {
        SFMacroHelper expander = parent.getParent().expander;
        URL url = null;
        boolean isDocument = getLocationPlaceHolder().equals( "document" );
        if ( isDocument )
        {
            // strip document scheme from URL
            // so it can be passed to getSystemPath
            path = path.substring( VNDPREFIX.length() );
        }

        // fix ../../ spaces etc in path
        else
        {
            LogUtils.DEBUG("expanded path: "+path);
            path = expander.getSystemPath( path );
            path = PathUtils.FILE_URL_PREFIX + path;
            LogUtils.DEBUG("---->panded path: "+path);
        }
        // replace \ with /
        path = path.replace( '\\', '/' );
        // getSystemPath removes file:// add this back

        if ( path.length()  > 0 )
        {

            if ( isDocument )
            {
                // put back scheme
                path = VNDPREFIX + path;

                // create URL
                URLStreamHandler handler = new XStorageStreamHandler();
                path += XStorageStreamHandler.separator;
                url = new URL(null, path, handler);
            }
            else
            {
                // application file url
                url = new URL( path );
            }
        }
        return url;
    }

    // TODO should decide whether this should throw or not
    // decide whether it should be public or protected ( final ? )
    public void loadSource()
    {
            try
            {
                LogUtils.DEBUG("** In load source ");
                URL sourceUrl = getSourceURL();

                if ( sourceUrl != null )
                {
                    StringBuffer buf = new StringBuffer();
                    InputStream in = sourceUrl.openStream();

                    byte[] contents = new byte[1024];
                    int len = 0;

                    while ((len = in.read(contents, 0, 1024)) != -1) {
                        buf.append(new String(contents, 0, len));
                    }

                    try {
                        in.close();
                    }
                    catch (java.io.IOException ignore ) {
                        LogUtils.DEBUG("** Failed to read scriot from url " + ignore.toString() );
                    }

                    source = buf.toString();
                    hasSource = true;
                }
            }
            catch (java.io.IOException e) {
                LogUtils.DEBUG("** Failed to read scriot from url " + e.toString());
            }

        }
    protected boolean writeSourceFile()
    {
        String parcelLocation = parent.getPathToParcel();
        String sourceFileName = getLanguageName();
        boolean result = false;

        boolean isDocument = getLocationPlaceHolder().equals("document");

        if ( isDocument ) // its a document
        {
            XStorageHelper helper =  null;
            XStream parcelStream = null;
            XStream sourceStream = null;
            boolean commitFlag = true;
            try
            {

                helper = new XStorageHelper( parcelLocation, ElementModes.READWRITE, true );
                sourceStream = helper.getStorage().openStreamElement( getLanguageName(), ElementModes.READWRITE | ElementModes.TRUNCATE);
                if ( sourceStream != null )
                {
                    OutputStream os = new XOutputStreamWrapper(sourceStream.getOutputStream());
                    os.write( getSourceBytes() );
                    os.close();
                 }
                result = true;
            }
            catch( Exception e )
            {

                LogUtils.DEBUG("Caught exception whild trying to writer sourceStream " + e);
                commitFlag = false;
            }

            finally
            {
                if ( parcelStream != null )
                {
                    try
                    {
                        XStorageHelper.disposeObject( parcelStream );
                    }
                    catch( Exception ignore )
                    {
                    }
                }
                if ( sourceStream != null )
                {

                    try
                    {
                        XStorageHelper.disposeObject( sourceStream );
                    }
                    catch( Exception ignore )
                    {
                    }
                }
                if ( helper != null )
                {

                    try
                    {
                        helper.disposeObject( commitFlag );
                    }
                    catch( Exception ignore )
                    {
                    }
               }
               helper =  null;
               sourceStream = null;
               parcelStream = null;
            }
        }
        else
        {
            FileOutputStream fos = null;
            try
            {
                File scriptFile = new File( parcelLocation, sourceFileName );
                fos = new FileOutputStream( scriptFile );
                fos.write( getSourceBytes() );
                fos.close();
                fos = null;
                result = true;
            }
            catch( Exception ignore )
            {
            }
            finally
            {
                if ( fos != null )
                {
                    try
                    {
                        fos.close();
                    }
                    catch ( Exception ignore )
                    {
                    }

                }
            }

        }
        return result;
    }
    protected boolean removeSourceFile()
    {
        String parcelLocation = parent.getPathToParcel();
        String sourceFileName = getLanguageName();
        boolean result = false;
        boolean isDocument = getLocationPlaceHolder().equals( "document" );
        if ( isDocument ) // its a document
        {
            XStorageHelper xParcelDirStorageHelper = null;
            boolean commitFlag = true;
            try
            {
                xParcelDirStorageHelper = new XStorageHelper( parcelLocation, ElementModes.READWRITE, false );
                xParcelDirStorageHelper.getStorage().removeElement( sourceFileName );
                result = true;
            }
            catch ( Exception e )
            {
                LogUtils.DEBUG("Caught exception removing script " + sourceFileName );
            }
            finally
            {
                try
                {
                    xParcelDirStorageHelper.disposeObject( commitFlag );
                }
                catch ( Exception ignore )
                {

                }

            }

        }
        else
        {
            File scriptFile = new File( parcelLocation, sourceFileName );
            result = scriptFile.delete();
        }
        return result;
    }

    public URL getSourceURL() throws java.net.MalformedURLException
    {
        String sUrl = null;
        URL scriptURL = null;

        sUrl = getParcelLocation();
        if ( !sUrl.endsWith( "/" ) )
        {
            sUrl += "/";
        }
        sUrl +=  getLanguageName();
        scriptURL = createURL( sUrl );
        return scriptURL;
    }
}
