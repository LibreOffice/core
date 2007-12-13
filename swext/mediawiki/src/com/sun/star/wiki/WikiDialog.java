/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WikiDialog.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mav $ $Date: 2007-12-13 10:34:07 $
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

import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlContainer;
import com.sun.star.awt.XDialog;
import com.sun.star.awt.XDialogEventHandler;
import com.sun.star.awt.XDialogProvider2;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

public class WikiDialog implements XDialogEventHandler{

    XComponentContext m_xContext;
    XControlContainer m_xControlContainer;
    XDialog m_xDialog;
    String[] m_aMethods;
    boolean m_bAction = false;
    Settings m_aSettings;


    /** Creates a new instance of WikiDialog */
    public WikiDialog(XComponentContext c, String DialogURL) {
        this.m_xContext = c;
        XMultiComponentFactory xMCF = m_xContext.getServiceManager();
        m_aSettings = Settings.getSettings(m_xContext);
        try {
            Object obj;
            obj = xMCF.createInstanceWithContext("com.sun.star.awt.DialogProvider2", m_xContext );
            XDialogProvider2 xDialogProvider = (XDialogProvider2) UnoRuntime.queryInterface( XDialogProvider2.class, obj );

            m_xDialog = xDialogProvider.createDialogWithHandler( DialogURL, this );
            m_xControlContainer = (XControlContainer)UnoRuntime.queryInterface(XControlContainer.class, m_xDialog );
        } catch (com.sun.star.uno.Exception ex) {
            ex.printStackTrace();
        }
    }


    protected void setMethods (String [] Methods) {
        this.m_aMethods = Methods;
    }


    public boolean show( ) {
        if( m_xDialog != null ) m_xDialog.execute();
        return m_bAction;
    }


    public String[] getSupportedMethodNames() {
        return m_aMethods;
    }


    public boolean callHandlerMethod( XDialog xDialog, Object EventObject, String MethodName ) {
        return true;
    }


    protected XPropertySet getPropSet(String sControl) {
        XControl xControl = m_xControlContainer.getControl(sControl);
        XPropertySet xPS = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xControl.getModel() );
        if ( xPS == null )
            throw new com.sun.star.uno.RuntimeException();

        return xPS;
    }

    public static XDialog CreateSimpleDialog( XComponentContext xContext, String sURL )
    {
        XDialog xResult = null;

        if ( xContext != null && sURL != null && sURL.length() > 0 )
        {
            try
            {
                Object oDialogProvider = xContext.getServiceManager().createInstanceWithContext("com.sun.star.awt.DialogProvider2", xContext );
                XDialogProvider2 xDialogProvider = (XDialogProvider2) UnoRuntime.queryInterface( XDialogProvider2.class, oDialogProvider );

                if ( xDialogProvider != null )
                    xResult = xDialogProvider.createDialog( sURL );
            }
            catch (com.sun.star.uno.Exception ex)
            {
                ex.printStackTrace();
            }
        }

        return xResult;
    }

}
