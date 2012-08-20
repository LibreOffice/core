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

package com.sun.star.comp.beans;

import java.awt.Container;
import java.io.File;
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;

import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XEventListener;
import com.sun.star.connection.XConnection;
import com.sun.star.connection.XConnector;
import com.sun.star.bridge.XBridge;
import com.sun.star.bridge.XBridgeFactory;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Exception;
import com.sun.star.lib.uno.helper.UnoUrl;
import com.sun.star.lib.util.NativeLibraryLoader;

/**
 * This class reprecents a connection to the local office application.
 *
 * @since OOo 2.0.0
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
    private XBridge mBridge;

    private String              mURL;
    private String              mConnType;
    private String              mPipe;
    private String              mPort;
    private String              mProtocol;
    private String              mInitialObject;

    private List<XEventListener> mComponents = new ArrayList<XEventListener>();

    private static long m_nBridgeCounter = 0;
    //-------------------------------------------------------------------------
    static
    {
        // preload shared libraries whichs import lips are linked to officebean
        if ( System.getProperty( "os.name" ).startsWith( "Windows" ) )
        {
            try
            {
                NativeLibraryLoader.loadLibrary(LocalOfficeConnection.class.getClassLoader(), "msvcr70");
            }
            catch (Throwable e)
            {
                // loading twice would fail
                System.err.println( "cannot find msvcr70" );
            }

            try
            {
                NativeLibraryLoader.loadLibrary(LocalOfficeConnection.class.getClassLoader(), "msvcr71");
            }
            catch (Throwable e)
            {
                // loading twice would fail
                System.err.println( "cannot find msvcr71" );
            }

            try
            {
                NativeLibraryLoader.loadLibrary(LocalOfficeConnection.class.getClassLoader(), "uwinapi");
            }
            catch (Throwable e)
            {
                // loading twice would fail
                System.err.println( "cannot find uwinapi" );
            }

            try
            {
                NativeLibraryLoader.loadLibrary(LocalOfficeConnection.class.getClassLoader(), "jawt");
            }
            catch (Throwable e)
            {
                // loading twice would fail
                System.err.println( "cannot find jawt" );
            }
        }

        // load shared library for JNI code
        NativeLibraryLoader.loadLibrary( LocalOfficeConnection.class.getClassLoader(), "officebean" );
    }

    //-------------------------------------------------------------------------

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
    }

        /**
         * protected Constructor
         * Initialise a LocalOfficeConnection with an already running office.
         * This C'Tor is only used in complex tests at the moment.
         * @param xContext
         */
        protected LocalOfficeConnection(com.sun.star.uno.XComponentContext xContext)
        {
            this.mContext = xContext;
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
                mConnType = aURL.getConnection();
                mPipe = (String) aURL.getConnectionParameters().get( "pipe" );
                mPort = (String) aURL.getConnectionParameters().get( "port" );
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
    synchronized public XComponentContext getComponentContext()
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
        while (itr.hasNext() == true) {
            // ignore runtime exceptions in dispose
            try { itr.next().disposing(null); }
            catch ( RuntimeException aExc ) {}
        }
        mComponents.clear();

        //Terminate the bridge. It turned out that this is necessary for the bean
        //to work properly when displayed in an applet within Internet Explorer.
        //When navigating off the page which is showing  the applet and then going
        //back to it, then the Java remote bridge is damaged. That is the Java threads
        //do not work properly anymore. Therefore when Applet.stop is called the connection
        //to the office including the bridge needs to be terminated.
        if (mBridge != null)
        {
            XComponent comp = (XComponent)UnoRuntime.queryInterface(
                    XComponent.class, mBridge);
            if (comp != null)
               comp.dispose();
            else
                System.err.println("LocalOfficeConnection: could not dispose bridge!");

            mBridge = null;
        }

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

            // try to connect to soffice
            Object aInitialObject = null;
            try
            {
                aInitialObject = resolve(xLocalContext, mURL);
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
                        Thread.currentThread().sleep( 500 );
                        aInitialObject = resolve(xLocalContext, mURL);
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
                XPropertySet xPropertySet = (XPropertySet)
                    UnoRuntime.queryInterface( XPropertySet.class, aInitialObject);
                        Object xContext = xPropertySet.getPropertyValue("DefaultContext");
                        XComponentContext xComponentContext = (XComponentContext) UnoRuntime.queryInterface(
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


    //The function is copied and adapted from the UrlResolver.resolve.
    //We cannot use the URLResolver because we need access to the bridge which has
    //to be disposed when Applet.stop is called.
    private Object resolve(XComponentContext xLocalContext, String dcp)
        throws com.sun.star.connection.NoConnectException,
            com.sun.star.connection.ConnectionSetupException,
            com.sun.star.lang.IllegalArgumentException
    {
         String conDcp = null;
        String protDcp = null;
        String rootOid = null;

        if(dcp.indexOf(';') == -1) {// use old style
            conDcp = dcp;
            protDcp = "iiop";
            rootOid = "classic_uno";
        }
        else { // new style
            int index = dcp.indexOf(':');
            String url = dcp.substring(0, index).trim();
            dcp = dcp.substring(index + 1).trim();

            index = dcp.indexOf(';');
            conDcp = dcp.substring(0, index).trim();
            dcp = dcp.substring(index + 1).trim();

            index = dcp.indexOf(';');
            protDcp = dcp.substring(0, index).trim();
            dcp = dcp.substring(index + 1).trim();

            rootOid = dcp.trim().trim();
        }

        Object rootObject = null;
        XBridgeFactory xBridgeFactory= null;

        XMultiComponentFactory xLocalServiceManager = xLocalContext.getServiceManager();
        try {
            xBridgeFactory = (XBridgeFactory)UnoRuntime.queryInterface(
                    XBridgeFactory.class,
                    xLocalServiceManager.createInstanceWithContext(
                        "com.sun.star.bridge.BridgeFactory", xLocalContext));
        } catch (com.sun.star.uno.Exception e) {
            throw new com.sun.star.uno.RuntimeException(e.getMessage());
        }
        synchronized(this) {
            if(mBridge == null) {
                Object connector= null;
                try {
                    connector = xLocalServiceManager.createInstanceWithContext(
                            "com.sun.star.connection.Connector", xLocalContext);
                } catch (com.sun.star.uno.Exception e) {
                    throw new com.sun.star.uno.RuntimeException(e.getMessage());
                }
                XConnector connector_xConnector = (XConnector)UnoRuntime.queryInterface(XConnector.class, connector);
                // connect to the server
                XConnection xConnection = connector_xConnector.connect(conDcp);
                // create the bridge name. This should not be necessary if we pass an
                //empty string as bridge name into createBridge. Then we should always get
                //a new bridge. This does not work because of (i51323). Therefore we
                //create unique bridge names for the current process.
                String sBridgeName = "OOoBean_private_bridge_" + String.valueOf(m_nBridgeCounter++);
                try {
                    mBridge = xBridgeFactory.createBridge(sBridgeName, protDcp, xConnection, null);
                } catch (com.sun.star.bridge.BridgeExistsException e) {
                    throw new com.sun.star.uno.RuntimeException(e.getMessage());
                }
            }
            rootObject = mBridge.getInstance(rootOid);
            return rootObject;
        }
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
        int idx = url.indexOf(";urp;StarOffice.ServiceManager");
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
                    if (name.equals("path")) {
                        if (path == null)
                            path    = buffer.toString();
                        else
                            state   = -3;   // error: more then one 'path'
                    } else if (name.equals("pipe")) {
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
        return replaceAll( replaceAll( java.net.URLEncoder.encode(aPipeName), "+", "%20" ), "%", "_" );
    }

    /**
     * @para This is an implementation of the native office service.
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
            int nSizeCmdArray = 4;
            String sOption = null;
            //examine if user specified command-line options in system properties.
            //We may offer later a more sophisticated way of providing options if
            //the need arises. Currently this is intended to ease the pain during
            //development  with pre-release builds of LibO where one wants to start
            //LibO with the --norestore options. The value of the property is simple
            //passed on to the Runtime.exec call.
            try {
                sOption = System.getProperty("com.sun.star.officebean.Options");
                if (sOption != null)
                    nSizeCmdArray ++;
            } catch (java.lang.SecurityException e)
            {
                e.printStackTrace();
            }
           // create call with arguments
            String[] cmdArray = new String[nSizeCmdArray];

            // read UNO_PATH environment variable to get path to soffice binary
            String unoPath = System.getenv("UNO_PATH");
            if (unoPath == null)
                throw new java.io.IOException( "UNO_PATH environment variable is not set (required system path to the office program directory)" );

//          cmdArray[0] = (new File(getProgramPath(), OFFICE_APP_NAME)).getPath();
            cmdArray[0] = (new File(unoPath, OFFICE_APP_NAME)).getPath();
            cmdArray[1] = "--nologo";
            cmdArray[2] = "--nodefault";
            if ( mConnType.equals( "pipe" ) )
                cmdArray[3] = "--accept=pipe,name=" + getIdentifier() + ";" +
                          mProtocol + ";" + mInitialObject;
            else if ( mConnType.equals( "socket" ) )
                cmdArray[3] = "--accept=socket,port=" + mPort + ";urp";
            else
                throw new java.io.IOException( "not connection specified" );

            if (sOption != null)
                cmdArray[4] = sOption;

            // start process
            mProcess = Runtime.getRuntime().exec(cmdArray);
            if ( mProcess == null )
                throw new RuntimeException( "cannot start soffice: " + cmdArray );
            new StreamProcessor(mProcess.getInputStream(), System.out);
            new StreamProcessor(mProcess.getErrorStream(), System.err);
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



    class StreamProcessor extends Thread
    {
        java.io.InputStream m_in;
        java.io.PrintStream m_print;

        public StreamProcessor(final java.io.InputStream in, final java.io.PrintStream out)
        {
            m_in = in;
            m_print = out;
            start();
        }

        public void run() {
            java.io.BufferedReader r = new java.io.BufferedReader(
                new java.io.InputStreamReader(m_in) );
            try {
                for ( ; ; ) {
                    String s = r.readLine();
                    if ( s == null ) {
                        break;
                    }
                    m_print.println(s);
                }
            } catch ( java.io.IOException e ) {
                e.printStackTrace( System.err );
            }
        }
    }

}
