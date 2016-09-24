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
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.SOfficeFactory;
import util.utils;


/**
 * Test for object which is represented by accessible component
 * of a menu item in main menu of a document. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleExtendedComponent</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleEventBroadcaster</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleComponent</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleValue</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleAction</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleContext</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleText</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleExtendedComponent
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see com.sun.star.accessibility.XAccessibleComponent
 * @see com.sun.star.accessibility.XAccessibleValue
 * @see com.sun.star.accessibility.XAccessibleAction
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see com.sun.star.accessibility.XAccessibleText
 * @see ifc.accessibility._XAccessibleExtendedComponent
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleComponent
 * @see ifc.accessibility._XAccessibleValue
 * @see ifc.accessibility._XAccessibleAction
 * @see ifc.accessibility._XAccessibleContext
 * @see ifc.accessibility._XAccessibleText
 */
public class AccessibleMenuItem extends TestCase {
    private static XTextDocument xTextDoc = null;
    private static XMultiServiceFactory msf = null;

    /**
     * Finds first accessible component with role <code>MENUITEM</code>
     * walking through the accessible component tree of a document.
     */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param,
                                                    PrintWriter log) throws Exception {
        XInterface oObj = (XInterface) msf.createInstance("com.sun.star.awt.Toolkit");

        XExtendedToolkit tk = UnoRuntime.queryInterface(
                                      XExtendedToolkit.class, oObj);

        util.utils.waitForEventIdle(Param.getMSF());

        Object atw = tk.getActiveTopWindow();

        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class,
                                                              atw);

        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        AccessibilityTools.printAccessibleTree(log, xRoot, Param.getBool(util.PropertyName.DEBUG_IS_ACTIVE));
        XAccessibleContext MenuBar = AccessibilityTools.getAccessibleObjectForRole(xRoot,
                                                                   AccessibleRole.MENU_BAR);

        try {
            //activate Edit-Menu
            XAccessible Menu = MenuBar.getAccessibleChild(1);
            XAccessibleAction act = UnoRuntime.queryInterface(
                                            XAccessibleAction.class, Menu);
            act.doAccessibleAction(0);
            util.utils.waitForEventIdle(Param.getMSF());

            //get a menu-item
            oObj = Menu.getAccessibleContext().getAccessibleChild(11);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
        }


        log.println("ImplementationName " + utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        final XAccessibleAction action = UnoRuntime.queryInterface(
                                                 XAccessibleAction.class, oObj);

        tEnv.addObjRelation("EventProducer",
                            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
            public void fireEvent() {
                try {
                    action.doAccessibleAction(0);
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                }
            }
        });

        XAccessibleText text = UnoRuntime.queryInterface(
                                       XAccessibleText.class, oObj);

        tEnv.addObjRelation("XAccessibleText.Text", text.getText());

        tEnv.addObjRelation("EditOnly",
                            "Can't change or select Text in MenuBarItem");

        tEnv.addObjRelation("Destroy", Boolean.TRUE);

        tEnv.addObjRelation("LimitedBounds", "yes");

        return tEnv;
    }

    /**
     * Creates writer document.
     */
    @Override
    protected void initialize(TestParameters Param, PrintWriter log) throws Exception {
        msf = Param.getMSF();

        SOfficeFactory SOF = SOfficeFactory.getFactory(msf);
        xTextDoc = SOF.createTextDoc(null);
    }

    /**
     * Disposes document.
     */
    @Override
    protected void cleanup(TestParameters Param, PrintWriter log) {
        util.DesktopTools.closeDoc(xTextDoc);
    }
}