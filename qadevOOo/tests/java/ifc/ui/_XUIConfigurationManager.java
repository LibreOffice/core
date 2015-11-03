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
package ifc.ui;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XIndexContainer;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.ui.UIElementType;
import com.sun.star.ui.XImageManager;
import com.sun.star.ui.XUIConfigurationManager;
import java.io.PrintWriter;
import lib.MultiMethodTest;

public class _XUIConfigurationManager extends MultiMethodTest {

    public XUIConfigurationManager oObj;
    private static final String msResourceUrl = "private:resource/menubar/menubar";
    private static final String msMyResourceUrl = "private:resource/menubar/mymenubar";
    private XIndexContainer mxSettings = null;
    private XIndexAccess mxMenuBarSettings = null;
    private XMultiServiceFactory mxMSF = null;
    private String sShortCutManagerServiceName = null;


    /**
     * Some stuff before the tests:
     * extract the multi service factory.
     */
    @Override
    protected void before() {
        mxMSF = tParam.getMSF();
        sShortCutManagerServiceName = (String)tEnv.getObjRelation("XConfigurationManager.ShortCutManager");

    }

    /**
     * reset all changes: do at the end.
     */
    public void _reset() {
        requiredMethod("removeSettings()");
        oObj.reset();
        tRes.tested("reset()", true);
    }

    public void _getUIElementsInfo() {
        boolean result = true;
        try {
            PropertyValue[][]props = oObj.getUIElementsInfo(UIElementType.UNKNOWN);
            for (int i=0; i<props.length; i++)
                for(int j=0; j<props[i].length; j++)
                    log.println("Prop["+i+"]["+j+"]: " + props[i][j].Name + "   " + props[i][j].Value.toString());
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            result = false;
            e.printStackTrace(log);
        }
        tRes.tested("getUIElementsInfo()", result);
    }

    public void _createSettings() {
        mxSettings = oObj.createSettings();
        util.dbg.printInterfaces(mxSettings);
        tRes.tested("createSettings()", mxSettings != null);
    }

    public void _hasSettings() {
        boolean result = false;
        try {
            result = oObj.hasSettings(msResourceUrl);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println(e);
            result = false;
        }
        tRes.tested("hasSettings()", result);
    }

    public void _getSettings() {
        requiredMethod("hasSettings()");
        boolean result = true;
        try {
            mxMenuBarSettings = oObj.getSettings(msResourceUrl, true);
            for (int i=0; i<mxMenuBarSettings.getCount(); i++) {
                Object[] o = (Object[])mxMenuBarSettings.getByIndex(i);
                log.println("+++++++++ i = " + i);
                for (int j=0; j<o.length; j++) {
                    PropertyValue prop = (PropertyValue)o[j];
                    log.println("Property" + j + ": " + prop.Name + "   " + prop.Value.toString());
                }
            }
        }
        catch(com.sun.star.container.NoSuchElementException e) {
            result = false;
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            result = false;
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            result = false;
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            result = false;
            e.printStackTrace(log);
        }
        tRes.tested("getSettings()", result);
    }

    public void _replaceSettings() {
        requiredMethod("getSettings()");
        boolean result = true;
        PropertyValue[] prop = createMenuBarEntry("My Entry", mxMenuBarSettings, mxMSF, log);
        if (prop == null) {
            tRes.tested("replaceSettings()", false);
            return;
        }

        createMenuBarItem("Click for Macro", UnoRuntime.queryInterface(
                                        XIndexContainer.class, prop[3].Value), log);

        XIndexContainer x = UnoRuntime.queryInterface(XIndexContainer.class, mxMenuBarSettings);
        try {
            x.insertByIndex(x.getCount(), prop);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            result = false;
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            result = false;
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            result = false;
            e.printStackTrace(log);
        }

        try {
            oObj.replaceSettings(msResourceUrl, mxMenuBarSettings);
        }
        catch(com.sun.star.container.NoSuchElementException e) {
            result = false;
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            result = false;
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.IllegalAccessException e) {
            result = false;
            e.printStackTrace(log);
        }
        _getSettings();
        tRes.tested("replaceSettings()", result);
    }

    public void _removeSettings() {
        requiredMethod("insertSettings()");
        boolean result = true;
        try {
            oObj.removeSettings(msMyResourceUrl);
        }
        catch(com.sun.star.container.NoSuchElementException e) {
            result = false;
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            result = false;
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.IllegalAccessException e) {
            e.printStackTrace(log);
            result = false;
        }
        tRes.tested("removeSettings()", result);
    }

    public void _insertSettings() {
        requiredMethod("createSettings()");
        requiredMethod("replaceSettings()");
        boolean result = true;
        util.dbg.printInterfaces(mxSettings);
        PropertyValue[] prop = createMenuBarEntry("A new entry", mxSettings, mxMSF, log);
        if (prop == null) {
            tRes.tested("replaceSettings()", false);
            return;
        }

        createMenuBarItem("A new sub entry", UnoRuntime.queryInterface(
                                        XIndexContainer.class, prop[3].Value), log);

        XIndexContainer x = UnoRuntime.queryInterface(XIndexContainer.class,mxSettings);
        try {
            int count = x.getCount();
            x.insertByIndex(count, prop);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            result = false;
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            result = false;
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            result = false;
            e.printStackTrace(log);
        }

        try {
            oObj.insertSettings(msMyResourceUrl, mxSettings);
        }
        catch(com.sun.star.container.ElementExistException e) {
            e.printStackTrace(log);
            result = false;
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            result = false;
        }
        catch(com.sun.star.lang.IllegalAccessException e) {
            e.printStackTrace(log);
            result = false;
        }
        tRes.tested("insertSettings()", result);
    }

    /**
     * Only a short test.
     * See complex.imageManager.CheckImageManager for a more extensive test of
     * this implementation.
     */
    public void _getImageManager() {
        Object o = oObj.getImageManager();
        log.println("###### ImageManager ");
        XImageManager xImageManager = UnoRuntime.queryInterface(XImageManager.class, o);
        tRes.tested("getImageManager()", xImageManager != null);
    }


    /**
     * get a shortcut manager
     */
    public void _getShortCutManager() {
        Object o = oObj.getShortCutManager();
        XServiceInfo xSI = UnoRuntime.queryInterface(XServiceInfo.class,o);
        String[] serviceNames = xSI.getSupportedServiceNames();
        boolean bSupportedServiceFound = false;
        for (int i=0; i<serviceNames.length; i++) {
            log.println("SuppService: " + serviceNames[i]);
            if (serviceNames[i].equals(sShortCutManagerServiceName)) {
                bSupportedServiceFound = true;
            }
        }
        tRes.tested("getShortCutManager()", bSupportedServiceFound);
    }

    public void _getEventsManager() {
        Object o = oObj.getEventsManager();
        tRes.tested("getEventsManager()", o == null);
    }

    /**
     * Create  a menu bar entry for adding to the menu bar of the Office.
     * @param sLabelName The name of the new entry.
     * @param xMenuBarSettings The existing menu bar settings, used for creating the new entry.
     * @return An array of properties of the new entry.
     */
    public static PropertyValue[] createMenuBarEntry(String sLabelName, XIndexAccess xMenuBarSettings, XMultiServiceFactory xMSF, PrintWriter log) {
        PropertyValue[] prop = new PropertyValue[4];
        prop[0] = new PropertyValue();
        prop[0].Name = "CommandURL";
        prop[0].Value = "vnd.openoffice.org:MyMenu";
        prop[1] = new PropertyValue();
        prop[1].Name = "Label";
        prop[1].Value = sLabelName;
        prop[2] = new PropertyValue();
        prop[2].Name = "Type";
        prop[2].Value = Short.valueOf((short)0);
        prop[3] = new PropertyValue();
        prop[3].Name = "ItemDescriptorContainer";

        XSingleComponentFactory xFactory = UnoRuntime.queryInterface(
                                XSingleComponentFactory.class, xMenuBarSettings);
        try {
            XPropertySet xProp = UnoRuntime.queryInterface(XPropertySet.class, xMSF);
            XComponentContext xContext = UnoRuntime.queryInterface(
                    XComponentContext.class, xProp.getPropertyValue("DefaultContext"));
            prop[3].Value = xFactory.createInstanceWithContext(xContext);
        }
        catch(com.sun.star.uno.Exception e) {
            log.println("Could not create an instance for ItemDescriptorContainer property.");
            e.printStackTrace(log);
            return null;
        }
        return prop;
    }

    /**
     * Create a sub entry to the menu bar.
     * @param sLabelName The name of the entry in the UI.
     * @param xDescriptionContainer The parent entry in the menu bar where
     *      this entry is added.
     */
    public static void createMenuBarItem(String sLabelName, XIndexContainer xDescriptionContainer, PrintWriter log) {
        PropertyValue[]aMenuItem = new PropertyValue[3];
        // create a menu item
        aMenuItem[0] = new PropertyValue();
        aMenuItem[0].Name = "CommandURL";
        aMenuItem[0].Value = "macro:///Standard.Module1.Test()";
        aMenuItem[1] = new PropertyValue();
        aMenuItem[1].Name = "Label";
        aMenuItem[1].Value = sLabelName;
        aMenuItem[2] = new PropertyValue();
        aMenuItem[2].Name = "Type";
        aMenuItem[2].Value = Short.valueOf((short)0);

        try {
            xDescriptionContainer.insertByIndex(0, aMenuItem);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
        }
    }
}
