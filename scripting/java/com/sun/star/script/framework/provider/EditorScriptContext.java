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

import com.sun.star.frame.XModel;
import com.sun.star.frame.XDesktop;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.document.XScriptInvocationContext;

import com.sun.star.script.provider.XScriptContext;


/**
 *  Description of the Class
 *
 * @author     Noel Power
 * @created    August 2, 2002
 */
public class EditorScriptContext implements XScriptContext
{
    private XDesktop m_xDeskTop;
    private XComponentContext  m_xComponentContext;
    private XDesktop m_xCtx;
    public EditorScriptContext( XComponentContext xmComponentContext,
        XDesktop xDesktop )
    {
        this.m_xComponentContext = xmComponentContext;
        this.m_xDeskTop = xDesktop;
    }

    //----------------------------------------------------------------------
    /**
        Obtain the document reference on which the script can operate

        @returns
          XModel interface
    */
    public XModel getDocument()
    {
        XModel xModel = ( XModel ) UnoRuntime.queryInterface( XModel.class,
             m_xDeskTop.getCurrentComponent() );

        return xModel;
    }

    public XScriptInvocationContext getInvocationContext()
    {
        XScriptInvocationContext xContext = ( XScriptInvocationContext ) UnoRuntime.queryInterface(
                XScriptInvocationContext.class, getDocument() );
        return xContext;
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
