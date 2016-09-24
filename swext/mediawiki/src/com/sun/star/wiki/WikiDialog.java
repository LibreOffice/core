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

package com.sun.star.wiki;

import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlContainer;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XDialog;
import com.sun.star.awt.XDialogEventHandler;
import com.sun.star.awt.XDialogProvider2;
import com.sun.star.awt.XAnimation;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.awt.XTopWindow;
import com.sun.star.awt.XTopWindowListener;
import com.sun.star.awt.XWindow;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XMultiServiceFactory;

public class WikiDialog implements XDialogEventHandler, XTopWindowListener
{
    protected XComponentContext m_xContext;
    private XControlContainer m_xControlContainer;
    protected XDialog m_xDialog;
    private String[] m_aMethods;
    protected boolean m_bAction = false;
    protected Settings m_aSettings;
    protected Thread m_aThread;
    protected boolean m_bThreadFinished = false;


    /** Creates a new instance of WikiDialog */
    public WikiDialog(XComponentContext c, String DialogURL)
    {
        this.m_xContext = c;
        XMultiComponentFactory xMCF = m_xContext.getServiceManager();
        m_aSettings = Settings.getSettings(m_xContext);
        try
        {
            Object obj;
            obj = xMCF.createInstanceWithContext("com.sun.star.awt.DialogProvider2", m_xContext );
            XDialogProvider2 xDialogProvider = UnoRuntime.queryInterface( XDialogProvider2.class, obj );

            m_xDialog = xDialogProvider.createDialogWithHandler( DialogURL, this );
            m_xControlContainer = UnoRuntime.queryInterface( XControlContainer.class, m_xDialog );
            XTopWindow xTopWindow = UnoRuntime.queryInterface( XTopWindow.class, m_xDialog );
            if ( xTopWindow != null )
                xTopWindow.addTopWindowListener( this );
        }
        catch (com.sun.star.uno.Exception ex)
        {
            ex.printStackTrace();
        }
    }

    public synchronized void ThreadStop( boolean bSelf )
    {
        if ( bSelf || m_aThread != null && !m_bThreadFinished )
        {
            try
            {
                Helper.AllowConnection( bSelf );
            }
            catch( Exception ex )
            {
                ex.printStackTrace();
            }
        }

        m_aThread = null;
        m_bThreadFinished = true;
    }

    protected void setMethods (String [] Methods)
    {
        this.m_aMethods = Methods;
    }


    public boolean show( )
    {
        m_bThreadFinished = false;

        if( m_xDialog != null ) m_xDialog.execute();
        return m_bAction;
    }


    public String[] getSupportedMethodNames()
    {
        return m_aMethods;
    }


    public boolean callHandlerMethod( XDialog xDialog, Object EventObject, String MethodName )
    {
        return true;
    }

    public void SetTitle( String sTitle )
        throws Exception
    {
        SetTitle( m_xDialog, sTitle );
    }

    private static void SetTitle( XDialog xDialog, String sTitle )
        throws Exception
    {
        if ( xDialog == null || sTitle == null ) {
            return;
        }
        XControl xDialogControl = UnoRuntime.queryInterface( XControl.class, xDialog );
        if ( xDialogControl != null )
        {
            XPropertySet xPropSet = UnoRuntime.queryInterface( XPropertySet.class, xDialogControl.getModel() );
            if ( xPropSet != null )
                xPropSet.setPropertyValue( "Title", sTitle );
        }
    }

    protected XPropertySet GetPropSet(String sControl)
    {
        return GetPropSet( m_xControlContainer, sControl );
    }

    private static XPropertySet GetPropSet( XControlContainer xControlContainer, String sControl )
    {
        XPropertySet xPS = null;

        if ( xControlContainer != null && sControl != null )
        {
            XControl xControl = xControlContainer.getControl(sControl);
            xPS = UnoRuntime.queryInterface(XPropertySet.class, xControl.getModel() );
        }

        if ( xPS == null )
            throw new com.sun.star.uno.RuntimeException();

        return xPS;
    }



    protected void InsertThrobber( int X, int Y, int Width, int Height )
    {
        try
        {
            XControl xDialogControl = UnoRuntime.queryInterface( XControl.class, m_xDialog );
            XControlModel xDialogModel = null;
            if ( xDialogControl != null )
                xDialogModel = xDialogControl.getModel();

            XMultiServiceFactory xDialogFactory = UnoRuntime.queryInterface( XMultiServiceFactory.class, xDialogModel );
            if ( xDialogFactory != null )
            {
                XControlModel xThrobberModel = UnoRuntime.queryInterface( XControlModel.class, xDialogFactory.createInstance( "com.sun.star.awt.SpinningProgressControlModel" ) );
                XPropertySet xThrobberProps = UnoRuntime.queryInterface( XPropertySet.class, xThrobberModel );
                if ( xThrobberProps != null )
                {
                    xThrobberProps.setPropertyValue( "Name", "WikiThrobber" );
                    xThrobberProps.setPropertyValue( "PositionX", Integer.valueOf( X ) );
                    xThrobberProps.setPropertyValue( "PositionY", Integer.valueOf( Y ) );
                    xThrobberProps.setPropertyValue( "Height", Integer.valueOf( Width ) );
                    xThrobberProps.setPropertyValue( "Width", Integer.valueOf( Height ) );

                    XNameContainer xDialogContainer = UnoRuntime.queryInterface( XNameContainer.class, xDialogModel );
                    xDialogContainer.insertByName( "WikiThrobber", xThrobberModel );
                }
            }
        }
        catch( Exception e )
        {
            e.printStackTrace();
        }

        SetThrobberVisible( false );
    }

    public void SetThrobberActive( boolean bActive )
    {
        if ( m_xControlContainer != null )
        {
            try
            {
                XAnimation xThrobber = UnoRuntime.queryInterface( XAnimation.class, m_xControlContainer.getControl( "WikiThrobber" ) );
                if ( xThrobber != null )
                {
                    if ( bActive )
                        xThrobber.startAnimation();
                    else
                        xThrobber.stopAnimation();
                }
            }
            catch( Exception e )
            {
                e.printStackTrace();
            }
        }
    }

    public void SetThrobberVisible( boolean bVisible )
    {
        if ( m_xControlContainer != null )
        {
            try
            {
                XWindow xWindow = UnoRuntime.queryInterface( XWindow.class, m_xControlContainer.getControl( "WikiThrobber" ) );
                if ( xWindow != null )
                    xWindow.setVisible( bVisible );
            }
            catch ( Exception e )
            {
                e.printStackTrace();
            }
        }
    }

    public void SetFocusTo( String aControl )
    {
        if ( m_xControlContainer != null )
        {
            try
            {
                XWindow xWindow = UnoRuntime.queryInterface( XWindow.class, m_xControlContainer.getControl( aControl ) );
                if ( xWindow != null )
                    xWindow.setFocus();
            }
            catch ( Exception e )
            {
                e.printStackTrace();
            }
        }
    }

    public void DisposeDialog()
    {
        Helper.Dispose( m_xDialog );
    }

    public void windowOpened( EventObject e )
    {}

    public void windowClosing( EventObject e )
    {}

    public void windowClosed( EventObject e )
    {}

    public void windowMinimized( EventObject e )
    {}

    public void windowNormalized( EventObject e )
    {}

    public void windowActivated( EventObject e )
    {}

    public void windowDeactivated( EventObject e )
    {}

    public void disposing( EventObject e )
    {}
}

