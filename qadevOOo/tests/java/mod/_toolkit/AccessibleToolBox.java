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
import com.sun.star.awt.XWindow;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.AccessibilityTools;
import util.SOfficeFactory;

/**
 * Test for object that implements the following interfaces :
 * <ul>
 *  <li><code>
 *  ::com::sun::star::accessibility::XAccessibleContext</code></li>
 *  <li><code>
 *  ::com::sun::star::accessibility::XAccessibleEventBroadcaster
 *  </code></li>
 *  <li><code>
 *  ::com::sun::star::accessibility::XAccessibleComponent</code></li>
 *  <li><code>
 *  ::com::sun::star::accessibility::XAccessibleExtendedComponent</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see com.sun.star.accessibility.XAccessibleComponent
 * @see com.sun.star.accessibility.XAccessibleExtendedComponent
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleContext
 * @see ifc.accessibility._XAccessibleComponent
 * @see ifc.accessibility._XAccessibleExtendedComponent
 */
public class AccessibleToolBox extends TestCase {
    private XTextDocument xTextDoc;

    /**
     * Disposes the document, if exists, created in
     * <code>createTestEnvironment</code> method.
     */
    @Override
    protected void cleanup(TestParameters Param, PrintWriter log) {
        log.println("disposing xTextDoc");

        if (xTextDoc != null) {
            util.DesktopTools.closeDoc(xTextDoc);
        }
    }

    /**
     * Creates a text document.
     * Then obtains an accessible object with
     * the role <code>AccessibleRole.TOOLBAR</code>.
     * Object relations created :
     * <ul>
     *  <li> <code>'EventProducer'</code> for
     *      {@link ifc.accessibility._XAccessibleEventBroadcaster}:
     *      grabs focus
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

        if (xTextDoc != null) {
            util.DesktopTools.closeDoc(xTextDoc);
        }

        XMultiServiceFactory msf = tParam.getMSF();

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(msf);

        log.println("creating a text document");
        xTextDoc = SOF.createTextDoc(null);

        XWindow xWindow = UnoRuntime.queryInterface(XModel.class, xTextDoc).
            getCurrentController().getFrame().getContainerWindow();

        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        AccessibilityTools.printAccessibleTree(log, xRoot, tParam.getBool(util.PropertyName.DEBUG_IS_ACTIVE));

        XAccessibleContext xAccContext = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.TOOL_BAR);

        log.println("ImplementationName: " + util.utils.getImplName(xAccContext));

        TestEnvironment tEnv = new TestEnvironment(xAccContext);

        tEnv.addObjRelation("LimitedBounds", "yes");

        XAccessible child = null;

        try {
            child = xAccContext.getAccessibleChild(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
        }

        util.dbg.printInterfaces(child);

        final XAccessibleAction action = UnoRuntime.queryInterface(
                XAccessibleAction.class,
                child);

        tEnv.addObjRelation("EventProducer",
                new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
            public void fireEvent() {
                try {
                    action.doAccessibleAction(0);
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    System.out.println("Couldn't fire event");
                }
            }
        });

        return tEnv;
    }
}
