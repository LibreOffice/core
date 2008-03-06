/*************************************************************************
*
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ScriptContext.java,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 16:09:59 $
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
package com.sun.star.script.framework.provider;

import com.sun.star.document.XScriptInvocationContext;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XDesktop;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.lib.uno.helper.PropertySet;
import com.sun.star.uno.Type;

import com.sun.star.script.provider.XScriptContext;

import com.sun.star.script.framework.log.LogUtils;


/**
 *  Description of the Class
 *
 * @author     Noel Power
 * @created    August 2, 2002
 */
public class ScriptContext extends PropertySet implements XScriptContext
{
    /**
     *  Description of the Class
     *
     * @author     John Rice
     * @created    18/09/02
     */

    public final static String HM_DOC_REF = "DocumentReference";
    public final static String HM_DESKTOP = "Desktop";
    public final static String HM_COMPONENT_CONTEXT = "ComponentContext";

    private final static String DOC_REF = "SCRIPTING_DOC_REF";
    private final static String DOC_URI = "SCRIPTING_DOC_URI";


    public XModel m_xModel = null;
    public XScriptInvocationContext m_xInvocationContext = null;
    public String m_sDocURI = null;
    public XDesktop m_xDeskTop = null;
    public Integer m_iStorageID = null;
    public XComponentContext m_xComponentContext = null;

    public ScriptContext( XComponentContext xmComponentContext,
        XDesktop xDesktop, XModel xModel, XScriptInvocationContext xInvocContext)
    {
        this.m_xDeskTop = xDesktop;
        this.m_xComponentContext = xmComponentContext;
        this.m_xModel = xModel;
        this.m_xInvocationContext = xInvocContext;

        if ( m_xModel != null )
        {
            registerProperty( DOC_URI, new Type(String.class),
                (short)(PropertyAttribute.MAYBEVOID | PropertyAttribute.TRANSIENT), "m_sDocURI");
        }

        registerProperty( HM_DOC_REF, new Type(XModel.class),
            (short)(PropertyAttribute.MAYBEVOID | PropertyAttribute.TRANSIENT), "m_xModel");
        registerProperty( HM_DESKTOP, new Type(XDesktop.class),
            (short)(PropertyAttribute.MAYBEVOID | PropertyAttribute.TRANSIENT), "m_xDeskTop");
        registerProperty( HM_COMPONENT_CONTEXT, new Type(XDesktop.class),
            (short)(PropertyAttribute.MAYBEVOID | PropertyAttribute.TRANSIENT), "m_xComponentContext");
    }

    public static XScriptContext createContext( XModel xModel, XScriptInvocationContext xInvocContext,
        XComponentContext xCtxt, XMultiComponentFactory xMCF)
    {
        XScriptContext sc = null;

        try {

            Object xInterface = null;
            XDesktop xDesktop = null;

            xInterface = xMCF.createInstanceWithContext(
                "com.sun.star.frame.Desktop", xCtxt);
            xDesktop = (XDesktop)
                UnoRuntime.queryInterface(XDesktop.class, xInterface);
            if ( xModel != null )
            {
                sc = new ScriptContext(xCtxt, xDesktop, xModel, xInvocContext);
            }
            else
            {
                sc = new EditorScriptContext(xCtxt, xDesktop );
            }

        }
        catch ( Exception e ) {
            LogUtils.DEBUG( LogUtils.getTrace( e ) );
        }
        return sc;
    }

    //----------------------------------------------------------------------
    /**
        Obtain the document reference on which the script can operate

        @returns
          XModel interface
    */
    public XModel getDocument()
    {
        return m_xModel;
    }

    public XScriptInvocationContext getInvocationContext()
    {
        return m_xInvocationContext;
    }

    /**
        Obtain the desktop reference on which the script can operate

        @returns
          XDesktop interface
    */
    public XDesktop getDesktop()
    {
        return m_xDeskTop;
    }

    /**
        Obtain the component context which the script can use to create other uno components

        @returns
          XComponentContext interface
    */
    public XComponentContext getComponentContext()
    {
       return m_xComponentContext;
    }

}
