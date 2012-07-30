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

import com.sun.star.uno.UnoRuntime;

// @requirement FUNC.PERF.LRN/0.6
// @requirement FUNC.PERF.LOC/0.6
// @requirement FUNC.PERF.FIX/0.6
/** This is the basic JavaBean for all OOo application modules.

    @requirement FUNC.RES.OTH/0.2
        No other resources are needed yet.

    @since OOo 2.0.0
 */
public class OOoBean

    // @requirement FUNC.BEAN.VIEW/0.4
    extends java.awt.Container

    implements
        // @requirement FUNC.PER/0.2
        java.io.Externalizable
{
    // timeout values (milli secs)
    int nOOoStartTimeOut = 60000;
    int nOOoCallTimeOut =   3000;
    int nOOoCheckCycle =    1000;

    // This member contains the connection to an OOo instance if established.
    private transient OfficeConnection      iConnection;
    private transient EventListener         xConnectionListener;

    // @requirement FUNC.BEAN.VIEW/0.4
    // @requirement FUNC.BEAN.EDIT/0.4
    // This member contains the OOo window
    // if a connection is established.
    // It is a child of the OOoBean canvas.
    private OfficeWindow xFrameWindow;

    // application environment
    private transient com.sun.star.lang.XMultiServiceFactory xServiceFactory;
    private transient com.sun.star.frame.XDesktop xDesktop;

    // document and frame
    private transient Frame aFrame;
    private transient Controller aController;
    private transient OfficeDocument aDocument;

    // slot command execution environment
    private transient com.sun.star.frame.XDispatchProvider xDispatcher;
    private transient com.sun.star.util.XURLTransformer xURLTransformer;

    // properties
    private boolean bIgnoreVisibility = false; // to show even if already visible
    private boolean bMenuBarVisible = true;
    private boolean bStandardBarVisible = true;
    private boolean bToolBarVisible = true;
    private boolean bStatusBarVisible = true;


    // debugging method
    private void dbgPrint( String aMessage )
    {
        // System.err.println( "OOoBean: " + aMessage );
    }

    // @requirement FUNC.PER/0.2
    /** @internal
     *  @deprecated
     */
    public void writeExternal( java.io.ObjectOutput aObjOut )
    {
        // TBD
    }

    // @requirement FUNC.PER/0.2
    /** @internal
     *  @deprecated
     */
    public void readExternal( java.io.ObjectInput aObjIn )
    {
        // TBD
    }

    /** Generic constructor of the OOoBean.

        Neither a connection is established nor any document loaded.
     */
    public OOoBean()
    {}

       // @requirement FUNC.CON.MULT/0.3
    /** Constructor for an OOoBean which uses a specific office connection.

        The connection must be established but no document is loaded.

        @throws NoConnectionException
            if the connection is not established.

        @deprecated Clients could use the getOOoConnection to obtain an OfficeConnection
        and use it as argument in a constructor for another OOoBean instance. Calling
        the dispose method of the OfficeConnection or the OOoBean's stopOOoConnection
        method would make all instances of OOoBean stop working.
     */
    public OOoBean( OfficeConnection iConnection )
        throws NoConnectionException
    {
        try { setOOoConnection( iConnection ); }
        catch ( HasConnectionException aExc )
        { /* impossible here */ }
    }

    /** Sets the timeout for methods which launch OOo in milli seconds.

        This method does not need a connection to an OOo instance.
     */
    public void setOOoStartTimeOut( int nMilliSecs )
    {
        nOOoStartTimeOut = nMilliSecs;
    }

    /** Sets the timeout for normal OOO methods calls in milli seconds.

        This method does not need a connection to an OOo instance.
     */
    public void setOOoCallTimeOut( int nMilliSecs )
    {
        nOOoCallTimeOut = nMilliSecs;
    }

    /** Sets the period length in milli seconds to check the OOo connection.

        This method does not need a connection to an OOo instance.
     */
    public void setOOoCheckCycle( int nMilliSecs )
    {
        nOOoCheckCycle = nMilliSecs;
    }

    /** Sets the a connection to an OOo instance.

        @internal
     */
    private synchronized void setOOoConnection( OfficeConnection iNewConnection )
        throws  HasConnectionException, NoConnectionException
    {
        // the connection cannot be exchanged
        if ( iConnection != null )
            throw new HasConnectionException();

        // is there a real connection, not just the proxy?
        com.sun.star.uno.XComponentContext xComponentContext = null;
        try { xComponentContext = iNewConnection.getComponentContext(); }
        catch ( java.lang.Throwable aExc )
        { throw new NoConnectionException(); }
        if ( xComponentContext == null )
            throw new NoConnectionException();

        // set the connection
        iConnection = iNewConnection;

        // get notified when connection dies
        if ( xConnectionListener != null )
            xConnectionListener.end();
        xConnectionListener = this.new EventListener("setOOoConnection");
    }

    // @requirement FUNC.CON.STRT/0.4
    /** Starts a connection to an OOo instance which is lauched if not running.

        @throws HasConnectionException
            if a connection was already established.

        @throws NoConnectionException
            if the specified connection cannot be established
     */
    public void startOOoConnection( String aConnectionURL )
        throws  java.net.MalformedURLException,
            HasConnectionException,
            NoConnectionException
    {
        // create a new connection from the given connection URL
        LocalOfficeConnection aConnection = new LocalOfficeConnection();
        aConnection.setUnoUrl( aConnectionURL );
        setOOoConnection( aConnection );
    }

    // @requirement FUNC.CON.CHK/0.7
    /** Returns true if this OOoBean is connected to an OOo instance,
        false otherwise.

        @deprecated This method is not useful in a multithreaded environment. Then
        all threads accessing the instance would have to be synchronized in order to
        make is method work. It is better
        to call OOoBean's methods and be prepared to catch a NoConnectionException.
     */
    public boolean isOOoConnected()
    {
        return iConnection != null;
    }

    // @requirement FUNC.CON.STOP/0.4
    /** Disconnects from the connected OOo instance.

        If there was no connection yet or anymore, this method can be called
        anyway.

        When the OOoBean is displayed in an applet by a web browser, then this
        method must be called from within java.applet.Applet.stop.
     */
    public synchronized void stopOOoConnection()
    {
        // clear OOo document, frame etc.
        clear();

        // cut the connection
        OfficeConnection iExConnection = iConnection;
        if ( iConnection != null )
        {
            if ( xConnectionListener != null )
            {
                xConnectionListener.end();
            }
            iConnection = null;
            iExConnection.dispose();
        }

    }

    // @requirement FUNC.CON.STOP/0.4 (via XComponent.dispose())
       // @requirement FUNC.CON.NTFY/0.4 (via XComponent.addEventListener())
    /** Returns the a connection to an OOo instance.

        If no connection exists, a default connection will be created. An OfficeConnection
        can be used to register listeners of type com.sun.star.lang.EventListener,
        which are notified when the connection to the
        office dies. One should not call the dispose method, because this may result
        in receiving com.sun.star.lang.DisposedExceptions when calling
        {@link #stopOOoConnection stopOOoConnection} or other API methods. If other instances share the
        same connection then they will stop function properly, because they loose their
        connection as well. The recommended way to end the connection is
        calling {@link #stopOOoConnection stopOOoConnection}.

        @throws NoConnectionException
            if no connection can be established

     */
    public synchronized OfficeConnection getOOoConnection()
        throws NoConnectionException
    {
        if ( iConnection == null )
        {
            try { setOOoConnection( new LocalOfficeConnection() ); }
            catch ( HasConnectionException aExc )
            { /* impossible here */ }
        }
        if ( iConnection.getComponentContext() == null )
            throw new NoConnectionException();
        return iConnection;
    }

    /** Returns the service factory used by this OOoBean instance.

        @throws NoConnectionException
            if no connection is established and no default connection can be established.
     */
    public synchronized com.sun.star.lang.XMultiServiceFactory getMultiServiceFactory()
        throws NoConnectionException
    {
        if ( xServiceFactory == null )
        {
            // avoid concurrent access from multiple threads
            final OfficeConnection iConn = getOOoConnection();

            Thread aConnectorThread = new Thread() {
                public void run()
                {
                    com.sun.star.lang.XMultiComponentFactory aFactory =
                        iConn.getComponentContext().getServiceManager();
                    xServiceFactory = (com.sun.star.lang.XMultiServiceFactory)
                        UnoRuntime.queryInterface(
                            com.sun.star.lang.XMultiServiceFactory.class, aFactory );
                }
            };
            aConnectorThread.start();
            try { aConnectorThread.join(nOOoStartTimeOut); }
            catch ( java.lang.InterruptedException aExc )
            { throw new NoConnectionException(); }
            if ( xServiceFactory == null )
                throw new NoConnectionException();
        }

        return xServiceFactory;
    }

    /** Returns the XDesktop interface of the OOo instance used by this OOoBean.

        @throws NoConnectionException
            if no connection is established and no default connection can be established.
     */
    public synchronized com.sun.star.frame.XDesktop getOOoDesktop()
        throws NoConnectionException
    {
        if ( xDesktop == null )
        {
            try
            {
                Object aObject = getMultiServiceFactory().createInstance( "com.sun.star.frame.Desktop");
                xDesktop = (com.sun.star.frame.XDesktop) UnoRuntime.queryInterface(
                        com.sun.star.frame.XDesktop.class, aObject );
            }
            catch ( com.sun.star.uno.Exception aExc )
            {} // TBD: what if no connection exists?
        }

        return xDesktop;
    }

    /** Resets this bean to an empty document.

       If a document is loaded and the content modified,
       the changes are dismissed.  Otherwise nothing happens.

       This method is intended to be overridden in derived classes.
       This implementation simply calls clear.

       @param bClearStateToo
           Not only the document content but also the state of the bean,
        like visibility of child components is cleared.

        @deprecated There is currently no way to dismiss changes, except for loading
        of the unchanged initial document. Furthermore it is unclear how derived classes
        handle this and what exactly their state is (e.g. what members make up their state).
        Calling this method on a derived class requires knowledge about their implementation.
        Therefore a deriving class should declare their own clearDocument if needed. Clients
        should call the clearDocument of the deriving class or {@link #clear} which discards
        the currently displayed document.
     */
    public synchronized void clearDocument( boolean bClearStateToo )
        throws
            com.sun.star.util.CloseVetoException,
            NoConnectionException
    {
        // TBD
        clear();
    }

    /** Resets the OOoBean to an empty status.

        Any loaded document is unloaded, no matter whether it is modified or not.
        After calling this method, the OOoBean has no office document and no frame
        anymore.  The connection will stay, though.

        This method works with or without an established connection.
     */
    public synchronized void clear()
    {
        dbgPrint( "clear()" );

        try
        {
            CallWatchThread aCallWatchThread =
                new CallWatchThread( nOOoCallTimeOut, "clear" );
            //By closing the frame we avoid that dialogs are displayed, for example when
            //the document is modified.
            com.sun.star.util.XCloseable xCloseable = (com.sun.star.util.XCloseable)
                UnoRuntime.queryInterface( com.sun.star.util.XCloseable.class, aFrame );
            if ( xCloseable != null )
            {
                try
                {
                    xCloseable.close(true);
                }
                catch (com.sun.star.util.CloseVetoException exc)
                { // a print job may be running
                }
            }

            aDocument = null;
            xDispatcher = null;
            aFrame = null;

            // clear xFrameWindow
            if ( xFrameWindow != null )
            {
                try { releaseSystemWindow(); }
                catch ( NoConnectionException aExc )
                {} // ignore
                catch ( SystemWindowException aExc )
                {} // ignore
                remove( xFrameWindow.getAWTComponent() );
                xFrameWindow = null;
            }

            // clear xURTTransformer
            if ( xURLTransformer != null )
            {
                try
                {
                    com.sun.star.lang.XComponent xComp = (com.sun.star.lang.XComponent)
                        UnoRuntime.queryInterface(
                            com.sun.star.lang.XComponent.class, xURLTransformer );
                    if ( xComp != null )
                        xComp.dispose();
                }
                catch ( java.lang.Throwable aExc )
                {} // ignore
                xURLTransformer = null;
            }

            xDesktop = null;
            xServiceFactory = null;

            aCallWatchThread.cancel();
        }
        catch ( java.lang.InterruptedException aExc )
        { /* can be ignored */ }
    }

    // @requirement FUNC.PAR.LWP/0.4
    /** This method causes the office window to be displayed.

        If no document is loaded and the instance is added to a Java container that
        is showing, then this method needs not to be called. If later one of the methods
        {@link #loadFromURL loadFromURL}, {@link #loadFromStream loadFromStream1},
        or {@link #loadFromByteArray loadFromByteArray}
        is called, then the document is automatically displayed.

        Should one of the load methods have been called before the Java container
        was showing, then this method needs to be called after the container window
        was made visible (java.lang.Component.setVisible(true)).
        <p>
        Another scenario is that a OOoBean contains a document and is removed
        from a Java container and later added again. Then aquireSystemWindow needs
        to be called after the container window is displayed.

        @throws SystemWindowException
            if no system window can be aquired.

        @throws NoConnectionException
            if the connection is not established.
     */
    public synchronized void aquireSystemWindow()
        throws
            SystemWindowException,

            // @requirement FUNC.CON.LOST/0.2
            NoConnectionException
    {
        if ( iConnection == null )
            throw new NoConnectionException();
        if ( !isShowing() )
            throw new SystemWindowException();

        if ( xFrameWindow != null )
            xFrameWindow.getAWTComponent().setVisible(true);
        doLayout();
    }

    // @requirement FUNC.PAR.RWL/0.4
    // @estimation 16h
    /** This method must be called when the OOoBean before the
        sytem window may be released by it's parent AWT/Swing component.

        This is the case when java.awt.Component.isDisplayable() returns
        true.  This is definitely the case when the OOoBean is removed
        from it's parent container.

        @throws SystemWindowException
            if system window is not aquired.

        @throws NoConnectionException
            if the connection is not established.

        @deprecated When Component.removeNotify of the parent window of the actual
        office window is called, then the actions are performed for which this method
        needed to be called previously.
     */
    public synchronized void releaseSystemWindow()
        throws
            SystemWindowException,

            // @requirement FUNC.CON.LOST/0.2
            NoConnectionException
    {
        if ( iConnection == null )
            throw new NoConnectionException();

        try { xFrameWindow.getAWTComponent().setVisible(false); }
        catch ( com.sun.star.lang.DisposedException aExc )
        { throw new NoConnectionException(); }
    }

       // @requirement FUNC.BEAN.LOAD/0.4
       // @requirement FUNC.CON.AUTO/0.3
    /** Loads the bean from the given URL.

        If a document is already loaded and the content modified,
        the changes are dismissed.

        If no connection exists, a default connection is established.

        @throws IllegalArgumentException
            if either of the arguments is out of the specified range.

        @throws java.io.IOException
            if an IO error occurs reading the resource specified by the URL.

        @throws com.sun.star.lang.NoConnectionException
            if no connection can be established.

        @throws com.sun.star.util.CloseVetoException
            if the currently displayed document cannot be closed because it is
            still be used, for example it is printed.
     */
    public void loadFromURL(
            final String aURL,
            final com.sun.star.beans.PropertyValue aArguments[] )
        throws
            // @requirement FUNC.CON.LOST/0.2
            NoConnectionException,
            java.io.IOException,
            com.sun.star.lang.IllegalArgumentException,
            com.sun.star.util.CloseVetoException
    {
        dbgPrint( "loadFromURL()" );
         // try loading
        try
        {
            boolean bLoaded = false;
            while ( !bLoaded )
            {
                // watch loading in a thread with a timeout (if OOo hangs)
                CallWatchThread aCallWatchThread =
                    new CallWatchThread( nOOoStartTimeOut, "loadFromURL" );

                try
                {
                    // get window from OOo on demand
                    if ( xFrameWindow == null )
                    {
                        // Establish the connection by request of the ServiceFactory.
                        getMultiServiceFactory();

                        // remove existing child windows
                        removeAll();

                        // Create the OfficeWindow.
                        xFrameWindow = getOOoConnection().createOfficeWindow(OOoBean.this);
                        add( xFrameWindow.getAWTComponent() );
                    }

                    // create the document frame from UNO window.
                    if ( aFrame == null )
                    {
                        // create the frame
                        com.sun.star.awt.XWindow xWindow =
                            (com.sun.star.awt.XWindow) UnoRuntime.queryInterface(
                            com.sun.star.awt.XWindow.class, xFrameWindow.getUNOWindowPeer());
                        Object xFrame = xServiceFactory.createInstance( "com.sun.star.frame.Frame");
                        aFrame = new Frame( (com.sun.star.frame.XFrame)UnoRuntime.queryInterface(
                                com.sun.star.frame.XFrame.class, xFrame ) );
                        aFrame.initialize( xWindow );
                        aFrame.setName( aFrame.toString() );

                        // register the frame at the desktop
                        com.sun.star.frame.XFrames xFrames =
                                ( (com.sun.star.frame.XFramesSupplier)UnoRuntime.queryInterface(
                                com.sun.star.frame.XFramesSupplier.class, getOOoDesktop() ) ).getFrames();
                        xFrames.append( aFrame );
                    }

                    // Initializes the slot command execution environment.
                    xURLTransformer = (com.sun.star.util.XURLTransformer) UnoRuntime.queryInterface(
                        com.sun.star.util.XURLTransformer.class,
                        xServiceFactory.createInstance( "com.sun.star.util.URLTransformer") );

                                        try
                                        {
                                            xDispatcher = UnoRuntime.queryInterface(com.sun.star.frame.XDispatchProvider.class, aFrame);
                                        }
                                        catch (Exception e)
                                        {
                                            /*ignore!*/
                                        }

                    // get XComponentLoader from frame
                    com.sun.star.frame.XComponentLoader xLoader = (com.sun.star.frame.XComponentLoader)
                        UnoRuntime.queryInterface( com.sun.star.frame.XComponentLoader.class, aFrame );
                    if ( xLoader == null )
                    {
                        throw new java.lang.RuntimeException(
                            "com.sun.star.frame.Frame(" + aFrame +
                                ") without com.sun.star.frame.XComponentLoader" );
                    }

                    // Avoid Dialog 'Document changed' while reloading
                    if ( aDocument != null )
                    {
                        try {
                            aDocument.setModified(false);
                        } catch (com.sun.star.beans.PropertyVetoException ep) {
                            //it dosn't make sense to throw the exception here. The interface does not
                            //offer a way to add/remove respective listeners.
                        } catch (com.sun.star.lang.DisposedException ed) {
                            // can be disposed if user closed document via UI
                        }

                        com.sun.star.frame.XController xOldController = null;
                        if ( aFrame != null )
                            xOldController = aFrame.getController();

                        try
                        {

                            if ( aFrame != null && xOldController != null )
                                if (xOldController.suspend(true) == false)
                                    throw new com.sun.star.util.CloseVetoException(
                                            "Dokument is still being used and cannot be closed.", this);

                        }
                        catch (java.lang.IllegalStateException exp)
                        {}
                    }

                    // load the document.
                    com.sun.star.beans.PropertyValue aArgs[] =
                        addArgument( aArguments, new com.sun.star.beans.PropertyValue(
                            "MacroExecutionMode", -1,
                            new Short( com.sun.star.document.MacroExecMode.USE_CONFIG ),
                            com.sun.star.beans.PropertyState.DIRECT_VALUE ) );
                                    //String fn = aFRame.getName();

                    com.sun.star.lang.XComponent xComponent = xLoader.loadComponentFromURL(
                        aURL, /*aFrame.getName()*/"_self", 0, aArgs );

                    // nothing loaded?
                    if ( xComponent == null && aDocument != null )
                    {
                        // reactivate old document
                        if ( aFrame != null && aFrame.getController() != null )
                            aFrame.getController().suspend(false);
                        aDocument.setModified(true);

                        // throw exception
                        throw new java.io.IOException(
                            "Can not load a document: \"" + aURL + "\"");
                    }
                    // mDocumentURL = aURL; TBD: still needed?

                    // Get document's XModifiable interface if any.
                    aDocument = new OfficeDocument(
                        (com.sun.star.frame.XModel) UnoRuntime.queryInterface(
                        com.sun.star.frame.XModel.class, xComponent ) );
                    bLoaded = true;
                }
                catch ( NoConnectionException aExc )
                {
                    // stop, clear and retry
                    stopOOoConnection();
                }
                catch ( com.sun.star.lang.DisposedException aExc )
                {
                    // stop, clear and retry
                    stopOOoConnection();
                }
                catch ( com.sun.star.uno.Exception aExc )
                {
                    // TDB: handling failure in createInstance
                    aExc.printStackTrace();
                    throw new java.io.IOException();
                }

                aCallWatchThread.cancel();
                if ( xServiceFactory == null )
                    throw new NoConnectionException();
            }
            if ( iConnection == null )
            {
                throw new NoConnectionException();
            }

            applyToolVisibilities();
        }
        catch ( java.lang.InterruptedException aExc )
        {
            throw new NoConnectionException();
        }
    }

    /** Loads a document from a Java stream.

           See loadFromURL() for further information.
     */
    public void loadFromStream(
            final java.io.InputStream iInStream,
            final com.sun.star.beans.PropertyValue aArguments[] )
        throws
            // @requirement FUNC.CON.LOST/0.2
            NoConnectionException,
            java.io.IOException,
            com.sun.star.lang.IllegalArgumentException,
            com.sun.star.util.CloseVetoException
    {
        // wrap Java stream into UNO stream
                /*
        com.sun.star.io.XInputStream xStream =
                new com.sun.star.lib.uno.adapter.InputStreamToXInputStreamAdapter(
                    iInStream );
                 */
                 // copy stream....

                 int s = 4096;
                 int r=0 ,n = 0;
                 byte[] buffer = new byte[s];
                 byte[] newBuffer = null;
                 while ((r = iInStream.read(buffer, n, buffer.length-n))>0) {
                     n += r;
                     if (iInStream.available() > buffer.length - n) {
                         newBuffer = new byte[buffer.length*2];
                         System.arraycopy(buffer, 0, newBuffer, 0, n);
                         buffer = newBuffer;
                     }
                }
                if (buffer.length != n) {
                    newBuffer = new byte[n];
                    System.arraycopy(buffer, 0, newBuffer, 0, n);
                    buffer = newBuffer;
                }
                com.sun.star.io.XInputStream xStream =
                    new com.sun.star.lib.uno.adapter.ByteArrayToXInputStreamAdapter(buffer);

        // add stream to arguments
        com.sun.star.beans.PropertyValue[] aExtendedArguments =
            addArgument( aArguments, new com.sun.star.beans.PropertyValue(
                "InputStream", -1, xStream, com.sun.star.beans.PropertyState.DIRECT_VALUE ) );

        // call normal load method
        loadFromURL( "private:stream", aExtendedArguments );
    }

    /** Loads a document from a byte array.

           See loadFromURL() for further information.
     */
    public void loadFromByteArray(
            final byte aInBuffer[],
            final com.sun.star.beans.PropertyValue aArguments[] )
        throws
            // @requirement FUNC.CON.LOST/0.2
            NoConnectionException,
            java.io.IOException,
            com.sun.star.lang.IllegalArgumentException,
            com.sun.star.util.CloseVetoException
    {
        // wrap byte arrray into UNO stream
        com.sun.star.io.XInputStream xStream =
                new com.sun.star.lib.uno.adapter.ByteArrayToXInputStreamAdapter(
                    aInBuffer );

        // add stream to arguments
        com.sun.star.beans.PropertyValue[] aExtendedArguments =
            addArgument( aArguments, new com.sun.star.beans.PropertyValue(
                "InputStream", -1, xStream, com.sun.star.beans.PropertyState.DIRECT_VALUE ) );

        // call normal load method
        loadFromURL( "private:stream", aExtendedArguments );
    }

    /** Stores a document to the given URL.
        <p>
        Due due a bug (50651) calling this method may cause the office to crash,
        when at the same time the office writes a backup of the document. This bug
        also affects {@link #storeToByteArray storeToByteArray} and
        {@link #storeToStream storeToStream}. The workaround
        is to start the office with the option --norestore, which disables the automatic
        backup and recovery mechanism. OOoBean offers currently no supported way of providing
        startup options for OOo. But it is possible to set a Java property when starting
        Java, which is examined by OOoBean:
        <pre>
            java -Dcom.sun.star.officebean.Options=--norestore  ...
        </pre>
        It is planned to offer a way of specifying startup options in a future version.
        The property can be used until then. When using this property only one option
        can be provided.

        @throws IllegalArgumentException
            if either of the arguments is out of the specified range.

        @throws java.io.IOException
            if an IO error occurs reading the resource specified by the URL.

        @throws com.sun.star.lang.NoConnectionException
            if no connection is established.

        @throws NoDocumentException
            if no document is loaded
     */
    public void storeToURL(
            final String aURL,
            final com.sun.star.beans.PropertyValue aArguments[] )
        throws
            // @requirement FUNC.CON.LOST/0.2
            NoConnectionException,
            java.io.IOException,
            com.sun.star.lang.IllegalArgumentException,
            NoDocumentException
    {
        // no document available?
        if ( aDocument == null )
            throw new NoDocumentException();

        try
        {
            // start runtime timeout
            CallWatchThread aCallWatchThread =
                new CallWatchThread( nOOoCallTimeOut, "storeToURL" );

            // store the document
            try { aDocument.storeToURL( aURL, aArguments ); }
            catch ( com.sun.star.io.IOException aExc )
            { throw new java.io.IOException(); }

            // end runtime timeout
            aCallWatchThread.cancel();
        }
        catch ( java.lang.InterruptedException aExc )
        { throw new NoConnectionException(); }
    }

    /** Stores a document to a stream.

           See {@link #storeToURL storeToURL} for further information.
        @see #storeToURL storeToURL
     */
    public java.io.OutputStream storeToStream(
            java.io.OutputStream aOutStream,
            final com.sun.star.beans.PropertyValue aArguments[] )
        throws
            // @requirement FUNC.CON.LOST/0.2
            NoConnectionException,
            NoDocumentException,
            java.io.IOException,
            com.sun.star.lang.IllegalArgumentException

    {
        // wrap Java stream into UNO stream
        com.sun.star.lib.uno.adapter.OutputStreamToXOutputStreamAdapter aStream =
                new com.sun.star.lib.uno.adapter.OutputStreamToXOutputStreamAdapter(
                    aOutStream );

        // add stream to arguments
        com.sun.star.beans.PropertyValue[] aExtendedArguments =
            addArgument( aArguments, new com.sun.star.beans.PropertyValue(
                "OutputStream", -1, aStream, com.sun.star.beans.PropertyState.DIRECT_VALUE ) );

        // call normal store method
        storeToURL( "private:stream", aExtendedArguments );

        // get byte array from document stream
        try { aStream.closeOutput(); }
        catch ( com.sun.star.io.NotConnectedException aExc )
        { /* TDB */ }
        catch ( com.sun.star.io.BufferSizeExceededException aExc )
        { /* TDB */ }
        catch ( com.sun.star.io.IOException aExc )
        { throw new java.io.IOException(); }
        return aOutStream;
    }

    /** Stores a document to a byte array.

           See {@link #storeToURL storeToURL} for further information.
        @see #storeToURL storeToURL
     */
    public byte[] storeToByteArray(
            byte aOutBuffer[],
            final com.sun.star.beans.PropertyValue aArguments[] )
        throws
            // @requirement FUNC.CON.LOST/0.2
            NoConnectionException,
            NoDocumentException,
            java.io.IOException,
            com.sun.star.lang.IllegalArgumentException
    {
        // wrap byte arrray into UNO stream
        com.sun.star.lib.uno.adapter.XOutputStreamToByteArrayAdapter aStream =
                new com.sun.star.lib.uno.adapter.XOutputStreamToByteArrayAdapter(
                    aOutBuffer );

        // add stream to arguments
        com.sun.star.beans.PropertyValue[] aExtendedArguments =
            addArgument( aArguments, new com.sun.star.beans.PropertyValue(
                "OutputStream", -1, aStream, com.sun.star.beans.PropertyState.DIRECT_VALUE ) );

        // call normal store method
        storeToURL( "private:stream", aExtendedArguments );

        // get byte array from document stream
        try { aStream.closeOutput(); }
        catch ( com.sun.star.io.NotConnectedException aExc )
        { /* TDB */ }
        catch ( com.sun.star.io.BufferSizeExceededException aExc )
        { /* TDB */ }
        catch ( com.sun.star.io.IOException aExc )
        { throw new java.io.IOException(); }
        return aStream.getBuffer();
    }

    // @requirement FUNC.BEAN.PROG/0.5
    // @requirement API.SIM.SEAP/0.2
    /** returns the <type scope="com::sun::star::frame">Frame</a>
        of the bean.

        @returns
            a Java class which implements all interfaces which the service
        <type scope="com::sun::star::frame">Frame</a> implements.
        Thus, methods can be called directly without queryInterface.
        This feature might be implemented by UNO or explicitly coded.

        @throws NoConnectionException
            if the connection is not established.

        @throws NotDocumentException
            if no document is loaded an thus no frame is available.
     */
    public Frame getFrame()

        throws
            NoConnectionException // @requirement FUNC.CON.LOST/0.2
    {
        if ( iConnection == null )
            throw new NoConnectionException();
        return aFrame;
    }

       // @requirement FUNC.BEAN.PROG/0.5
       // @requirement API.SIM.SEAP/0.2
    /** returns the <type scope="com::sun::star::frame::Controller"> of the bean.

        @returns
            a Java class which implements all interfaces which the service
        <type scope="com::sun::star::frame">Controller</a> implements.
        Thus, methods can be called directly without queryInterface.
        This feature might be implemented by UNO or explicitly coded.

        @throws NoConnectionException
            if the connection is not established.
     */
    public Controller getController()

        // @requirement FUNC.CON.LOST/0.2
        throws NoConnectionException
    {
        if ( iConnection == null )
            throw new NoConnectionException();
        if ( aController == null )
            aController = new Controller( aFrame.getController() );
        return aController;
    }

    // @requirement FUNC.BEAN.PROG/0.5
    // @requirement FUNC.BEAN.STOR/0.4
    // @requirement FUNC.BEAN.PRNT/0.4
    // @requirement API.SIM.SEAP/0.2
       /** returns the <type scope="com::sun::star::document::OfficeDocument">
        of the bean.

        @returns
            a Java class which implements all interfaces which the service
        <type scope="com::sun::star::document">OfficeDocument</a>
        implements.
        Thus, methods can be called directly without queryInterface.
        This feature might be implemented by UNO or explicitly coded.

        @throws NoConnectionException
            if the connection is not established.
     */
    public OfficeDocument getDocument()

        // @requirement FUNC.CON.LOST/0.2
        throws NoConnectionException
    {
        if ( iConnection == null )
            throw new NoConnectionException();
        return aDocument;
    }

    /** Sets visibility of all tool bars known by this OOoBean version.

         Initially all tool bars are visible.  By hiding all tool bars
        utilizing this method, it is possible to turn just a subset of
        tool bars on afterwards, no matter whether all available tool
        bars are known or not.
        <p>
        If an older OOoBean instance is used with a newer OOo instance,
        some tool bars might not be affected by this method.
        <p>
        If no connection is established or no document is loaded,
        the setting is memorized until a document is loaded.  Same
        is valid when the connection dies within this function call.

        @deprecated Clients should use the service com.sun.star.frame.LayoutManager,
        which can be obtained from a frame, to control toolbars. For example:
        <pre>
com.sun.star.beans.XPropertySet xPropSet =
  (com.sun.star.beans.XPropertySet) UnoRuntime.queryInterface(
    com.sun.star.beans.XPropertySet.class, aFrame );
com.sun.star.frame.XLayoutManager xLayoutManager =
  (com.sun.star.frame.XLayoutManager) UnoRuntime.queryInterface(
    com.sun.star.frame.XLayoutManager.class,
    xPropSet.getPropertyValue( "LayoutManager" ) );
xLayoutManager.showElement("private:resource/menubar/menubar");
        </pre>
     */
    public void setAllBarsVisible( boolean bVisible )
    {
        bIgnoreVisibility = true;
        setMenuBarVisible( bVisible );
        setStandardBarVisible( bVisible );
        setToolBarVisible( bVisible );
        setStatusBarVisible( bVisible );
        bIgnoreVisibility = false;
    }

    //--------------------------------------------------------------------------
    /** Applies all tool visiblities to the real thing.

        @deprecated Clients should use the service com.sun.star.frame.LayoutManager,
        which can be obtained from a frame, to control toolbars. See also
        {@link #setAllBarsVisible setAllBarsVisible}.
     */
    protected void applyToolVisibilities()
        throws
            java.lang.InterruptedException
    {
        bIgnoreVisibility = true;
        setMenuBarVisible( bMenuBarVisible );
        setStandardBarVisible( bStandardBarVisible );
        setToolBarVisible( bToolBarVisible );
        setStatusBarVisible( bStatusBarVisible );
        bIgnoreVisibility = false;
    }

    /** Helper method to set tool bar visibilty.

         @param bnewValue
            If false, the tool bar is disabled,
            If true, the tool bar is visible.

        @deprecated Clients should use the service com.sun.star.frame.LayoutManager,
        which can be obtained from a frame, to control toolbars. See also
        {@link #setAllBarsVisible}.
     */
    protected boolean setToolVisible( String aProperty, String aResourceURL,
        boolean bOldValue, boolean bNewValue )

        throws
            java.lang.InterruptedException
    {
        // start runtime timeout
        CallWatchThread aCallWatchThread =
            new CallWatchThread( nOOoCallTimeOut, "setToolVisible" );

        // Does a frame exist?
        if ( aFrame != null )
        {
            if ( bIgnoreVisibility || bOldValue != bNewValue )
            {
                try
                {
                    com.sun.star.beans.XPropertySet xPropSet =
                            (com.sun.star.beans.XPropertySet) UnoRuntime.queryInterface(
                            com.sun.star.beans.XPropertySet.class, aFrame );
                    com.sun.star.frame.XLayoutManager xLayoutManager =
                            (com.sun.star.frame.XLayoutManager) UnoRuntime.queryInterface(
                            com.sun.star.frame.XLayoutManager.class,
                            xPropSet.getPropertyValue( "LayoutManager" ) );
                    if ( bNewValue )
                        xLayoutManager.showElement( aResourceURL );
                    else
                        xLayoutManager.hideElement( aResourceURL );
                }
                catch (  com.sun.star.beans.UnknownPropertyException aExc )
                {
                    throw new RuntimeException( "not layout manager found" );
                }
                catch (  com.sun.star.lang.WrappedTargetException aExc )
                {
                    throw new RuntimeException( "not layout manager found" );
                }

                // notify change
                firePropertyChange( aProperty, new Boolean(bOldValue), new Boolean(bNewValue) );
           }
        }

        // end runtime timeout
        aCallWatchThread.cancel();

        // the new value will be stored by caller
        return bNewValue;
    }

    /** Sets the visibility of the menu bar.

        Initially the menu bar is visible.
        <p>
        If not connected or no document loaded, the value is stored
        and automatically applied to the document after it is loaded.
        Same is valid when the connection dies within this function call.

         @param bVisible
            If false, the menu bar is disabled,
            If true, the menu bar is visible.

        @deprecated Clients should use the service com.sun.star.frame.LayoutManager,
        which can be obtained from a frame, to control toolbars. See also
        {@link #setAllBarsVisible}.
     */
    public void setMenuBarVisible(boolean bVisible)
    {
        try
        {
            bMenuBarVisible = setToolVisible( "MenuBarVisible",
                    "private:resource/menubar/menubar", bMenuBarVisible, bVisible );
        }
        catch ( java.lang.InterruptedException aExc )
        {
            bMenuBarVisible = bVisible;
        }
    }

      /** Returns the visibility of the menu bar.

           This method works independently from a connetion or loaded document.
        If no connection is established or no document is loaded,
        this method just returns a memorized status.

        @return
            True if the menu bar is visible,
            false if the menu bar is hidden.

        @deprecated Clients should use the service com.sun.star.frame.LayoutManager,
        which can be obtained from a frame, to control toolbars. See also
        {@link #setAllBarsVisible}.
     */
    public boolean isMenuBarVisible()
    {
        return bMenuBarVisible;
    }

    /** Sets the main function bar visibilty.

        Initially the standard bar is visible.

        If not connected or no document loaded, the value is stored
        and automatically applied to the document after it is loaded.
        Same is valid when the connection dies within this function call.

         @param bVisible
            If false, the main function bar is disabled,
            If true, the main function bar is visible.

        @deprecated Clients should use the service com.sun.star.frame.LayoutManager,
        which can be obtained from a frame, to control toolbars. See also
        {@link #setAllBarsVisible}.
     */
    public void setStandardBarVisible(boolean bVisible)
    {
        try
        {
            bStandardBarVisible = setToolVisible( "StandardBarVisible",
                    "private:resource/toolbar/standardbar", bStandardBarVisible, bVisible );
        }
        catch ( java.lang.InterruptedException aExc )
        {
            bMenuBarVisible = bVisible;
        }
    }

      /** Returns the visibility of the main function bar.

           This method works independently from a connetion or loaded document.
        If no connection is established or no document is loaded,
        this method just returns a memorized status.

        @return
            True if the main function bar is visible,
            false if the main function bar is hidden.

        @deprecated Clients should use the service com.sun.star.frame.LayoutManager,
        which can be obtained from a frame, to control toolbars. See also
        {@link #setAllBarsVisible}.
    */
    public boolean isStandardBarVisible()
    {
        return bStandardBarVisible;
    }

    /** Sets the tool function bar visibilty.

        Initially the tool bar is visible.

        If not connected or no document loaded, the value is stored
        and automatically applied to the document after it is loaded.
        Same is valid when the connection dies within this function call.

         @param bVisible
            If false, the tool function bar is disabled,
            If true, the tool function bar is visible.

        @deprecated Clients should use the service com.sun.star.frame.LayoutManager,
        which can be obtained from a frame, to control toolbars. See also
        {@link #setAllBarsVisible}.
     */
    public void setToolBarVisible(boolean bVisible)
    {
        try
        {
            bToolBarVisible = setToolVisible( "ToolBarVisible",
                    "private:resource/toolbar/toolbar", bToolBarVisible, bVisible );
        }
        catch ( java.lang.InterruptedException aExc )
        {
            bMenuBarVisible = bVisible;
        }
    }

      /** Returns the visibility of the tool function bar.

           This method works independently from a connetion or loaded document.
        If no connection is established or no document is loaded,
        this method just returns a memorized status.

        @return
            True if the tool function bar is visible,
            false if the tool function bar is hidden.

        @deprecated Clients should use the service com.sun.star.frame.LayoutManager,
        which can be obtained from a frame, to control toolbars. See also
        {@link #setAllBarsVisible}.
     */
    public boolean isToolBarVisible()
    {
        return bToolBarVisible;
    }

    /** Sets the status function bar visibilty.

        Initially the status bar is visible.

        If not connected or no document loaded, the value is stored
        and automatically applied to the document after it is loaded.
        Same is valid when the connection dies within this function call.

         @param bVisible
            If false, the status function bar is disabled,
            If true, the status function bar is visible.

        @deprecated Clients should use the service com.sun.star.frame.LayoutManager,
        which can be obtained from a frame, to control toolbars. See also
        {@link #setAllBarsVisible}.
     */
    public void setStatusBarVisible(boolean bVisible)
    {
        try
        {
            bStatusBarVisible = setToolVisible( "StatusBarVisible",
                    "private:resource/statusbar/statusbar", bStatusBarVisible, bVisible );
        }
        catch ( java.lang.InterruptedException aExc )
        {
            bMenuBarVisible = bVisible;
        }
    }

      /**   Returns the visibility of the status function bar.

           This method works independently from a connetion or loaded document.
        If no connection is established or no document is loaded,
        this method just returns a memorized status.

        @return
            True if the status function bar is visible,
            false if the status function bar is hidden.

        @deprecated Clients should use the service com.sun.star.frame.LayoutManager,
        which can be obtained from a frame, to control toolbars. See also
        {@link #setAllBarsVisible}.
     */
    public boolean isStatusBarVisible()
    {
        return bStatusBarVisible;
    }

    //===========================================================================
    // Helper Methods / Internal Methods
    //---------------------------------------------------------------------------

    // general instance intializer
    {
        setLayout(new java.awt.BorderLayout());
    }

    /**
        @deprecated
     */
    public void paint( java.awt.Graphics aGraphics )
    {
    }

    /** Adds a single argument to an array of arguments.

        If the argument by its name is already in aArguments
        it is exchanged and aArguments is returned.
        <p>
        If the argument by its name is not yet in aArguments,
        a new array is created, aArgument added and the new
        array returned.
    */
    protected com.sun.star.beans.PropertyValue[] addArgument(
                com.sun.star.beans.PropertyValue aArguments[],
                final com.sun.star.beans.PropertyValue aArgument )
    {
        // get number of current arguments
        int nNumArgs = 0;
        if ( aArguments != null )
            nNumArgs = aArguments.length;

        // is new argument already set?
        for ( int n = 0; n < nNumArgs; ++n )
        {
            if ( aArguments[n].Name == aArgument.Name )
            {
                // substitute this argument
                aArguments[n] = aArgument;

                // return current array
                return aArguments;
            }
        }

        // create extended arguments
        com.sun.star.beans.PropertyValue[] aExtendedArguments =
            new com.sun.star.beans.PropertyValue[ nNumArgs + 1 ];

        // copy current arguments
        for ( int n = 0; n < nNumArgs; ++n )
            aExtendedArguments[n] = aArguments[n];

        // add new argument
        aExtendedArguments[ nNumArgs ] = aArgument;

        // return new arguments
        return aExtendedArguments;
    }

    //===========================================================================
    // Helper Classes
    //---------------------------------------------------------------------------

    /** Helper class to listen on the connection to learn when it dies.

        @internal
     */
    private class EventListener
        extends Thread
        implements
            com.sun.star.lang.XEventListener,
            com.sun.star.frame.XTerminateListener
    {
        String aTag;

        EventListener( String aTag )
            throws NoConnectionException
        {
            // init members
            this.aTag = aTag;

            // listen on a dying connection
            iConnection.addEventListener( this );

            // listen on a terminating OOo
            getOOoDesktop().addTerminateListener( this );

            // start this thread as a daemon
            setDaemon( true );
            start();
        }

        public void end()
        {
            // do not listen on a dying connection anymore
            try {
                iConnection.removeEventListener( this );
            }
            catch ( Throwable aExc ) {};

            // do not listen on a terminating OOo anymore
            try {
                getOOoDesktop().removeTerminateListener( this );
            }
            catch ( Throwable aExc ) {};

            // stop thread
            this.interrupt();
        }

        /// gets called when the connection dies
        public void disposing( /*IN*/ com.sun.star.lang.EventObject Source )
        {
            // empty the OOoBean and cut the connection
            stopOOoConnection();
        }

        /// gets called when the user wants to terminate OOo
           public void queryTermination( /*IN*/ com.sun.star.lang.EventObject Event )
            throws com.sun.star.frame.TerminationVetoException
        {
            // disallow termination of OOo while a OOoBean exists
            throw new com.sun.star.frame.TerminationVetoException();
        }

        /// gets called when OOo terminates
        public void notifyTermination( /*IN*/ com.sun.star.lang.EventObject Event )
        {
            // empty the OOoBean and cut the connection
            stopOOoConnection();
        }

        /// watching the connection
        public void run()
        {
            dbgPrint( "EventListener(" + aTag + ").run()" );

            // remote call might hang => watch try
            CallWatchThread aCallWatchThread =
                new CallWatchThread( nOOoCallTimeOut, "EventListener(" + aTag + ")" );

            // continue to trying to connect the OOo instance
            long n = 0;
            while ( isInterrupted() == false
                    && iConnection != null
                    && iConnection.getComponentContext() != null )
            {
                dbgPrint( "EventListener(" + aTag + ").running() #" + ++n );

                // still alive?
                com.sun.star.lang.XMultiComponentFactory xServiceManager = null;
                try
                {
                    // an arbitrary (but cheap) call into OOo
                    xServiceManager = iConnection.getComponentContext().getServiceManager();

                    // call successfully performed, restart watch for next loop
                    try
                    {
                        aCallWatchThread.restart();
                    }
                    catch ( java.lang.InterruptedException aExc )
                    {
                        // ignore late interrupt
                    }
                }
                catch ( java.lang.RuntimeException aExc )
                {
                    // hung
                    OfficeConnection iDeadConn = iConnection;
                    iConnection = null;
                    iDeadConn.dispose();
                }

                // sleep
                try {
                        sleep(nOOoCheckCycle);
                }
                catch ( java.lang.InterruptedException aExc )
                {
                    dbgPrint("EventListener(" + aTag + ") interupted.");
                    //thread can be ended by EvendListener.end();
                    break;
                }
            }
        }
    }

}



