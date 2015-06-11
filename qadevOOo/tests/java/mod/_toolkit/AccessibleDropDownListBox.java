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
package mod._toolkit;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleAction;
import com.sun.star.awt.XWindow;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.AccessibilityTools;
import util.DesktopTools;
import util.SOfficeFactory;
import util.utils;


/**
 * Test for object which is represented by style list box
 * on toolbar of a document. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleExtendedComponent</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleEventBroadcaster</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleComponent</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleAction</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleContext</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleExtendedComponent
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see com.sun.star.accessibility.XAccessibleComponent
 * @see com.sun.star.accessibility.XAccessibleAction
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see ifc.accessibility._XAccessibleExtendedComponent
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleComponent
 * @see ifc.accessibility._XAccessibleAction
 * @see ifc.accessibility._XAccessibleContext
 */
public class AccessibleDropDownListBox extends TestCase {
    private static XTextDocument xTextDoc = null;
    /**
     * Finds AccessibleDropDownListBox walking through the
     * accessible component tree of a writer document.
     */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param,
                                                    PrintWriter log) {
        XInterface oObj = null;

        util.utils.waitForEventIdle(Param.getMSF());

        XWindow xWindow = AccessibilityTools.getCurrentWindow(
                                  xTextDoc);

        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        AccessibilityTools.printAccessibleTree(log, xRoot, Param.getBool(util.PropertyName.DEBUG_IS_ACTIVE));

        oObj = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.COMBO_BOX,
                                             "", "AccessibleDropDownListBox");

        log.println("ImplementationName " + utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        final XAccessibleAction acomp = UnoRuntime.queryInterface(
                                                XAccessibleAction.class, oObj);
        tEnv.addObjRelation("EventProducer",
                            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
            public void fireEvent() {
                try {
                    acomp.doAccessibleAction(0);
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    throw new StatusException("Can't perform action 0", e);
                }
            }
        });

        return tEnv;
    }

    /**
     * Creates writer document.
     */
    @Override
    protected void initialize(TestParameters Param, PrintWriter log) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                     Param.getMSF());
        // Dock the Navigator to supply an AccessibleDropDownListBox
        DesktopTools.dockNavigator(Param.getMSF());
        xTextDoc = SOF.createTextDoc(null);
    }

    /**
     * Disposes writer document.
     */
    @Override
    protected void cleanup(TestParameters Param, PrintWriter log) {
        util.DesktopTools.closeDoc(xTextDoc);
    }
}
