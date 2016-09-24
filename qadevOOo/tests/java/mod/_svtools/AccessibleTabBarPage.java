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

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.Point;

import java.awt.Robot;
import java.awt.event.InputEvent;
import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.SOfficeFactory;

import com.sun.star.accessibility.XAccessible;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

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
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleExtendedComponent
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see com.sun.star.accessibility.XAccessibleComponent
 * @see ifc.accessibility._XAccessibleComponent
 * @see ifc.accessibility._XAccessibleExtendedComponent
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleContext
 */
public class AccessibleTabBarPage extends TestCase {

    static XComponent xDoc;

    /**
     * Disposes the document, if exists, created in
     * <code>createTestEnvironment</code> method.
     */
    @Override
    protected void cleanup( TestParameters Param, PrintWriter log) {

        log.println("disposing xCalcDoc");
        if (xDoc != null) {
            util.DesktopTools.closeDoc(xDoc);
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
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters tParam,
        PrintWriter log) throws Exception {

        log.println( "creating a test environment" );

        if (xDoc != null) xDoc.dispose();

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF());

        log.println( "creating a calc document" );
        xDoc = UnoRuntime.queryInterface(XComponent.class, SOF.createCalcDoc(null));// SOF.createDrawDoc(null);

        util.utils.waitForEventIdle(tParam.getMSF());

        XInterface oObj = (XInterface) tParam.getMSF().createInstance
                ("com.sun.star.awt.Toolkit") ;


        XExtendedToolkit tk = UnoRuntime.queryInterface(XExtendedToolkit.class, oObj);


        util.utils.waitForEventIdle(tParam.getMSF());

        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class,tk.getActiveTopWindow());

        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);
        AccessibilityTools.printAccessibleTree(log, xRoot, tParam.getBool(util.PropertyName.DEBUG_IS_ACTIVE));
        oObj = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.PAGE_TAB, "Sheet1");
        XAccessibleContext acc = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.PAGE_TAB, "Sheet2");
        XAccessibleComponent accComp = UnoRuntime.queryInterface(
                                               XAccessibleComponent.class,
                                               acc);
        final Point point = accComp.getLocationOnScreen();
        log.println("ImplementationName: " + util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment( oObj );
        tEnv.addObjRelation("Destroy","yes");
        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer(){
                public void fireEvent() {
                try {
                    Robot rob = new Robot();
                    rob.mouseMove(point.X + 25, point.Y + 5);
                    rob.mousePress(InputEvent.BUTTON1_MASK);
                } catch (java.awt.AWTException e) {
                    System.out.println("couldn't fire event");
                }
                }
            });

        return tEnv;
    }

}
