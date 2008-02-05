/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MainThreadDialogExecutor.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mav $ $Date: 2008-02-05 16:35:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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

                    XRequestCallback xRequest = (XRequestCallback)UnoRuntime.queryInterface(
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
            m_bResult = ( m_xMessageBox.execute() == 1 );

        m_bCalled = true;
    }
};

