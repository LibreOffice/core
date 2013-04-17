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

import com.sun.star.uno.Any;
import com.sun.star.awt.XDialog;
import com.sun.star.awt.XCallback;
import com.sun.star.awt.XMessageBox;
import com.sun.star.awt.XRequestCallback;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

public class MainThreadDialogExecutor implements XCallback
{
    private WikiDialog m_aWikiDialog;
    private XDialog    m_xDialog;
    private XMessageBox m_xMessageBox;
    private boolean    m_bResult = false;
    private boolean    m_bCalled = false;
    private boolean    m_bClose = false;

    static public boolean Show( XComponentContext xContext, WikiDialog aWikiDialog )
    {
        MainThreadDialogExecutor aExecutor = new MainThreadDialogExecutor( aWikiDialog );
        return GetCallback( xContext, aExecutor );
    }

    static public boolean Execute( XComponentContext xContext, XDialog xDialog )
    {
        MainThreadDialogExecutor aExecutor = new MainThreadDialogExecutor( xDialog );
        return GetCallback( xContext, aExecutor );
    }

    static public boolean Execute( XComponentContext xContext, XMessageBox xMessageBox )
    {
        MainThreadDialogExecutor aExecutor = new MainThreadDialogExecutor( xMessageBox );
        return GetCallback( xContext, aExecutor );
    }

    static public boolean Close( XComponentContext xContext, XDialog xDialog )
    {
        MainThreadDialogExecutor aExecutor = new MainThreadDialogExecutor( xDialog, true );
        return GetCallback( xContext, aExecutor );
    }

    static private boolean GetCallback( XComponentContext xContext, MainThreadDialogExecutor aExecutor )
    {
        try
        {
            if ( aExecutor != null )
            {
                String aThreadName = null;
                Thread aCurThread = Thread.currentThread();
                if ( aCurThread != null )
                    aThreadName = aCurThread.getName();

                if ( aThreadName != null && aThreadName.equals( "com.sun.star.thread.WikiEditorSendingThread" ) )
                {
                    // the main thread should be accessed asynchronously
                    XMultiComponentFactory xFactory = xContext.getServiceManager();
                    if ( xFactory == null )
                        throw new com.sun.star.uno.RuntimeException();

                    XRequestCallback xRequest = UnoRuntime.queryInterface(
                        XRequestCallback.class,
                        xFactory.createInstanceWithContext( "com.sun.star.awt.AsyncCallback", xContext ) );
                    if ( xRequest != null )
                    {
                        xRequest.addCallback( aExecutor, Any.VOID );
                        do
                        {
                            Thread.yield();
                        }
                        while( !aExecutor.m_bCalled );
                    }
                }
                else
                {
                    // handle it as a main thread
                    aExecutor.notify( Any.VOID );
                }
            }
        }
        catch( Exception e )
        {
            e.printStackTrace();
        }

        return aExecutor.GetResult();
    }

    private MainThreadDialogExecutor( WikiDialog aWikiDialog )
    {
        m_aWikiDialog = aWikiDialog;
    }

    private MainThreadDialogExecutor( XDialog xDialog )
    {
        m_xDialog = xDialog;
    }

    private MainThreadDialogExecutor( XDialog xDialog, boolean bClose )
    {
        m_xDialog = xDialog;
        m_bClose = true;
        m_bCalled = true; // no yielding, asynchronous closing
    }

    private MainThreadDialogExecutor( XMessageBox xMessageBox )
    {
        m_xMessageBox = xMessageBox;
    }

    private boolean GetResult()
    {
        return m_bResult;
    }

    public void notify( Object aData )
    {
        if ( m_aWikiDialog != null )
            m_bResult = m_aWikiDialog.show();
        else if ( m_xDialog != null )
        {
            if ( !m_bClose )
                m_bResult = ( m_xDialog.execute() == 1 );
            else
            {
                try
                {
                    m_xDialog.endExecute();
                }
                catch( Exception e )
                {
                    e.printStackTrace();
                }
                m_bResult = true;
            }
        }
        else if ( m_xMessageBox != null )
        {
            int nRes = m_xMessageBox.execute();
            m_bResult = ( nRes == com.sun.star.awt.MessageBoxCommand.OK
                          || nRes == com.sun.star.awt.MessageBoxCommand.YES );
        }

        m_bCalled = true;
    }
};

