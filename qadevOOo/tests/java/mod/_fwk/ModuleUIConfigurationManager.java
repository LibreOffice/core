/*************************************************************************
 *
 *  $RCSfile: ModuleUIConfigurationManager.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Date: 2004-05-12 16:14:32 $
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
import drafts.com.sun.star.ui.XModuleUIConfigurationManagerSupplier;
import ifc.ui._XUIConfiguration;
import java.io.PrintWriter;
import com.sun.star.lang.EventObject;
import com.sun.star.text.XTextDocument;
import com.sun.star.util.XCloseable;
import drafts.com.sun.star.ui.ConfigurationEvent;
import drafts.com.sun.star.ui.XUIConfigurationManager;
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

    /**
     * Cleanup: close the created document
     * @param tParam The test parameters.
     * @param The log writer.
     * @return The test environment.
     */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");

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

    /**
     * Create environment.
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {
        TestEnvironment tEnv = null;
        try {
            xMSF = (XMultiServiceFactory)tParam.getMSF();

            log.println("Creating instance...");
            xTextDoc = WriterTools.createTextDoc(xMSF);

            Object o = (XInterface)xMSF.createInstance("drafts.com.sun.star.ui.ModuleUIConfigurationManagerSupplier");
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
            props[0] = util.utils.getOfficeTempDir(xMSF) + "dummyFile.dat";
            props[1] = new Integer(ElementModes.READWRITE);
            XStorage xStore = (XStorage)UnoRuntime.queryInterface(XStorage.class, xStorageService.createInstanceWithArguments(props));

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
