/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: LocalOfficeWindow.java,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package com.sun.star.beans;

import java.awt.Component;

import com.sun.star.lang.EventObject;
import com.sun.star.lang.SystemDependent;
import com.sun.star.lang.XEventListener;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.awt.XVclWindowPeer;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.WindowDescriptor;
import com.sun.star.awt.WindowAttribute;
import com.sun.star.awt.WindowClass;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

/**
 * This class represents a local office window.
 * @deprecated
 */
public class LocalOfficeWindow
    extends java.awt.Canvas
    implements OfficeWindow, XEventListener
{
    private transient OfficeConnection  mConnection;
    private transient XWindowPeer       mParentProxy;
    private transient XWindowPeer       mWindow;
    private boolean             bPeer = false;

    /**
     * Construnctor.
     *
     * @param connection The office connection object the window
     *  belongs to.
     */
    /* package */ LocalOfficeWindow(OfficeConnection connection)
    {
        mConnection = connection;
        mConnection.addEventListener((XEventListener)this);
    }

    /**
     * Retrives an AWT component object associated with the OfficeWindow.
     *
     * @return The AWT component object associated with the OfficeWindow.
     */
    public Component getAWTComponent()
    {
        return this;
    }

    /**
     * Retrives an UNO XWindowPeer object associated with the OfficeWindow.
     *
     * @return The UNO XWindowPeer object associated with the OfficeWindow.
     */
    public XWindowPeer getUNOWindowPeer()
    {
        if (mWindow == null)
            createUNOWindowPeer();
        return mWindow;
    }

    /**
     * Receives a notification about the connection has been closed.
     * This method has to set the connection to <code>null</code>.
     *
     * @source The event object.
     */
    public void disposing(EventObject source)
    {
        // the window will be disposed by the framework
        mWindow = null;
        mConnection = null;
    }

    /**
    * Returns an AWT toolkit.
        */
       private XToolkit queryAWTToolkit()
               throws com.sun.star.uno.Exception
       {
            // Create a UNO toolkit.
            XMultiComponentFactory  compfactory;
            XComponentContext xContext = mConnection.getComponentContext();
            if ( xContext != null )
            {
                compfactory     = mConnection.getComponentContext().getServiceManager();
                XMultiServiceFactory    factory;
                factory = (XMultiServiceFactory)UnoRuntime.queryInterface(
                        XMultiServiceFactory.class, compfactory);
                Object          object  = factory.createInstance( "com.sun.star.awt.Toolkit");
                return (XToolkit)UnoRuntime.queryInterface(XToolkit.class, object);
            }
            else
                return null;
       }

           /// called when system parent is available, reparents the bean window
    private void aquireSystemWindow()
    {
        if ( !bPeer )
        {
            // set real parent
            XVclWindowPeer xVclWindowPeer = (XVclWindowPeer)UnoRuntime.queryInterface(
                               XVclWindowPeer.class, mWindow);
            xVclWindowPeer.setProperty( "PluginParent", new Long(getNativeWindow()) );
            bPeer = true;

                   // show document window
            XWindow aWindow = (XWindow)UnoRuntime.queryInterface(XWindow.class, mWindow);
            aWindow.setVisible( true );
        }
    }

           /// called when system parent is about to die, reparents the bean window
    private void releaseSystemWindow()
    {
        if ( bPeer )
        {
                   // hide document window
            XWindow aWindow = (XWindow)UnoRuntime.queryInterface(XWindow.class, mWindow);
            aWindow.setVisible( false );

            // set null parent
            XVclWindowPeer xVclWindowPeer = (XVclWindowPeer)UnoRuntime.queryInterface(
                               XVclWindowPeer.class, mWindow);
            xVclWindowPeer.setProperty( "PluginParent", new Long(0) );
            bPeer = false;
        }
    }

       /// callback handler to get to know when we become visible
        //@deprecated
       class ComponentEventHandler
               extends java.awt.event.ComponentAdapter
       {
        public void componentHidden( java.awt.event.ComponentEvent e)
        {
            // only when we become invisible, we might lose our system window
            CallWatchThread aCallWatchThread = new CallWatchThread( 500 );
            setVisible(false);
            try { aCallWatchThread.cancel(); }
            catch ( java.lang.InterruptedException aExc )
            {} // ignore
        }

        public void componentShown( java.awt.event.ComponentEvent e)
        {
            // only when we become visible, we get a system window
             aquireSystemWindow();
        }
    }

    /// Overriding java.awt.Component.setVisible() due to Java bug (no showing event).
    public void setVisible( boolean b )
    {
        super.setVisible(b);

        // Java-Bug: componentShown() is never called :-(
        // is still at least in Java 1.4.1_02
        if ( b )
            aquireSystemWindow();
        else
            releaseSystemWindow();
    }

       /** Factory method for a UNO AWT toolkit window as a child of this Java window.
    *
    */
       private XWindowPeer createUNOWindowPeer()
       {
        try
        {
            // get this windows native window type
                        int type = getNativeWindowSystemType();

                        // Java AWT windows only have a system window when showing.
                        XWindowPeer parentPeer;
                        if ( isShowing() )
                        {
                // create direct parent relationship
                //setVisible( true );
                                parentPeer = new JavaWindowPeerFake( getNativeWindow(), type);
                bPeer = true;
                        }
                        else
            {
                // no parent yet
                parentPeer = null;
                bPeer = false;
            }

            // create native window (mWindow)
            Rectangle aRect = new Rectangle( 0, 0, 20, 20 );
            WindowDescriptor desc = new WindowDescriptor();
            desc.Type = WindowClass.TOP;
            desc.Parent = parentPeer;
            desc.Bounds = aRect;
            desc.WindowServiceName = "workwindow";
            desc.WindowAttributes = (type == SystemDependent.SYSTEM_WIN32)
                ? WindowAttribute.SHOW : 0;
            mWindow = queryAWTToolkit().createWindow(desc);

            // to get notified when we become visible
            addComponentListener( new ComponentEventHandler() );

            // set initial visibility
                        XWindow aWindow = (XWindow)UnoRuntime.queryInterface(XWindow.class, mWindow);
            aWindow.setVisible( bPeer );
        }
        catch (com.sun.star.uno.Exception exp) {
        }

        return mWindow;
    }

    /**
     * Retrives a platform dependant system window identifier.
     *
     * @return The system window identifier.
     */
    private native long getNativeWindow();

    /**
     * Retrives a platform dependant system window type.
     *
     * @return The system window type.
     */
    private native int getNativeWindowSystemType();

        //---------------------------------------------------------------------------
        /** Helper class to watch calls into OOo with a timeout.
         * @deprecated
         */
        class CallWatchThread extends Thread
        {
                Thread aWatchedThread;
                long nTimeout;

                CallWatchThread( long nTimeout )
                {
                        this.aWatchedThread = Thread.currentThread();
                        this.nTimeout = nTimeout;
                        start();
                }

                void cancel()
                        throws java.lang.InterruptedException
                {
                        Thread aThread = aWatchedThread;
                        aWatchedThread = null;
                        stop();

                        if ( aThread.interrupted() )
                                throw new InterruptedException();
                }

                public void run()
                {
                        while ( aWatchedThread != null )
                        {
                                try { sleep( nTimeout ); }
                                catch ( java.lang.InterruptedException aExc )
                                {}

                                //synchronized
                                {
                                        if ( aWatchedThread != null )
                                        {
                                                aWatchedThread.interrupt();
                                        }
                                }
                        }
                }
        };

}
