/*************************************************************************
 *
 *  $RCSfile: UIConfigurationManager.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Date: 2004-05-12 16:15:34 $
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
import com.sun.star.embed.ElementModes;
import com.sun.star.embed.XStorage;
import com.sun.star.embed.XTransactedObject;
import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import drafts.com.sun.star.ui.XModuleUIConfigurationManagerSupplier;
import ifc.ui._XUIConfiguration;
import java.io.PrintWriter;
import com.sun.star.lang.EventObject;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.util.XCloseable;
import drafts.com.sun.star.ui.ConfigurationEvent;
import drafts.com.sun.star.ui.UIElementType;
import drafts.com.sun.star.ui.XUIConfigurationManager;
import drafts.com.sun.star.ui.XUIConfigurationManagerSupplier;
import drafts.com.sun.star.ui.XUIConfigurationStorage;
import drafts.com.sun.star.ui.XUIElement;
import drafts.com.sun.star.ui.XUIElementFactory;
import ifc.ui._XUIConfigurationManager;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.WriterTools;
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
     * @param The log writer.
     * @return The test environment.
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {
        TestEnvironment tEnv = null;
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();

        log.println("Creating instance...");

        try {
            xManager = (XUIConfigurationManager)UnoRuntime.queryInterface(
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
            XStorage xSubStorage = null;
            Object o = (XInterface)xMSF.createInstance("com.sun.star.embed.StorageFactory");
            XSingleServiceFactory xSSF = (XSingleServiceFactory)UnoRuntime.queryInterface(
            XSingleServiceFactory.class, o);
            Object[] props = new Object[2];
            props[0] = util.utils.getFullTestURL("delete.cfg");
            props[1] = new Integer(ElementModes.READWRITE);
            XStorage xRootStorage = (XStorage)UnoRuntime.queryInterface(XStorage.class, xSSF.createInstanceWithArguments(props));
            xSubStorage = xRootStorage.openStorageElement("Configurations2", ElementModes.READWRITE);

            XUIConfigurationStorage xConfigStorage =(XUIConfigurationStorage)UnoRuntime.queryInterface(XUIConfigurationStorage.class, xManager);
            xConfigStorage.setStorage(xSubStorage);
            tEnv.addObjRelation("XUIConfigurationStorage.Storage", xSubStorage);
        }
        catch(com.sun.star.uno.Exception e) {
            log.println("Could not create storage.");
        }
        util.dbg.printInterfaces(xManager);

        tEnv.addObjRelation("XUIConfiguration.XUIConfigurationListenerImpl",
        new ConfigurationListener(log, xManager, xMSF));
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
                    PropertyValue[][]props = xUIManager.getUIElementsInfo(UIElementType.UNKNOWN);
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


