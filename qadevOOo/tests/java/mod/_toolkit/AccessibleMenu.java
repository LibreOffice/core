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
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleText;
import com.sun.star.awt.XWindow;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.AccessibilityTools;
import util.SOfficeFactory;
import util.utils;


/**
 * Test for object which is represented accessible component of
 * main menu of a document. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleEventBroadcaster</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleAction</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleSelection</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleValue</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleExtendedComponent</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleComponent</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleText</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleContext</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see com.sun.star.accessibility.XAccessibleAction
 * @see com.sun.star.accessibility.XAccessibleSelection
 * @see com.sun.star.accessibility.XAccessibleValue
 * @see com.sun.star.accessibility.XAccessibleExtendedComponent
 * @see com.sun.star.accessibility.XAccessibleComponent
 * @see com.sun.star.accessibility.XAccessibleText
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleAction
 * @see ifc.accessibility._XAccessibleSelection
 * @see ifc.accessibility._XAccessibleValue
 * @see ifc.accessibility._XAccessibleExtendedComponent
 * @see ifc.accessibility._XAccessibleComponent
 * @see ifc.accessibility._XAccessibleText
 * @see ifc.accessibility._XAccessibleContext
 */
public class AccessibleMenu extends TestCase {
    private static XTextDocument xTextDoc = null;
    /**
     * Finds accessible component with role <code>MENU</code>
     * walking through the accessible component tree of a document.
     */
    protected TestEnvironment createTestEnvironment(TestParameters Param,
                                                    PrintWriter log) {
        XMultiServiceFactory msf = (XMultiServiceFactory) Param.getMSF();

        try {
            SOfficeFactory SOF = SOfficeFactory.getFactory(msf);
            xTextDoc = SOF.createTextDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            throw new StatusException("Can't create document", e);
        }

        shortWait();

        XWindow xWindow = UnoRuntime.queryInterface(XModel.class, xTextDoc).
            getCurrentController().getFrame().getContainerWindow();

        AccessibilityTools at = new AccessibilityTools();

        XAccessible xRoot = at.getAccessibleObject(xWindow);

        at.printAccessibleTree(log, xRoot, Param.getBool(util.PropertyName.DEBUG_IS_ACTIVE));
        XAccessibleContext menubar = at.getAccessibleObjectForRole(xRoot,
                                                                   AccessibleRole.MENU_BAR);
        XInterface oObj = null;
        Object menu2 = null;

        try {
            oObj = menubar.getAccessibleChild(2);
            menu2 = menubar.getAccessibleChild(1);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
        }

        log.println("ImplementationName " + utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        final XAccessibleAction act1 = UnoRuntime.queryInterface(
                                               XAccessibleAction.class, oObj);
        final XAccessibleAction act2 = UnoRuntime.queryInterface(
                                               XAccessibleAction.class, menu2);

        tEnv.addObjRelation("EventProducer",
                            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
            public void fireEvent() {
                try {
                    act2.doAccessibleAction(0);

                    try {
                        Thread.sleep(500);
                    } catch (InterruptedException e) {
                    }

                    act1.doAccessibleAction(0);
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                }
            }
        });

        XAccessibleText text = UnoRuntime.queryInterface(
                                       XAccessibleText.class, oObj);

        tEnv.addObjRelation("XAccessibleText.Text", text.getText());

        tEnv.addObjRelation("EditOnly", "Can't change or select Text in Menu");

        tEnv.addObjRelation("LimitedBounds", "yes");

        tEnv.addObjRelation("XAccessibleSelection.OneAlwaysSelected",
                            new Boolean(false));

        tEnv.addObjRelation("XAccessibleSelection.multiSelection",
                            new Boolean(false));

        return tEnv;
    }

    protected void cleanup(TestParameters Param, PrintWriter log) {
        try {
            util.DesktopTools.closeDoc(xTextDoc);
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("Object already disposed");
        }
    }

    /**
     * Creates a new writer document
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
    }

    /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
            log.println("While waiting :" + e);
        }
    }
}