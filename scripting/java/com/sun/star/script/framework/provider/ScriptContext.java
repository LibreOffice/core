/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
