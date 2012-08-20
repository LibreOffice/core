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

package com.sun.star.beans;

import java.awt.Container;
import java.io.File;
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;

import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XEventListener;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lib.uno.helper.UnoUrl;
import com.sun.star.lib.util.NativeLibraryLoader;

/**
 * This class represents a connection to the local office application.
 * @deprecated
 */
public class LocalOfficeConnection
    implements OfficeConnection
{
    public static final String      OFFICE_APP_NAME     = "soffice";
    public static final String      OFFICE_LIB_NAME     = "officebean";
    public static final String      OFFICE_ID_SUFFIX    = "_Office";

    private Process             mProcess;
    private ContainerFactory        mContainerFactory;
    private XComponentContext       mContext;

    private String              mURL;
    private String              mProgramPath;
    private String              mConnType;
    private String              mPipe;
    private String              mPort;
    private String              mProtocol;
    private String              mInitialObject;

    private List<XEventListener> mComponents = new ArrayList<XEventListener>();

    /**
     * Constructor.
     * Sets up paths to the office application and native libraries if
     * values are available in <code>OFFICE_PROP_FILE</code> in the user
     * home directory.<br />
     * "com.sun.star.beans.path" - the office application directory;<br/>
     * "com.sun.star.beans.libpath" - native libraries directory.
     */
    public LocalOfficeConnection()
    {
        // init member vars
        try
        {
            setUnoUrl( "uno:pipe,name=" + getPipeName() + ";urp;StarOffice.ServiceManager" );
        }
        catch ( java.net.MalformedURLException e )
        {}

        // load libofficebean.so/officebean.dll
        String aSharedLibName = getProgramPath() + java.io.File.separator +
            System.mapLibraryName(OFFICE_LIB_NAME);
        System.load( aSharedLibName );
    }

    /**
     * Sets a connection URL.
     * This implementation accepts a UNO URL with following format:<br />
     * <pre>
     * url    := uno:localoffice[,&lt;params&gt;];urp;StarOffice.ServiceManager
     * params := &lt;path&gt;[,&lt;pipe&gt;]
     * path   := path=&lt;pathv&gt;
     * pipe   := pipe=&lt;pipev&gt;
     * pathv  := platform_specific_path_to_the_local_office_distribution
     * pipev  := local_office_connection_pipe_name
     * </pre>
     *
     * @param url This is UNO URL which discribes the type of a connection.
     */
    public void setUnoUrl(String url)
        throws java.net.MalformedURLException
    {
        mURL    = null;

        String prefix = "uno:localoffice";
        if ( url.startsWith(prefix) )
            parseUnoUrlWithOfficePath( url, prefix );
        else
        {
            try
            {
                UnoUrl aURL = UnoUrl.parseUnoUrl( url );
                mProgramPath = null;
                mConnType = aURL.getConnection();
                mPipe = aURL.getConnectionParameters().get( "pipe" );
                mPort = aURL.getConnectionParameters().get( "port" );
                mProtocol = aURL.getProtocol();
                mInitialObject = aURL.getRootOid();
            }
            catch ( com.sun.star.lang.IllegalArgumentException eIll )
            {
                throw new java.net.MalformedURLException(
                    "Invalid UNO connection URL.");
            }
        }
        mURL    = url;
    }

    /**
     * Sets an AWT container catory.
     *
     * @param containerFactory This is a application provided AWT container
     *  factory.
     */
    public void setContainerFactory(ContainerFactory containerFactory)
    {
        mContainerFactory   = containerFactory;
    }

    /**
     * Retrives the UNO component context.
     * Establishes a connection if necessary and initialises the
     * UNO service manager if it has not already been initialised.
     * This method can return <code>null</code> if it fails to connect
     * to the office application.
     *
     * @return The office UNO component context.
     */
    public XComponentContext getComponentContext()
    {
        if ( mContext == null )
            mContext = connect();
        return mContext;
    }

    /**
     * Creates an office window.
     * The window is either a sub-class of java.awt.Canvas (local) or
     * java.awt.Container (RVP).
     *
     * @param container This is an AWT container.
     * @return The office window instance.
     */
    public OfficeWindow createOfficeWindow(Container container)
    {
        return new LocalOfficeWindow(this);
    }

    /**
     * Closes the connection.
     */
    public void dispose()
    {
        Iterator<XEventListener> itr = mComponents.iterator();
        while (itr.hasNext()) {
            // ignore runtime exceptions in dispose
            try { itr.next().disposing(null); }
            catch ( RuntimeException aExc ) {}
        }
        mComponents.clear();

        mContainerFactory = null;
        mContext = null;
    }

    /**
     * Adds an event listener to the object.
     *
     * @param listener is a listener object.
     */
    public void addEventListener(XEventListener listener)
    {
        mComponents.add(listener);
    }

    /**
     * Removes an event listener from the listener list.
     *
     * @param listener is a listener object.
     */
    public void removeEventListener(XEventListener listener)
    {
        mComponents.remove(listener);
    }

    /**
     * Establishes the connection to the office.
     */
    private XComponentContext connect()
    {
        try
        {
            // create default local component context
            XComponentContext xLocalContext =
                com.sun.star.comp.helper.Bootstrap.createInitialComponentContext(null);

            // initial serviceManager
            XMultiComponentFactory xLocalServiceManager = xLocalContext.getServiceManager();

            // create a urlresolver
            Object urlResolver  = xLocalServiceManager.createInstanceWithContext(
                "com.sun.star.bridge.UnoUrlResolver", xLocalContext );

            // query for the XUnoUrlResolver interface
            XUnoUrlResolver xUrlResolver =
                UnoRuntime.queryInterface( XUnoUrlResolver.class, urlResolver );

            // try to connect to soffice
            Object aInitialObject = null;
            try
            {
                aInitialObject = xUrlResolver.resolve( mURL );
            }
            catch( com.sun.star.connection.NoConnectException e )
            {
                // launch soffice
                OfficeService aSOffice = new OfficeService();
                aSOffice.startupService();

                // wait until soffice is started
                long nMaxMillis = System.currentTimeMillis() + 1000*aSOffice.getStartupTime();
                while ( aInitialObject == null )
                {
                    try
                    {
                        // try to connect to soffice
                        Thread.sleep( 500 );
                        aInitialObject = xUrlResolver.resolve( mURL );
                    }
                    catch( com.sun.star.connection.NoConnectException aEx )
                    {
                        // soffice did not start in time
                        if ( System.currentTimeMillis() > nMaxMillis )
                            throw aEx;

                    }
                }
            }
            finally
            {
            }

            // XComponentContext
            if( null != aInitialObject )
            {
                XPropertySet xPropertySet = UnoRuntime.queryInterface( XPropertySet.class, aInitialObject);
                        Object xContext = xPropertySet.getPropertyValue("DefaultContext");
                        XComponentContext xComponentContext = UnoRuntime.queryInterface(
                    XComponentContext.class, xContext);
                return xComponentContext;
            }
        }
        catch( com.sun.star.connection.NoConnectException e )
        {
            System.out.println( "Couldn't connect to remote server" );
            System.out.println( e.getMessage() );
        }
        catch( com.sun.star.connection.ConnectionSetupException e )
        {
            System.out.println( "Couldn't access necessary local resource to establish the interprocess connection" );
            System.out.println( e.getMessage() );
        }
        catch( com.sun.star.lang.IllegalArgumentException e )
        {
            System.out.println( "uno-url is syntactical illegal ( " + mURL + " )" );
            System.out.println( e.getMessage() );
        }
        catch( com.sun.star.uno.RuntimeException e )
        {
            System.out.println( "--- RuntimeException:" );
            System.out.println( e.getMessage() );
            e.printStackTrace();
            System.out.println( "--- end." );
            throw e;
        }
        catch( java.lang.Exception e )
        {
            System.out.println( "java.lang.Exception: " );
            System.out.println( e );
            e.printStackTrace();
            System.out.println( "--- end." );
            throw new com.sun.star.uno.RuntimeException( e.toString() );
        }

        return null;
    }

    /**
     * Retrives a path to the office program folder.
     *
     * @return The path to the office program folder.
     */
    private String getProgramPath()
    {
        if (mProgramPath == null)
        {
            // determine name of executable soffice
            String aExec = OFFICE_APP_NAME; // default for UNIX
            String aOS = System.getProperty("os.name");

            // running on Windows?
            if (aOS.startsWith("Windows"))
                aExec = OFFICE_APP_NAME + ".exe";

            // add other non-UNIX operating systems here
            // ...

            // find soffice executable relative to this class's class loader:
            File path = NativeLibraryLoader.getResource(
                this.getClass().getClassLoader(), aExec);
            if (path != null) {
                mProgramPath = path.getParent();
        }

            // default is ""
            if ( mProgramPath == null )
                mProgramPath = "";
        }
        return mProgramPath;
    }

    /**
     * Parses a connection URL.
     * This method accepts a UNO URL with following format:<br />
     * <pre>
     * url    := uno:localoffice[,&lt;params&gt;];urp;StarOffice.NamingService
     * params := &lt;path&gt;[,&lt;pipe&gt;]
     * path   := path=&lt;pathv&gt;
     * pipe   := pipe=&lt;pipev&gt;
     * pathv  := platform_specific_path_to_the_local_office_distribution
     * pipev  := local_office_connection_pipe_name
     * </pre>
     *
     * <h4>Examples</h4>
     * <ul>
     *  <li>"uno:localoffice,pipe=xyz_Office,path=/opt/openoffice11/program;urp;StarOffice.ServiceManager";
     *  <li>"uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager";
     * </ul>
     *
     * @param url This is UNO URL which describes the type of a connection.
     * @exception java.net.MalformedURLException when inappropreate URL was
     *  provided.
     */
    private void parseUnoUrlWithOfficePath(String url, String prefix)
        throws java.net.MalformedURLException
    {
        // Extruct parameters.
        int idx = url.indexOf(";urp;StarOffice.NamingService");
        if (idx < 0)
            throw new java.net.MalformedURLException(
                "Invalid UNO connection URL.");
        String  params  = url.substring(prefix.length(), idx + 1);

        // Parse parameters.
        String  name    = null;
        String  path    = null;
        String  pipe    = null;
        char    ch;
        int     state   = 0;
        StringBuffer    buffer  = new StringBuffer();
        for(idx = 0; idx < params.length(); idx += 1) {
            ch  = params.charAt(idx);
            switch (state) {
            case 0: // initial state
                switch(ch) {
                case ',':
                    buffer.delete(0, buffer.length());
                    state   = 1;
                    break;

                case ';':
                    state   = 7;
                    break;

                default:
                    buffer.delete(0, buffer.length());
                    buffer.append(ch);
                    state   = 1;
                    break;
                }
                break;

            case 1: // parameter name
                switch(ch) {
                case ' ':
                case '=':
                    name    = buffer.toString();
                    state   = (ch == ' ')? 2: 3;
                    break;

                case ',':
                case ';':
                    state   = -6;           // error: invalid name
                    break;

                default:
                    buffer.append(ch);
                    break;
                }
                break;

            case 2: // equal between the name and the value
                switch(ch) {
                case '=':
                    state   = 3;
                    break;

                case ' ':
                    break;

                default:
                    state   = -1;           // error: missing '='
                    break;
                }
                break;

            case 3: // value leading spaces
                switch(ch) {
                case ' ':
                    break;

                default:
                    buffer.delete(0, buffer.length());
                    buffer.append(ch);
                    state   = 4;
                    break;
                }
                break;

            case 4: // value
                switch(ch) {
                case ' ':
                case ',':
                case ';':
                    idx     -= 1;           // put back the last read character
                    state   = 5;
                    if (("path").equals(name)) {
                        if (path == null)
                            path    = buffer.toString();
                        else
                            state   = -3;   // error: more then one 'path'
                    } else if (("pipe").equals(name)) {
                        if (pipe == null)
                            pipe    = buffer.toString();
                        else
                            state   = -4;   // error: more then one 'pipe'
                    } else
                        state   = -2;       // error: unknown parameter
                    buffer.delete(0, buffer.length());
                    break;

                default:
                    buffer.append(ch);
                    break;
                }
                break;

            case 5: // a delimeter after the value
                switch(ch) {
                case ' ':
                    break;

                case ',':
                    state   = 6;
                    break;

                case ';':
                    state   = 7;
                    break;

                default:
                    state   = -5;           // error: ' ' inside the value
                    break;
                }
                break;

            case 6: // leading spaces before next parameter name
                switch(ch) {
                case ' ':
                    break;

                default:
                    buffer.delete(0, buffer.length());
                    buffer.append(ch);
                    state   = 1;
                    break;
                }
                break;

            default:
                throw new java.net.MalformedURLException(
                    "Invalid UNO connection URL.");
            }
        }
        if (state != 7)
            throw new java.net.MalformedURLException(
                "Invalid UNO connection URL.");

        // Set up the connection parameters.
        if (path != null)
            mProgramPath = path;
        if (pipe != null)
            mPipe = pipe;
    }

    /* replaces each substring aSearch in aString by aReplace.

        StringBuffer.replaceAll() is not avaialable in Java 1.3.x.
     */
    private static String replaceAll(String aString, String aSearch, String aReplace )
    {
        StringBuffer aBuffer = new StringBuffer(aString);

        int nPos = aString.length();
        int nOfs = aSearch.length();

        while ( ( nPos = aString.lastIndexOf( aSearch, nPos - 1 ) ) > -1 )
            aBuffer.replace( nPos, nPos+nOfs, aReplace );

        return aBuffer.toString();
    }


    /** creates a unique pipe name.
    */
    static String getPipeName()
    {
        // turn user name into a URL and file system safe name (% chars will not work)
        String aPipeName = System.getProperty("user.name") + OFFICE_ID_SUFFIX;
        aPipeName = replaceAll( aPipeName, "_", "%B7" );
        return replaceAll( replaceAll( java.net.URLEncoder.encode(aPipeName), "\\+", "%20" ), "%", "_" );
    }

    /**
     * @para This is an implementation of the native office service.
     * @deprecated
     */
    private class OfficeService
        implements NativeService
    {
        /**
         * Retrive the office service identifier.
         *
         * @return The identifier of the office service.
         */
        public String getIdentifier()
        {
            if ( mPipe == null)
                return getPipeName();
            else
                return mPipe;
        }

        /**
         * Starts the office process.
         */
        public void startupService()
            throws java.io.IOException
        {
           // create call with arguments
            String[] cmdArray = new String[4];
            cmdArray[0] = (new File(getProgramPath(), OFFICE_APP_NAME)).getPath();
            cmdArray[1] = "--nologo";
            cmdArray[2] = "--nodefault";
            if ( mConnType.equals( "pipe" ) )
                cmdArray[3] = "--accept=pipe,name=" + getIdentifier() + ";" +
                          mProtocol + ";" + mInitialObject;
            else if ( mConnType.equals( "socket" ) )
                cmdArray[3] = "--accept=socket,port=" + mPort + ";urp";
            else
                throw new java.io.IOException( "not connection specified" );

            // start process
            mProcess = Runtime.getRuntime().exec(cmdArray);
            if ( mProcess == null )
                throw new RuntimeException( "cannot start soffice: " + cmdArray );
        }

        /**
         * Retrives the ammount of time to wait for the startup.
         *
         * @return The ammount of time to wait in seconds(?).
         */
        public int getStartupTime()
        {
            return 60;
        }
    }
}
