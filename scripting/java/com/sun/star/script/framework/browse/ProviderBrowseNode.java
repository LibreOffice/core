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

package com.sun.star.script.framework.browse;

import com.sun.star.beans.XIntrospectionAccess;

import com.sun.star.lang.XMultiComponentFactory;

import com.sun.star.lib.uno.helper.PropertySet;

import com.sun.star.script.XInvocation;
import com.sun.star.script.browse.BrowseNodeTypes;
import com.sun.star.script.browse.XBrowseNode;
import com.sun.star.script.framework.container.ParcelContainer;
import com.sun.star.script.framework.log.LogUtils;
import com.sun.star.script.framework.provider.ScriptProvider;

import com.sun.star.ucb.XSimpleFileAccess;

import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

import java.util.ArrayList;
import java.util.Collection;

import javax.swing.JOptionPane;

public class ProviderBrowseNode extends PropertySet implements
    XBrowseNode, XInvocation {

    private final ScriptProvider provider;
    private Collection<XBrowseNode> browsenodes;
    private final String name;
    protected ParcelContainer container;
    private final XComponentContext m_xCtx;
    // these are properties, they are accessed by reflection
    public boolean deletable = true;
    public boolean creatable = true;
    public boolean editable = false;

    public ProviderBrowseNode(ScriptProvider provider, ParcelContainer container,
                              XComponentContext xCtx) {

        LogUtils.DEBUG("*** ProviderBrowseNode ctor");
        this.container = container;
        this.name = this.container.getLanguage();
        this.provider = provider;
        this.m_xCtx = xCtx;

        registerProperty("Deletable", new Type(boolean.class), (short)0, "deletable");
        registerProperty("Creatable", new Type(boolean.class), (short)0, "creatable");
        registerProperty("Editable", new Type(boolean.class), (short)0, "editable");
        XMultiComponentFactory xFac = m_xCtx.getServiceManager();

        try {
            XSimpleFileAccess xSFA = UnoRuntime.queryInterface(XSimpleFileAccess.class,
                                         xFac.createInstanceWithContext(
                                             "com.sun.star.ucb.SimpleFileAccess",
                                             xCtx));
            if (  container.isUnoPkg() || xSFA.isReadOnly( container.getParcelContainerDir() ) )
            {
                deletable = false;
                creatable = false;
            }
        }
        // TODO propage errors
        catch (com.sun.star.uno.Exception e) {
            LogUtils.DEBUG("Caught exception in creation of ProviderBrowseNode ");
            LogUtils.DEBUG(LogUtils.getTrace(e));
        }
    }

    public String getName() {
        return name;
    }

    public XBrowseNode[] getChildNodes() {
        LogUtils.DEBUG("***** ProviderBrowseNode.getChildNodes()");

        if (hasChildNodes()) {

            // needs initialisation?
            LogUtils.DEBUG("** ProviderBrowseNode.getChildNodes(), container is " +
                           container);

            String[] parcels = container.getElementNames();
            browsenodes = new ArrayList<XBrowseNode>(parcels.length);

            for (String parcel : parcels) {
                try {
                    XBrowseNode node = new ParcelBrowseNode(provider, container, parcel);
                    browsenodes.add(node);
                } catch (Exception e) {
                    LogUtils.DEBUG("*** Failed to create parcel node for " + parcel);
                    LogUtils.DEBUG(e.toString());
                }
            }

            ParcelContainer[] packageContainers = container.getChildContainers();

            LogUtils.DEBUG("**** For container named " + container.getName() +
                           " with root path " + container.getParcelContainerDir() +
                           " has " + packageContainers.length + " child containers ");

            for (ParcelContainer packageContainer : packageContainers) {

                XBrowseNode node =
                    new PkgProviderBrowseNode(provider, packageContainer, m_xCtx);

                browsenodes.add(node);
            }
        } else {
            LogUtils.DEBUG("*** No container available");
            return new XBrowseNode[0];
        }

        return browsenodes.toArray(new XBrowseNode[browsenodes.size()]);
    }

    public boolean hasChildNodes() {
        boolean result = true;

        if (container == null ||
            (!container.hasElements() && container.getChildContainers().length == 0)) {

            result = false;
        }

        if (container == null) {
            LogUtils.DEBUG("***** ProviderBrowseNode.hasChildNodes(): " + "name=" + name +
                       ", path=<none>, result=" + result);
        } else {
            LogUtils.DEBUG("***** ProviderBrowseNode.hasChildNodes(): " + "name=" + name +
                       ", path=" + container.getParcelContainerDir() + ", result=" + result);
        }

        return result;
    }

    public short getType() {
        return BrowseNodeTypes.CONTAINER;
    }

    @Override
    public String toString() {
        return getName();
    }

    // implementation of XInvocation interface
    public XIntrospectionAccess getIntrospection() {
        return null;
    }

    public Object invoke(String aFunctionName, Object[] aParams,
                         short[][] aOutParamIndex, Object[][] aOutParam) throws
        com.sun.star.lang.IllegalArgumentException,
        com.sun.star.script.CannotConvertException,
        com.sun.star.reflection.InvocationTargetException {

        // Initialise the out parameters - not used but prevents error in
        // UNO bridge
        aOutParamIndex[0] = new short[0];
        aOutParam[0] = new Object[0];

        Any result = new Any(new Type(Boolean.class), Boolean.TRUE);

        if (aFunctionName.equals("Creatable")) {
            try {
                String name;

                if (aParams == null || aParams.length < 1 ||
                    !AnyConverter.isString(aParams[0])) {

                    String prompt = "Enter name for new Parcel";
                    String title = "Create Parcel";

                    // try to get a DialogFactory instance, if it fails
                    // just use a Swing JOptionPane to prompt for the name
                    try {
                        DialogFactory dialogFactory = DialogFactory.getDialogFactory();
                        name = dialogFactory.showInputDialog(title, prompt);
                    } catch (Exception e) {

                        name = JOptionPane.showInputDialog(null, prompt, title,
                                                           JOptionPane.QUESTION_MESSAGE);

                    }
                } else {
                    name = AnyConverter.toString(aParams[0]);
                }

                if (name == null || name.length() == 0) {
                    result =  new Any(new Type(Boolean.class), Boolean.FALSE);
                } else {

                    Object newParcel  = container.createParcel(name);
                    LogUtils.DEBUG("Parcel created " + name + " " + newParcel);

                    if (newParcel == null) {
                        result =  new Any(new Type(Boolean.class), Boolean.FALSE);
                    } else {
                        ParcelBrowseNode parcel = new ParcelBrowseNode(provider, container, name);
                        LogUtils.DEBUG("created parcel node ");

                        if (browsenodes == null) {
                            browsenodes = new ArrayList<XBrowseNode>(5);
                        }

                        browsenodes.add(parcel);
                        result = new Any(new Type(XBrowseNode.class), parcel);
                    }
                }
            } catch (Exception e) {
                LogUtils.DEBUG("ProviderBrowseNode[create] failed with: " + e);
                LogUtils.DEBUG(LogUtils.getTrace(e));
                result = new Any(new Type(Boolean.class), Boolean.FALSE);
            }
        } else {
            throw new com.sun.star.lang.IllegalArgumentException(
                "Function " + aFunctionName + " not supported.");
        }

        return result;
    }

    public void setValue(String aPropertyName, Object aValue) throws
        com.sun.star.beans.UnknownPropertyException,
        com.sun.star.script.CannotConvertException,
        com.sun.star.reflection.InvocationTargetException {
    }

    public Object getValue(String aPropertyName) throws
        com.sun.star.beans.UnknownPropertyException {

        return null;
    }

    public boolean hasMethod(String aName) {
        return false;
    }

    public boolean hasProperty(String aName) {
        return false;
    }
}
