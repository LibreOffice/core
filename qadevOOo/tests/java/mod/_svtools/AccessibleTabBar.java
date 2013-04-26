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

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.SOfficeFactory;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.awt.PosSize;
import com.sun.star.awt.XWindow;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;


public class AccessibleTabBar extends TestCase {
    static XComponent xDoc;

    /**
     * Disposes the document, if exists, created in
     * <code>createTestEnvironment</code> method.
     */
    protected void cleanup(TestParameters Param, PrintWriter log) {
        log.println("disposing xCalcDoc");

        if (xDoc != null) {
            closeDoc(xDoc);
        }
    }

    /**
     * Creates a spreadsheet document. Then obtains an accessible object with
     * the role <code>AccessibleRole.PAGETAB</code>.
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
    protected TestEnvironment createTestEnvironment(TestParameters tParam,
                                                    PrintWriter log) {
        log.println("creating a test environment");

        if (xDoc != null) {
            closeDoc(xDoc);
        }

        XMultiServiceFactory msf = (XMultiServiceFactory) tParam.getMSF();

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(msf);

        try {
            log.println("creating a calc document");
            xDoc = UnoRuntime.queryInterface(XComponent.class,
                                                          SOF.createCalcDoc(
                                                                  null));
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }

        shortWait();

        XInterface oObj = null;

        AccessibilityTools at = new AccessibilityTools();

        shortWait();

        XWindow xWindow = UnoRuntime.queryInterface(XModel.class, xDoc).
            getCurrentController().getFrame().getContainerWindow();

        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);
        AccessibilityTools.printAccessibleTree(log, xRoot, tParam.getBool(util.PropertyName.DEBUG_IS_ACTIVE));
        oObj = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.PANEL);

        log.println("ImplementationName: " + util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        final XWindow aWin = xWindow;

        tEnv.addObjRelation("EventProducer",
                            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
            public void fireEvent() {
                aWin.setPosSize(100,100, 500, 500, PosSize.POSSIZE);
            }
        });

        return tEnv;
    }

    /**
    * Sleeps for 3 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(5000);
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e);
        }
    }

    protected void closeDoc(XComponent xDoc) {
        XCloseable closer = UnoRuntime.queryInterface(
                                    XCloseable.class, xDoc);

        try {
            closer.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            log.println("Couldn't close document " + e.getMessage());
        } catch (java.lang.NullPointerException e) {
            log.println("Couldn't close document " + e.getMessage());
        }
    }
}