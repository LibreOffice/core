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

package mod._fwk;

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XIndexContainer;
import com.sun.star.embed.ElementModes;
import com.sun.star.embed.XStorage;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.uno.UnoRuntime;
import ifc.ui._XUIConfiguration;
import java.io.PrintWriter;
import com.sun.star.lang.EventObject;
import com.sun.star.ui.ConfigurationEvent;
import com.sun.star.ui.UIElementType;
import com.sun.star.ui.XUIConfigurationManager;
import com.sun.star.ui.XUIConfigurationStorage;
import ifc.ui._XUIConfigurationManager;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

/**
 */
public class UIConfigurationManager extends TestCase {
    XUIConfigurationManager xManager = null;


    /**
     * Create test environment:
     * <ul>
     * <li>create a text doc</li>
     * <li>get the model from the text doc</li>
     * <li>query model for XUIConfigurationManagerSupplier interface</li>
     * <li>get the manager from the supplier</li>
     * </ul>
     * @param tParam The test parameters.
     * @param log The log writer.
     * @return The test environment.
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {
        TestEnvironment tEnv = null;
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();

        log.println("Creating instance...");

        try {
            xManager = UnoRuntime.queryInterface(
                    XUIConfigurationManager.class, xMSF.createInstance(
                    "com.sun.star.comp.framework.UIConfigurationManager"));
        }
        catch(com.sun.star.uno.Exception e) {
        }
        // just to make sure, it's the right one.
        log.println("TestObject: " + utils.getImplName(xManager));
        tEnv = new TestEnvironment(xManager);

        // create a configuration storage
        try {
            String sourceDeleteCfg = util.utils.getFullTestURL("delete.cfg");
            String deleteCfg = util.utils.getFullURL(util.utils.getOfficeTemp(xMSF) + "delete.cfg");
            util.utils.copyFile(xMSF, sourceDeleteCfg, deleteCfg);
            XStorage xSubStorage = null;
            Object o = xMSF.createInstance("com.sun.star.embed.StorageFactory");
            XSingleServiceFactory xSSF = UnoRuntime.queryInterface(
            XSingleServiceFactory.class, o);
            Object[] props = new Object[2];
            props[0] = deleteCfg;
            props[1] = new Integer(ElementModes.READWRITE);
            XStorage xRootStorage = UnoRuntime.queryInterface(XStorage.class, xSSF.createInstanceWithArguments(props));
            xSubStorage = xRootStorage.openStorageElement("Configurations2", ElementModes.READWRITE);

            XUIConfigurationStorage xConfigStorage =UnoRuntime.queryInterface(XUIConfigurationStorage.class, xManager);
            xConfigStorage.setStorage(xSubStorage);
            tEnv.addObjRelation("XUIConfigurationStorage.Storage", xSubStorage);
        }
        catch(com.sun.star.uno.Exception e) {
            log.println("Could not create storage: " + e.toString());
        }
        util.dbg.printInterfaces(xManager);

        tEnv.addObjRelation("XUIConfiguration.XUIConfigurationListenerImpl",
            new ConfigurationListener(log, xManager, xMSF));

        // the short cut manager service name
        // 2do: correct the service name when it's no longer in
        tEnv.addObjRelation("XConfigurationManager.ShortCutManager",
            "com.sun.star.ui.DocumentAcceleratorConfiguration");
        return tEnv;
    }

    /**
     * An implementation of the _XUIConfiguration.XUIConfigurationListenerImpl
     * interface to trigger the event for a listener call.
     * @see ifc.ui._XUIConfiguration
     */
    public static class ConfigurationListener implements _XUIConfiguration.XUIConfigurationListenerImpl {
        private boolean triggered = false;
        private PrintWriter log = null;
        private XUIConfigurationManager xUIManager = null;
        private XMultiServiceFactory xMSF = null;
        private static int iUniqueCounter;

        public ConfigurationListener(PrintWriter _log, XUIConfigurationManager xUIManager, XMultiServiceFactory xMSF) {
            log = _log;
            this.xUIManager = xUIManager;
            this.xMSF = xMSF;
            iUniqueCounter = 0;
        }
        public void reset(){
            triggered = false;
        }
        public void fireEvent() {
            try {
                if (iUniqueCounter == 0) {
                    iUniqueCounter++;
                    xUIManager.getUIElementsInfo(UIElementType.UNKNOWN);
                    XIndexAccess xMenuBarSettings = xUIManager.getSettings(
                    "private:resource/menubar/menubar", true);

                    PropertyValue[]prop = _XUIConfigurationManager.createMenuBarEntry(
                    "Trigger Event", xMenuBarSettings, xMSF, log);
                    _XUIConfigurationManager.createMenuBarItem("Click for Macro",
                    UnoRuntime.queryInterface(
                    XIndexContainer.class, prop[3].Value), log);
                    XIndexContainer x = UnoRuntime.queryInterface(XIndexContainer.class, xMenuBarSettings);
                    x.insertByIndex(x.getCount(), prop);
                    xUIManager.replaceSettings("private:resource/menubar/menubar", xMenuBarSettings);
                    xUIManager.reset();
                }
            }
            catch(com.sun.star.container.NoSuchElementException e) {
                log.println("_XUIConfiguration.XUIConfigurationListenerImpl: Exception.");
                e.printStackTrace(log);
            }
            catch(com.sun.star.lang.IllegalArgumentException e) {
                log.println("_XUIConfiguration.XUIConfigurationListenerImpl: Exception.");
                e.printStackTrace(log);
            }
            catch(com.sun.star.lang.IllegalAccessException e) {
                log.println("_XUIConfiguration.XUIConfigurationListenerImpl: Exception.");
                e.printStackTrace(log);
            }
            catch(com.sun.star.lang.IndexOutOfBoundsException e) {
                log.println("_XUIConfiguration.XUIConfigurationListenerImpl: Exception.");
                e.printStackTrace(log);
            }
            catch(com.sun.star.lang.WrappedTargetException e) {
                log.println("_XUIConfiguration.XUIConfigurationListenerImpl: Exception.");
                e.printStackTrace(log);
            }
        }
        public boolean actionWasTriggered(){
            return triggered;
        }
        public void disposing(EventObject e) {
            triggered = true;
            log.println("_XUIConfiguration.XUIConfigurationListenerImpl.disposing the listener.");
        }
        public void elementInserted(ConfigurationEvent configEvent) {
            triggered = true;
            log.println("_XUIConfiguration.XUIConfigurationListenerImpl.elementInserted.");
        }
        public void elementRemoved(ConfigurationEvent configEvent) {
            triggered = true;
            log.println("_XUIConfiguration.XUIConfigurationListenerImpl.elementRemoved.");
        }
        public void elementReplaced(ConfigurationEvent configEvent) {
            triggered = true;
            log.println("_XUIConfiguration.XUIConfigurationListenerImpl.elementReplaced.");
        }
    }

}


