/*************************************************************************
*
*  $RCSfile: ScriptContext.java,v $
*
*  $Revision: 1.3 $
*
*  last change: $Author: dfoster $ $Date: 2003-11-04 17:45:27 $
*
*  The Contents of this file are made available subject to the terms of
*  either of the following licenses
*
*         - GNU Lesser General Public License Version 2.1
*         - Sun Industry Standards Source License Version 1.1
*
*  Sun Microsystems Inc., October, 2000
*
*  GNU Lesser General Public License Version 2.1
*  =============================================
*  Copyright 2000 by Sun Microsystems, Inc.
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
*  Copyright: 2000 by Sun Microsystems, Inc.
*
*  All Rights Reserved.
*
*  Contributor(s): _______________________________________
*
*
************************************************************************/
package com.sun.star.script.framework.provider;

import com.sun.star.frame.XModel;
import com.sun.star.frame.XDesktop;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XVetoableChangeListener;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.lib.uno.helper.PropertySet;
import com.sun.star.uno.Type;

import java.util.HashMap;

import drafts.com.sun.star.script.provider.XScriptContext;

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
    private final static String DOC_STORAGE_ID = "SCRIPTING_DOC_STORAGE_ID";
    private final static String DOC_URI = "SCRIPTING_DOC_URI";


    public XModel m_xModel = null;
    public String m_sDocURI = null;
    public XDesktop m_xDeskTop = null;
    public Integer m_iStorageID = null;
    public XComponentContext m_xComponentContext = null;

    public ScriptContext( XComponentContext xmComponentContext,
        XDesktop xDesktop, XPropertySet invocationCtxPropSet)
    {
        this.m_xDeskTop = xDesktop;
        this.m_xComponentContext = xmComponentContext;
        try
        {
            m_xModel = ( XModel ) UnoRuntime.queryInterface(
                XModel.class,
                invocationCtxPropSet.getPropertyValue( DOC_REF ) );
            m_iStorageID = new Integer ( AnyConverter.toInt( invocationCtxPropSet.getPropertyValue(
                    DOC_STORAGE_ID ) ) );
            m_sDocURI = ( String )
                invocationCtxPropSet.getPropertyValue( DOC_URI );

            LogUtils.DEBUG( "DOC_REF query for URL = " + m_xModel.getURL() );
            LogUtils.DEBUG( "DOC_STORAGE_ID = " + m_iStorageID );
            LogUtils.DEBUG( "DOC_URI query for URL = " + m_sDocURI );

            registerProperty( DOC_STORAGE_ID, new Type(Integer.class),
            (short)(PropertyAttribute.MAYBEVOID | PropertyAttribute.TRANSIENT), "m_iStorageID");
            registerProperty( DOC_URI, new Type(String.class),
            (short)(PropertyAttribute.MAYBEVOID | PropertyAttribute.TRANSIENT), "m_sDocURI");
        }
        catch ( UnknownPropertyException upe )
        {
            upe.printStackTrace();
        }
        catch ( WrappedTargetException wte )
        {
            wte.printStackTrace();
        }
        catch ( IllegalArgumentException iae )
        {
            iae.printStackTrace();
        }
        registerProperty( HM_DOC_REF, new Type(XModel.class),
            (short)(PropertyAttribute.MAYBEVOID | PropertyAttribute.TRANSIENT), "m_xModel");
        registerProperty( HM_DESKTOP, new Type(XDesktop.class),
            (short)(PropertyAttribute.MAYBEVOID | PropertyAttribute.TRANSIENT), "m_xDeskTop");
        registerProperty( HM_COMPONENT_CONTEXT, new Type(XDesktop.class),
            (short)(PropertyAttribute.MAYBEVOID | PropertyAttribute.TRANSIENT), "m_xComponentContext");
    }

    public static ScriptContext createContext(Object invocationCtx,
        XComponentContext xCtxt, XMultiComponentFactory xMCF)
    {
        ScriptContext sc = null;

        try {
            XPropertySet invocationCtxPropSet = (XPropertySet)
                UnoRuntime.queryInterface( XPropertySet.class, invocationCtx);

            Object xInterface = null;
            XDesktop xDesktop = null;

            xInterface = xMCF.createInstanceWithContext(
                "com.sun.star.frame.Desktop", xCtxt);
            xDesktop = (XDesktop)
                UnoRuntime.queryInterface(XDesktop.class, xInterface);

            sc = new ScriptContext(xCtxt, xDesktop, invocationCtxPropSet);
        }
        catch ( Exception e ) {
            e.printStackTrace();
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
