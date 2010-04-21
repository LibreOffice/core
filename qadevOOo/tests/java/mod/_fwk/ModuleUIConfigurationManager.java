/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
     * @param The log writer.
     * @return The test environment.
     */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");
        if (xTextDoc != null) {
            try {
                XCloseable closer = (XCloseable) UnoRuntime.queryInterface(
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

            Object o = (XInterface)xMSF.createInstance("com.sun.star.ui.ModuleUIConfigurationManagerSupplier");
            XModuleUIConfigurationManagerSupplier xMUICMS = (XModuleUIConfigurationManagerSupplier)
            UnoRuntime.queryInterface(XModuleUIConfigurationManagerSupplier.class, o);

            util.dbg.printInterfaces(xMUICMS);
            oObj = xMUICMS.getUIConfigurationManager("com.sun.star.text.TextDocument");

            log.println("TestObject: " + utils.getImplName(oObj));
            tEnv = new TestEnvironment(oObj);

            XNameAccess xMM = (XNameAccess)UnoRuntime.queryInterface(XNameAccess.class, xMSF.createInstance("com.sun.star.comp.framework.ModuleManager"));
            String[] names = xMM.getElementNames();

            o = xMSF.createInstance("com.sun.star.embed.StorageFactory");
            XSingleServiceFactory xStorageService = (XSingleServiceFactory)
                    UnoRuntime.queryInterface(XSingleServiceFactory.class, o);
            Object[]props = new Object[2];

            String aFile = util.utils.getOfficeTempDir(xMSF) + "dummyFile.dat";
            log.println("storage file : '"+ aFile + "'");

            props[0] = aFile;
            props[1] = new Integer(ElementModes.READWRITE);
            xStore = (XStorage)UnoRuntime.queryInterface(XStorage.class, xStorageService.createInstanceWithArguments(props));

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
            propVal.Value = (XTransactedObject)UnoRuntime.queryInterface(XTransactedObject.class, xStore);
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
                            (XUIConfigurationManager)UnoRuntime.queryInterface(
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
                                (XIndexContainer)UnoRuntime.queryInterface(
                                XIndexContainer.class, prop[3].Value), log);
                XIndexContainer x = (XIndexContainer)UnoRuntime.queryInterface(XIndexContainer.class, xMenuBarSettings);
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
