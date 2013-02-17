/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package com.sun.star.script.framework.container;

import java.net.URL;

import java.io.ByteArrayInputStream;

import java.util.ArrayList;
import java.util.StringTokenizer;

import java.io.InputStream;
import java.io.OutputStream;

import com.sun.star.script.framework.log.LogUtils;

import com.sun.star.script.framework.provider.PathUtils;

import com.sun.star.script.framework.io.XInputStreamImpl;

import com.sun.star.script.framework.container.ScriptEntry;
import com.sun.star.script.framework.container.Parcel;

import com.sun.star.script.framework.io.UCBStreamHandler;

import com.sun.star.ucb.XSimpleFileAccess2;

import com.sun.star.uno.UnoRuntime;

public class ScriptMetaData extends ScriptEntry implements Cloneable {
    private boolean hasSource = false;
    private String locationPlaceHolder = "";
    private String source;
    private Parcel parent;


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

    private static final String SHARE =
        "vnd.sun.star.expand:$BRAND_BASE_DIR/$BRAND_SHARE_SUBDIR";

    private static final String USER =
        "vnd.sun.star.expand:${$BRAND_BASE_DIR/BRAND_BIN_SUBDIR/" +
        PathUtils.BOOTSTRAP_NAME +
        "::UserInstallation}/user";

    private static final String UNO_USER_PACKAGES1 =
        "vnd.sun.star.expand:$UNO_USER_PACKAGES_CACHE";

    private static final String UNO_USER_PACKAGES2 =
        USER + "/uno_packages";

    private static final String UNO_SHARED_PACKAGES1 =
        "$UNO_SHARED_PACKAGES_CACHE";

    private static final String UNO_SHARED_PACKAGES2 =
        SHARE + "/uno_packages";

    public static String getLocationPlaceHolder(String url, String pkgname)
    {
        String result = "Unknown";

        if ( url.indexOf(UNO_USER_PACKAGES1) > -1 ||
             url.indexOf(UNO_USER_PACKAGES2) > -1 )
        {
            result = PathUtils.make_url( "user:uno_packages", pkgname );
        }
        else if ( url.indexOf(UNO_SHARED_PACKAGES1) > -1 ||
                  url.indexOf(UNO_SHARED_PACKAGES2) > -1 )
        {
            result = PathUtils.make_url( "share:uno_packages", pkgname );
        }
        else if ( url.indexOf(SHARE) == 0 )
        {
            result = "share";
        }
        else if ( url.indexOf(USER) == 0 )
        {
            result = "user";
        }
        else if ( url.indexOf("vnd.sun.star.tdoc:") == 0 )
        {
            result = "document";
        }
        return result;
    }

    public String getLocationPlaceHolder()
    {
        String placeHolder = "Unknown";
        String pathToParcel = parent.getPathToParcel();

        if ( pathToParcel.indexOf(UNO_USER_PACKAGES1) > -1 ||
             pathToParcel.indexOf(UNO_USER_PACKAGES2) > -1 )
        {
            // its a package
            placeHolder = "user:uno_packages";
            String unoPkg = parent.parent.getName();
            if ( unoPkg != null )
            {
                placeHolder = PathUtils.make_url( placeHolder, unoPkg );
            }
        }
        else if ( pathToParcel.indexOf(UNO_SHARED_PACKAGES1) > -1 ||
                  pathToParcel.indexOf(UNO_SHARED_PACKAGES2) > -1 )
        {
            //its a package
            placeHolder = "share:uno_packages";
            String unoPkg = parent.parent.getName();
            if ( unoPkg != null )
            {
                placeHolder = PathUtils.make_url( placeHolder, unoPkg );
            }
        }
        else if ( pathToParcel.indexOf(SHARE) == 0 )
        {
            placeHolder = "share";
        }
        else if ( pathToParcel.indexOf(USER) == 0 )
        {
            placeHolder = "user";
        }
        else if ( pathToParcel.indexOf("vnd.sun.star.tdoc:") == 0 )
        {
            placeHolder = "document";
        }
        // TODO handling document packages ??? not really sure of package url
/*        else
        {
        } */
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
        return parent.getPathToParcel();
    }


    public String toString()
    {
        return "\nParcelLocation = " + getParcelLocation() + "\nLocationPlaceHolder = " + locationPlaceHolder + super.toString();
    }

    public URL[] getClassPath() throws java.net.MalformedURLException
    {
    try
    {
        String classpath = getLanguageProperties().get("classpath");

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

        // replace \ with /
        parcelPath = parcelPath.replace( '\\', '/' );

        ArrayList<URL> classPathVec =  new ArrayList<URL>();
        StringTokenizer stk = new StringTokenizer(classpath, ":");
        while (  stk.hasMoreElements() )
        {
            String relativeClasspath =  stk.nextToken();
            String pathToProcess  = PathUtils.make_url( parcelPath, relativeClasspath);
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

        return  classPathVec.toArray( new URL[classPathVec.size()]);
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
        URL url = null;
        int indexOfColon = path.indexOf(":");
        String scheme = path.substring( 0, indexOfColon );
        UCBStreamHandler handler = new UCBStreamHandler( parent.parent.m_xCtx, scheme, parent.m_xSFA);

        path += UCBStreamHandler.separator;
        url = new URL(null, path, handler);
        return url;
    }

    // TODO should decide whether this should throw or not
    // decide whether it should be public or protected ( final ? )
    public void loadSource()
    {
            try
            {
                URL sourceUrl = getSourceURL();
                LogUtils.DEBUG("** In load source BUT not loading yet for " + sourceUrl );

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
        String sourceFilePath = parent.getPathToParcel() + "/" + getLanguageName();
        boolean result = false;
        OutputStream os = null;
        try
        {
            XSimpleFileAccess2 xSFA2 = UnoRuntime.queryInterface( XSimpleFileAccess2.class,
                parent.m_xSFA );
            if ( xSFA2 != null )
            {
                ByteArrayInputStream bis = new ByteArrayInputStream( getSourceBytes() );
                XInputStreamImpl xis = new XInputStreamImpl( bis );
                xSFA2.writeFile( sourceFilePath, xis );
                xis.closeInput();
                result = true;
            }
        }
        // TODO re-examine exception processing should probably throw
        // exceptions back to caller
        catch ( Exception ignore )

        {
        }
        return result;
    }
    protected boolean removeSourceFile()
    {
        String parcelLocation = parent.getPathToParcel();
        String sourceFilePath = parcelLocation + "/" + getLanguageName();
        boolean result = false;
        try
        {
            parent.m_xSFA.kill( sourceFilePath );
            result = true;
        }
        // TODO reexamine exception handling
        catch ( Exception e )
        {
        }
        return result;
    }

    public URL getSourceURL() throws java.net.MalformedURLException
    {
        String sUrl = null;
        URL scriptURL = null;

        sUrl = getParcelLocation();
        sUrl = PathUtils.make_url( sUrl, getLanguageName() );
        LogUtils.DEBUG("Creating script url for " + sUrl );
        scriptURL = createURL( sUrl );
        return scriptURL;
    }
}
