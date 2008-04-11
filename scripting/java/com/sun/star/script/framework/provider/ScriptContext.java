/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ScriptContext.java,v $
 * $Revision: 1.11 $
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
