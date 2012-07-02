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
import com.sun.star.container.XNameAccess;
import com.sun.star.embed.ElementModes;
import com.sun.star.embed.XStorage;
import com.sun.star.embed.XTransactedObject;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.ui.XModuleUIConfigurationManagerSupplier;
import ifc.ui._XUIConfiguration;
import java.io.PrintWriter;
import com.sun.star.lang.EventObject;
import com.sun.star.text.XTextDocument;
import com.sun.star.util.XCloseable;
import com.sun.star.ui.ConfigurationEvent;
import com.sun.star.ui.XUIConfigurationManager;
import ifc.ui._XUIConfigurationManager;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.WriterTools;
import util.utils;

/**
 */
public class ModuleUIConfigurationManager extends TestCase {
    XInterface oObj = null;
    XMultiServiceFactory xMSF = null;
    XTextDocument xTextDoc = null;
    XStorage xStore = null;

    /**
     * Cleanup: close the created document
     * @param tParam The test parameters.
     * @param log The log writer.
     * @return The test environment.
     */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");
        if (xTextDoc != null) {
            try {
                XCloseable closer = UnoRuntime.queryInterface(
                    XCloseable.class, xTextDoc);
                closer.close(true);
            } catch (com.sun.star.util.CloseVetoException e) {
                log.println("couldn't close document");
            } catch (com.sun.star.lang.DisposedException e) {
                log.println("couldn't close document");
            }
        }
        log.println("   disposing storage");
        if (xStore != null) {
            xStore.dispose();
        }
    }

    /**
     * Create environment.
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {
        TestEnvironment tEnv = null;

        try {
            xMSF = (XMultiServiceFactory)tParam.getMSF();

            log.println("Creating instance...");
            xTextDoc = WriterTools.createTextDoc(xMSF);

            Object o = xMSF.createInstance("com.sun.star.ui.ModuleUIConfigurationManagerSupplier");
            XModuleUIConfigurationManagerSupplier xMUICMS = UnoRuntime.queryInterface(XModuleUIConfigurationManagerSupplier.class, o);

            util.dbg.printInterfaces(xMUICMS);
            oObj = xMUICMS.getUIConfigurationManager("com.sun.star.text.TextDocument");

            log.println("TestObject: " + utils.getImplName(oObj));
            tEnv = new TestEnvironment(oObj);

            XNameAccess xMM = UnoRuntime.queryInterface(XNameAccess.class, xMSF.createInstance("com.sun.star.comp.framework.ModuleManager"));
            xMM.getElementNames();

            o = xMSF.createInstance("com.sun.star.embed.StorageFactory");
            XSingleServiceFactory xStorageService = UnoRuntime.queryInterface(XSingleServiceFactory.class, o);
            Object[]props = new Object[2];

            String aFile = util.utils.getOfficeTempDir(xMSF) + "dummyFile.dat";
            log.println("storage file : '"+ aFile + "'");

            props[0] = aFile;
            props[1] = new Integer(ElementModes.READWRITE);
            xStore = UnoRuntime.queryInterface(XStorage.class, xStorageService.createInstanceWithArguments(props));

            PropertyValue[] initProps = new PropertyValue[4];
            PropertyValue propVal = new PropertyValue();
            propVal.Name = "DefaultConfigStorage";
            propVal.Value = xStore;
            initProps[0] = propVal;
            propVal = new PropertyValue();
            propVal.Name = "UserConfigStorage";
            propVal.Value = xStore;
            initProps[1] = propVal;
            propVal = new PropertyValue();
            propVal.Name = "ModuleIdentifier";
            propVal.Value = "swriter";
            initProps[2] = propVal;
            propVal = new PropertyValue();
            propVal.Name = "UserRootCommit";
            propVal.Value = UnoRuntime.queryInterface(XTransactedObject.class, xStore);
            initProps[3] = propVal;


            tEnv.addObjRelation("XInitialization.args", initProps);

            // the short cut manager service name
            // 2do: correct the service name when it's no longer in
            tEnv.addObjRelation("XConfigurationManager.ShortCutManager",
                "com.sun.star.ui.ModuleAcceleratorConfiguration");

            // the resourceURL
            tEnv.addObjRelation("XModuleUIConfigurationManager.ResourceURL",
                                            "private:resource/menubar/menubar");
            tEnv.addObjRelation("XUIConfiguration.XUIConfigurationListenerImpl",
                            new ConfigurationListener(log,
                            UnoRuntime.queryInterface(
                            XUIConfigurationManager.class, oObj), xMSF));
            tEnv.addObjRelation("XModuleUIConfigurationManagerSupplier.ConfigManagerImplementationName",
                        "com.sun.star.comp.framework.ModuleUIConfigurationManager");
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Cannot create test object", e);
        }
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

        public ConfigurationListener(PrintWriter _log, XUIConfigurationManager xUIManager, XMultiServiceFactory xMSF) {
            log = _log;
            this.xUIManager = xUIManager;
            this.xMSF = xMSF;
        }
        public void reset(){
            triggered = false;
        }
        public void fireEvent() {
            try {
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
