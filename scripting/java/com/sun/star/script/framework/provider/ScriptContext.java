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

package com.sun.star.script.framework.provider;

import com.sun.star.beans.PropertyAttribute;

import com.sun.star.document.XScriptInvocationContext;

import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XModel;

import com.sun.star.lang.XMultiComponentFactory;

import com.sun.star.lib.uno.helper.PropertySet;

import com.sun.star.script.framework.log.LogUtils;
import com.sun.star.script.provider.XScriptContext;

import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

public class ScriptContext extends PropertySet implements XScriptContext {

    private static final String HM_DOC_REF = "DocumentReference";
    private static final String HM_DESKTOP = "Desktop";
    private static final String HM_COMPONENT_CONTEXT = "ComponentContext";

    private static final String DOC_URI = "SCRIPTING_DOC_URI";

    public final XModel m_xModel;
    private final XScriptInvocationContext m_xInvocationContext;

    private final XDesktop m_xDeskTop;

    private final XComponentContext m_xComponentContext;

    // property, accessed via reflection
    @SuppressWarnings("URF_UNREAD_PUBLIC_OR_PROTECTED_FIELD")
    public String m_sDocURI = null;

    private ScriptContext(XComponentContext xmComponentContext, XDesktop xDesktop,
                          XModel xModel, XScriptInvocationContext xInvocContext) {

        this.m_xDeskTop = xDesktop;
        this.m_xComponentContext = xmComponentContext;
        this.m_xModel = xModel;
        this.m_xInvocationContext = xInvocContext;

        if (m_xModel != null) {

            registerProperty(DOC_URI, new Type(String.class),
                             (short)(PropertyAttribute.MAYBEVOID | PropertyAttribute.TRANSIENT),
                             "m_sDocURI");

        }

        registerProperty(HM_DOC_REF, new Type(XModel.class),
                         (short)(PropertyAttribute.MAYBEVOID | PropertyAttribute.TRANSIENT),
                         "m_xModel");

        registerProperty(HM_DESKTOP, new Type(XDesktop.class),
                         (short)(PropertyAttribute.MAYBEVOID | PropertyAttribute.TRANSIENT),
                         "m_xDeskTop");

        registerProperty(HM_COMPONENT_CONTEXT, new Type(XDesktop.class),
                         (short)(PropertyAttribute.MAYBEVOID | PropertyAttribute.TRANSIENT),
                         "m_xComponentContext");
    }

    public static XScriptContext createContext(XModel xModel,
            XScriptInvocationContext xInvocContext,
            XComponentContext xCtxt, XMultiComponentFactory xMCF) {

        XScriptContext sc = null;

        try {

            Object xInterface =
                xMCF.createInstanceWithContext("com.sun.star.frame.Desktop", xCtxt);

            XDesktop xDesktop = UnoRuntime.queryInterface(XDesktop.class, xInterface);

            if (xModel != null) {
                sc = new ScriptContext(xCtxt, xDesktop, xModel, xInvocContext);
            } else {
                sc = new EditorScriptContext(xCtxt, xDesktop);
            }

        } catch (Exception e) {
            LogUtils.DEBUG(LogUtils.getTrace(e));
        }

        return sc;
    }


    /**
        Obtain the document reference on which the script can operate

        @return  XModel interface
    */
    public XModel getDocument() {
        return m_xModel;
    }

    public XScriptInvocationContext getInvocationContext() {
        return m_xInvocationContext;
    }

    /**
        Obtain the desktop reference on which the script can operate

        @return  XDesktop interface
    */
    public XDesktop getDesktop() {
        return m_xDeskTop;
    }

    /**
        Obtain the component context which the script can use to create other uno components

        @return  XComponentContext interface
    */
    public XComponentContext getComponentContext() {
        return m_xComponentContext;
    }
}
