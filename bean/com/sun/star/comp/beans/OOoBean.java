/*************************************************************************
 *
 *  $RCSfile: OOoBean.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: mi $ $Date: 2004-10-14 10:37:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., September, 2004
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2004 by Sun Microsystems, Inc.
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
 *  Copyright: 2004 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

    // used slot ids
    public final static short   SID_TOGGLEMENUBAR   = 6661;
    public final static short   SID_TOGGLEOBJECTBAR = 5905;
    public final static short   SID_TOGGLETOOLBAR   = 5909;
    public final static short   SID_TOGGLEMAINBAR   = 5910;
    public final static short   SID_TOGGLESTATUSBAR = 5920;

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

    //-------------------------------------------------------------------------
    // debugging method
    private void dbgPrint( String aMessage )
    {
        // System.err.println( "OOoBean: " + aMessage );
    }

    //-------------------------------------------------------------------------
    // @requirement FUNC.PER/0.2
    /** @internal
     */
    public void writeExternal( java.io.ObjectOutput aObjOut )
    {
        // TBD
    }

    //-------------------------------------------------------------------------
    // @requirement FUNC.PER/0.2
    /** @internal
     */
    public void readExternal( java.io.ObjectInput aObjIn )
    {
        // TBD
    }

    //-------------------------------------------------------------------------
    /** Generic constructor of the OOoBean.

        Neither a connection is established nor any document loaded.
     */
    public OOoBean()
    {}

    //-------------------------------------------------------------------------
        // @requirement FUNC.CON.MULT/0.3
    /** Constructor for an OOoBean which uses a specific office connection.

        The connection must be established but no document is loaded.

        @throws NoConnectionException
            if the connection is not established.
     */
    public OOoBean( OfficeConnection iConnection )
        throws NoConnectionException
    {
        try { setOOoConnection( iConnection ); }
        catch ( HasConnectionException aExc )
        { /* impossible here */ }
    }

    //-------------------------------------------------------------------------
    /// Sets the timeout for methods which launch OOo in milli seconds.
    public void setOOoStartTimeOut( int nMilliSecs )
    {
        nOOoStartTimeOut = nMilliSecs;
    }

    //-------------------------------------------------------------------------
    /// Sets the timeout for normal OOO methods calls in milli seconds.
    public void setOOoCallTimeOut( int nMilliSecs )
    {
        nOOoCallTimeOut = nMilliSecs;
    }

    //-------------------------------------------------------------------------
    /// Sets the period length in milli seconds to check the OOo connection.
    public void setOOoCheckCycle( int nMilliSecs )
    {
        nOOoCheckCycle = nMilliSecs;
    }

    //-------------------------------------------------------------------------
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

    //-------------------------------------------------------------------------
    // @requirement FUNC.CON.STRT/0.4
    /** Starts a connection to an OOo instance which is lauched if not running.
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

    //-------------------------------------------------------------------------
    // @requirement FUNC.CON.CHK/0.7
    /** Returns true if this OOoBean is connected to an OOo instance,
        false otherwise.
     */
    public boolean isOOoConnected()
    {
        // TBD: could there be an existing, but dead connection?
        return iConnection != null;
    }

    //-------------------------------------------------------------------------
    // @requirement FUNC.CON.STOP/0.4
    /** Disconnects from the connected OOo instance.
     */
    public synchronized void stopOOoConnection()
    {
        // clear OOo document, frame etc.
        try { clear(); }
        catch (  java.lang.InterruptedException aExc )
        {} // ignore exceptions from an old, dead connection

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

    //-------------------------------------------------------------------------
    // @requirement FUNC.CON.STOP/0.4 (via XComponent.dispose())
        // @requirement FUNC.CON.NTFY/0.4 (via XComponent.addEventListener())
    /** Returns the a connection to an OOo instance.

        If no connection exists, a default connection will be created.
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

    //-------------------------------------------------------------------------

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

    //-------------------------------------------------------------------------
    /// TBD
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

    //-------------------------------------------------------------------------
    // @requirement TBD
    // @estimation 4h
    /** Resets this bean to an empty document.

       If a document is loaded and the content modified,
       the changes are dismissed.  Otherwise nothing happens.

       This method works with and without a connection.

       @param bClearStateToo
           Not only the document content but also the state of the bean,
        like visibility of child components is cleared.
     */
    public synchronized void clearDocument( boolean bClearStateToo )
        throws com.sun.star.util.CloseVetoException
    {
        // TBD
        com.sun.star.util.XCloseable xCloseable = (com.sun.star.util.XCloseable)
            UnoRuntime.queryInterface( com.sun.star.util.XCloseable.class, aFrame );
        if ( xCloseable != null )
            xCloseable.close(true);
        else
            throw new RuntimeException( "frame without XCloseable" );
    }

    //-------------------------------------------------------------------------
    /** Resets the OOoBean to an empty status.

        Any loaded document is unloaded, no matter whether it is modified or not.
        After calling this method, the OOoBean has no office document and no frame
        anymore.  The connection will stay, though.
     */
    public synchronized void clear()
        throws java.lang.InterruptedException
    {
        dbgPrint( "clear()" );

        CallWatchThread aCallWatchThread =
            new CallWatchThread( nOOoCallTimeOut, "clear" );
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

    //-------------------------------------------------------------------------
    // @requirement FUNC.PAR.LWP/0.4
    // @estimation 2h
    /** This method must be called when the OOoBean can aquire a
        sytem window from it's parent AWT/Swing component.

        This is the case when java.awt.Component.isDisplayable() returns
        true.  This again is usually true when the parent is visible and
        the parent container chain is created up to a top level system
        window.

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

    //-------------------------------------------------------------------------
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

    //-------------------------------------------------------------------------
        // @requirement FUNC.BEAN.LOAD/0.4
        // @requirement FUNC.CON.AUTO/0.3
    // @estimation 2h
    /** Loads the bean from the given URL.

        If a document is already loaded and the content modified,
        the changes are dismissed.

        If no connection exists, a default connection is established.

        @throws java.io.IOException
            if an IO error occurs reading the ressource specified by the URL.

        @throws IllegalArgumentException
            if either of the arguments is out of the specified range.

        @throws NoConnectionException
            if no connection can be established.
     */
    public void loadFromURL(
            final String aURL,
            final com.sun.star.beans.PropertyValue aArguments[] )
        throws
            java.io.IOException,
            java.lang.InterruptedException,
            com.sun.star.lang.IllegalArgumentException,

            // @requirement FUNC.CON.LOST/0.2
            NoConnectionException
    {
        dbgPrint( "loadFromURL()" );

        // try loading
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
                xDispatcher = (com.sun.star.frame.XDispatchProvider)UnoRuntime.queryInterface(
                    com.sun.star.frame.XDispatchProvider.class, aFrame );

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
                    com.sun.star.frame.XController xOldController = null;
                    if ( aFrame != null && aFrame.getController() != null )
                        xOldController = aFrame.getController();
                    try
                    {
                        if ( aFrame != null && xOldController != null )
                            xOldController.suspend(true);
                        if ( aDocument != null )
                            try {
                                // can be disposed if user closed document via UI
                                aDocument.setModified(false);
                            }
                            catch (  com.sun.star.lang.DisposedException aExc ) {}
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
                    aURL, /*aFrame.getName()*/"_self", com.sun.star.frame.FrameSearchFlag.ALL, aArgs );

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

    //---------------------------------------------------------------------------
    /** Loads a document from a Java stream.

           See loadFromURL() for further information.
     */
    public void loadFromStream(
            final java.io.InputStream iInStream,
            final com.sun.star.beans.PropertyValue aArguments[] )
        throws
            java.io.IOException,
            java.lang.InterruptedException,
            com.sun.star.lang.IllegalArgumentException,

            // @requirement FUNC.CON.LOST/0.2
            NoConnectionException

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
        loadFromURL( "private:stream/", aExtendedArguments );
    }

    //---------------------------------------------------------------------------
    /** Loads a document from a byte array.

           See loadFromURL() for further information.
     */
    public void loadFromByteArray(
            final byte aInBuffer[],
            final com.sun.star.beans.PropertyValue aArguments[] )
        throws
            java.io.IOException,
            java.lang.InterruptedException,
            com.sun.star.lang.IllegalArgumentException,

            // @requirement FUNC.CON.LOST/0.2
            NoConnectionException

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
        loadFromURL( "private:stream/", aExtendedArguments );
    }

    //---------------------------------------------------------------------------
    /** Stores a document to a URL.
     *
     *  TBD
     */
    public void storeToURL(
            final String aURL,
            final com.sun.star.beans.PropertyValue aArguments[] )
        throws
            java.io.IOException,
            java.lang.InterruptedException,
            com.sun.star.lang.IllegalArgumentException,

            // @requirement FUNC.CON.LOST/0.2
            NoConnectionException
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

    //---------------------------------------------------------------------------
    /** Stores a document to a stream.

           See storeToURL() for further information.
     */
    public java.io.OutputStream storeToStream(
            java.io.OutputStream aOutStream,
            final com.sun.star.beans.PropertyValue aArguments[] )
        throws
            java.io.IOException,
            java.lang.InterruptedException,
            com.sun.star.lang.IllegalArgumentException,

            // @requirement FUNC.CON.LOST/0.2
            NoConnectionException

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
        storeToURL( "private:stream/", aExtendedArguments );

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

    //---------------------------------------------------------------------------
    /** Stores a document to a byte array.

           See storeToURL() for further information.
     */
    public byte[] storeToByteArray(
            byte aOutBuffer[],
            final com.sun.star.beans.PropertyValue aArguments[] )
        throws
            java.io.IOException,
            java.lang.InterruptedException,
            com.sun.star.lang.IllegalArgumentException,

            // @requirement FUNC.CON.LOST/0.2
            NoConnectionException

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
        storeToURL( "private:stream/", aExtendedArguments );

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

    //-------------------------------------------------------------------------

    // @requirement FUNC.BEAN.PROG/0.5
    // @requirement API.SIM.SEAP/0.2
    /** returns the <type scope="com::sun::star::frame">Frame</a>
        of the bean.

        @returns
            a Java class which implements all interfaces which the service
        <type scope="com::sun::star::frame">Frame</a> implements.
        Thus, methods can be called directly without queryInterface.
        This feature might be implemented by UNO or explicitely coded.

        @throws NoConnectionException
            if the connection is not established.
     */
    public Frame getFrame()

        // @requirement FUNC.CON.LOST/0.2
        throws NoConnectionException
    {
        if ( iConnection == null )
            throw new NoConnectionException();
        return aFrame;
    }

    //-------------------------------------------------------------------------
        // @requirement FUNC.BEAN.PROG/0.5
        // @requirement API.SIM.SEAP/0.2
    // @estimation 1h
        /** returns the <type scope="com::sun::star::frame::Controller"> of the bean.

        @returns
            a Java class which implements all interfaces which the service
        <type scope="com::sun::star::frame">Controller</a> implements.
        Thus, methods can be called directly without queryInterface.
        This feature might be implemented by UNO or explicitely coded.

        @throws NoConnectionException
            if the connection is not established.

        @throws NotDocumentLoadedException
            if no document is loaded an thus no controller is available.
     */
    public Controller getController()

        // @requirement FUNC.CON.LOST/0.2
        throws NoConnectionException,

        // @requirement API.NODC
        NoDocumentException
    {
        if ( iConnection == null )
            throw new NoConnectionException();
        if ( aController == null )
            aController = new Controller( aFrame.getController() );
        return aController;
    }

    //-------------------------------------------------------------------------
    // @requirement FUNC.BEAN.PROG/0.5
    // @requirement FUNC.BEAN.STOR/0.4
    // @requirement FUNC.BEAN.PRNT/0.4
    // @requirement API.SIM.SEAP/0.2
    // @estimation 1h
        /** returns the <type scope="com::sun::star::document::OfficeDocument">
        of the bean.

        @returns
            a Java class which implements all interfaces which the service
        <type scope="com::sun::star::document">OfficeDocument</a>
        implements.
        Thus, methods can be called directly without queryInterface.
        This feature might be implemented by UNO or explicitely coded.

        @throws NoConnectionException
            if the connection is not established.

        @throws NotDocumentLoadedException
            if no document is loaded.
     */
    public OfficeDocument getDocument()

        // @requirement FUNC.CON.LOST/0.2
        throws NoConnectionException,

        // @requirement API.NODC
        NoDocumentException // TBD
    {
        if ( iConnection == null )
            throw new NoConnectionException();
        return aDocument;
    }

    //--------------------------------------------------------------------------
    /*  Sets visibility of all tool bars known by this OOoBean version.

         Initially all tool bars are visible.  By hiding all tool bars
        utilizing this method, it is possible to turn just a subset of
        tool bars on afterwards, no matter whether all available tool
        bars are known or not.

        If an older OOoBean instance is used with a newer OOo instance,
        some tool bars might not be affected by this method.
     */
    public void setAllBarsVisible( boolean bVisible )
        throws
            java.lang.InterruptedException
    {
        bIgnoreVisibility = true;
        setMenuBarVisible( bVisible );
        setStandardBarVisible( bVisible );
        setToolBarVisible( bVisible );
        setStatusBarVisible( bVisible );
        bIgnoreVisibility = false;
    }

    //--------------------------------------------------------------------------
    /*  Applies all tool visiblities to the real thing.
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

    //--------------------------------------------------------------------------
    /*  Helper method to set tool bar visibilty.

         @param bnewValue
            If false, the tool bar is disabled,
            If true, the tool bar is visible.
     */
    protected boolean setToolVisible( String aProperty, String aResourceURL,
                short nSlotID, String aSlotArgName, boolean bOldValue, boolean bNewValue )
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
                    drafts.com.sun.star.frame.XLayoutManager xLayoutManager =
                            (drafts.com.sun.star.frame.XLayoutManager) UnoRuntime.queryInterface(
                            drafts.com.sun.star.frame.XLayoutManager.class,
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

    //--------------------------------------------------------------------------
    /*  Sets the visibility of the menu bar.

        Initially the menu bar is visible.

        If not connected or no document loaded, the value is stored
        and automatically applied to the document after it is loaded.

         @param bVisible
            If false, the menu bar is disabled,
            If true, the menu bar is visible.
     */
    public void setMenuBarVisible(boolean bVisible)
        throws
            java.lang.InterruptedException
    {
        bMenuBarVisible = setToolVisible( "MenuBarVisible", "private:resource/menubar/menubar",
                SID_TOGGLEMENUBAR, "MenuBarVisible", bMenuBarVisible, bVisible );
    }

    //--------------------------------------------------------------------------
      /* Returns the visibility of the menu bar.

           This method works independently from a connetion or loaded document.

        @return
            True if the menu bar is visible,
            false if the menu bar is hidden.
     */
    public boolean isMenuBarVisible()
    {
        return bMenuBarVisible;
    }

    //--------------------------------------------------------------------------
    /*  Sets the main function bar visibilty.

        Initially the menu bar is visible.

        If not connected or no document loaded, the value is stored
        and automatically applied to the document after it is loaded.

         @param bVisible
            If false, the main function bar is disabled,
            If true, the main function bar is visible.
     */
    public void setStandardBarVisible(boolean bVisible)
        throws
            java.lang.InterruptedException
    {
        bStandardBarVisible = setToolVisible( "StandardBarVisible", "private:resource/toolbar/standardbar",
                SID_TOGGLEMAINBAR, "FunctionBarVisible", bStandardBarVisible, bVisible );
    }

    //--------------------------------------------------------------------------
      /* Returns the visibility of the main function bar.

           This method works independently from a connetion or loaded document.

        @return
            True if the main function bar is visible,
            false if the main function bar is hidden.
     */
    public boolean isStandardBarVisible()
    {
        return bStandardBarVisible;
    }

    //--------------------------------------------------------------------------
    /*  Sets the tool function bar visibilty.

        Initially the menu bar is visible.

        If not connected or no document loaded, the value is stored
        and automatically applied to the document after it is loaded.

         @param bVisible
            If false, the tool function bar is disabled,
            If true, the tool function bar is visible.
     */
    public void setToolBarVisible(boolean bVisible)
        throws
            java.lang.InterruptedException
    {
        bToolBarVisible = setToolVisible( "ToolBarVisible", "private:resource/toolbar/toolbar",
                SID_TOGGLETOOLBAR, "ToolBarVisible", bToolBarVisible, bVisible );
    }

    //--------------------------------------------------------------------------
      /* Returns the visibility of the tool function bar.

           This method works independently from a connetion or loaded document.

        @return
            True if the tool function bar is visible,
            false if the tool function bar is hidden.
     */
    public boolean isToolBarVisible()
    {
        return bToolBarVisible;
    }

    //--------------------------------------------------------------------------
    /*  Sets the status function bar visibilty.

        Initially the menu bar is visible.

        If not connected or no document loaded, the value is stored
        and automatically applied to the document after it is loaded.

         @param bVisible
            If false, the status function bar is disabled,
            If true, the status function bar is visible.
     */
    public void setStatusBarVisible(boolean bVisible)
        throws
            java.lang.InterruptedException
    {
        bStatusBarVisible = setToolVisible( "StatusBarVisible", "private:resource/statusbar/statusbar",
                SID_TOGGLESTATUSBAR, "StatusBarVisible", bStatusBarVisible, bVisible );
    }

    //--------------------------------------------------------------------------
      /*    Returns the visibility of the status function bar.

           This method works independently from a connetion or loaded document.

        @return
            True if the status function bar is visible,
            false if the status function bar is hidden.
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

    //---------------------------------------------------------------------------
    /// TBD
    public void paint( java.awt.Graphics aGraphics )
    {
        // TBD
        aGraphics.drawString( "empty", 10, 10 );
    }

    //---------------------------------------------------------------------------
    /** Adds a single argument to an array of arguments.

        If the argument by its name is already in aArguments
        it is exchanged and aArguments is returned.

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
            super.stop();
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
            while ( iConnection != null && iConnection.getComponentContext() != null )
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
                    // empty the OOoBean and cut the connection
                    stopOOoConnection();
                }
            }
        }
    }

    //---------------------------------------------------------------------------
    /** Helper class to watch calls into OOo with a timeout.
     */
    protected class CallWatchThread extends Thread
    {
        Thread aWatchedThread;
        String aTag;
        boolean bAlive;
        long nTimeout;

        CallWatchThread( long nTimeout, String aTag )
        {
            this.aWatchedThread = Thread.currentThread();
            this.nTimeout = nTimeout;
            this.aTag = aTag;
            setDaemon( true );
            dbgPrint( "CallWatchThread(" + this + ").start(" + aTag + ")" );
            start();
        }

        void cancel()
            throws java.lang.InterruptedException
        {
            dbgPrint( "CallWatchThread(" + this + ".cancel(" + aTag + ")" );
            if ( aWatchedThread != null && aWatchedThread != Thread.currentThread() )
                throw new RuntimeException( "wrong thread" );
            aWatchedThread = null;
            if ( interrupted() )
                throw new InterruptedException();
        }

        synchronized void restart()
            throws java.lang.InterruptedException
        {
            dbgPrint( "CallWatchThread(" + this + ".restart(" + aTag + ")" );
            if ( aWatchedThread != null && aWatchedThread != Thread.currentThread() )
                throw new RuntimeException( "wrong thread" );
            bAlive = true;
            if ( interrupted() )
                throw new InterruptedException();
            notify();
        }

        public void run()
        {
            dbgPrint( "CallWatchThread(" + this + ".run(" + aTag + ") ***** STARTED *****" );
            long n = 0;
            while ( aWatchedThread != null )
            {
                dbgPrint( "CallWatchThread(" + this + ").run(" + aTag + ") running #" + ++n );
                synchronized(this)
                {
                    bAlive = false;

                    // wait a while
                    try { wait( nTimeout ); }
                    catch ( java.lang.InterruptedException aExc )
                    {
                        bAlive = false;
                    }

                    // watched thread seems to be dead (not answering)?
                    if ( !bAlive && aWatchedThread != null )
                    {
                        dbgPrint( "CallWatchThread(" + this + ").run(" + aTag + ") interrupting" );
                        aWatchedThread.interrupt();
                        aWatchedThread = null;
                    }
                }
            }

            dbgPrint( "CallWatchThread(" + this + ").run(" + aTag + ") terminated" );
        }
    };

    //---------------------------------------------------------------------------
    /**
     * This class reprecents an office slot command.
     */
    public class OfficeCommand
    {
        private short mSlot;
        private java.util.List mProps = new java.util.ArrayList();

        /**
         * Constructor.
         *
         * @param slot The office slot identifier.
         * @param args The arguments of the office slot command.
         */
        public OfficeCommand(short slot, com.sun.star.beans.PropertyValue args[])
        {
            mSlot   = slot;
            for (int idx = 0; idx < args.length; idx += 1) {
                mProps.add(args[idx]);
            }
        }

        /**
         * Constructor.
         *
         * @param slot The office slot identifier.
         */
        public OfficeCommand(short slot)
        {
            mSlot   = slot;
        }

        /**
         * Appends an office slot command parameter.
         *
         * @param name The parameter name.
         * @param value The parameter value.
         */
        public synchronized void appendParameter(String name, Object value)
        {
            mProps.add(new com.sun.star.beans.PropertyValue(name, 0, value,
                com.sun.star.beans.PropertyState.DIRECT_VALUE));
        }

        /**
         * Executes the command on the specified office instance.
         *
         * @param office The office instance which is the target of the command.
         */
        public synchronized void execute()
            throws com.sun.star.comp.beans.NoConnectionException
        {
            // avoid conflicts with connect/disconnect...
            synchronized(OOoBean.this)
            {
                // ... do the job
                com.sun.star.util.URL[] aURL =
                    new com.sun.star.util.URL[1];
                aURL[0] = new com.sun.star.util.URL(
                    "slot:" + Short.toString(mSlot),
                    "",         // string Main
                    "",         // string Protocol
                    "",         // string User
                    "",         // string Password
                    "",         // string Server
                    (short)0,   // short Port
                    "",         // string Path
                    "",         // string Name
                    "",         // string Arguments
                    "");        // string Mark
                xURLTransformer.parseSmart(aURL, "slot");

                // workaround bug in dispatch
                aURL[0].Complete    = aURL[0].Main;
                com.sun.star.frame.XDispatch xDispatcher
                        = getFrame().queryDispatch( aURL[0], "", 0);
                xDispatcher.dispatch(aURL[0],
                        (com.sun.star.beans.PropertyValue[])mProps.toArray(
                        new com.sun.star.beans.PropertyValue[mProps.size()]));
            }

            // Good things come to those who wait...
            notifyAll();
        }
    }
};



