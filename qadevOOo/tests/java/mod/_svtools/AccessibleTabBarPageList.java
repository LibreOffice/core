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
package mod._svtools;

import com.sun.star.drawing.XLayerManager;
import com.sun.star.drawing.XLayerSupplier;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.SOfficeFactory;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;


/**
 * Test for object that implements the following interfaces :
 * <ul>
 *  <li><code>::com::sun::star::accessibility::XAccessibleComponent
 *  </code></li>
 *  <li><code>::com::sun::star::accessibility::XAccessibleContext
 *  </code></li>
 *  <li><code>::com::sun::star::accessibility::XAccessibleEventBroadcaster
 *  </code></li>
 *  <li>
 *  <code>::com::sun::star::accessibility::XAccessibleExtendedComponent
 *  </code></li>
 *  <li><code>::com::sun::star::accessibility::XAccessibleSelection
 *  </code></li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleSelection
 * @see com.sun.star.accessibility.XAccessibleExtendedComponent
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see com.sun.star.accessibility.XAccessibleComponent
 * @see ifc.accessibility._XAccessibleSelection
 * @see ifc.accessibility._XAccessibleComponent
 * @see ifc.accessibility._XAccessibleExtendedComponent
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleContext
 */
public class AccessibleTabBarPageList extends TestCase {
    static XComponent xDoc;

    /**
     * Disposes the document, if exists, created in
     * <code>createTestEnvironment</code> method.
     */
    @Override
    protected void cleanup(TestParameters Param, PrintWriter log) {
        log.println("disposing xDoc");

        if (xDoc != null) {
            closeDoc();
        }
    }

    /**
     * Creates a spreadsheet document. Then obtains an accessible object with
     * the role <code>AccessibleRole.PAGETABLIST</code>.
     * Object relations created :
     * <ul>
     *  <li> <code>'EventProducer'</code> for
     *      {@link ifc.accessibility._XAccessibleEventBroadcaster}:
     *   </li>
     * </ul>
     *
     * @param tParam test parameters
     * @param log writer to log information while testing
     *
     * @see com.sun.star.awt.Toolkit
     * @see com.sun.star.accessibility.AccessibleRole
     * @see ifc.accessibility._XAccessibleEventBroadcaster
     * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
     */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters tParam,
                                                    PrintWriter log) throws Exception {
        log.println("creating a test environment");

        if (xDoc != null) {
            closeDoc();
        }

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF());

        log.println("creating a draw document");
        xDoc = SOF.createDrawDoc(null);

        util.utils.waitForEventIdle(tParam.getMSF());

        XInterface oObj = (XInterface) tParam.getMSF()
                                      .createInstance("com.sun.star.awt.Toolkit");

        XExtendedToolkit tk = UnoRuntime.queryInterface(
                                      XExtendedToolkit.class, oObj);

        util.utils.waitForEventIdle(tParam.getMSF());

        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class,
                                                              tk.getActiveTopWindow());

        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        oObj = AccessibilityTools.getAccessibleObjectForRole(xRoot,
                                             AccessibleRole.PAGE_TAB_LIST);

        log.println("ImplementationName: " + util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("XAccessibleSelection.multiSelection",
                            Boolean.FALSE);

        tEnv.addObjRelation("XAccessibleSelection.OneAlwaysSelected",
                            Boolean.TRUE);

        XLayerSupplier oLS = UnoRuntime.queryInterface(XLayerSupplier.class, xDoc);
        XInterface oLM = oLS.getLayerManager();
        final XLayerManager xLM = UnoRuntime.queryInterface(XLayerManager.class, oLM);


        tEnv.addObjRelation("EventProducer",
                            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
            public void fireEvent() {
                xLM.insertNewByIndex(0);
            }
        });

        return tEnv;
    }

    protected void closeDoc() {
        XCloseable closer = UnoRuntime.queryInterface(
                                    XCloseable.class, xDoc);

        try {
            closer.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            log.println("Couldn't close document " + e.getMessage());
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("Couldn't close document " + e.getMessage());
        }
    }
}