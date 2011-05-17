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
package mod._toolkit;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.awt.PosSize;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.XExtendedToolkit;
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
import util.DesktopTools;
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
 *  ::com::sun::star::accessibility::XAccessibleExtendedComponent
 *  </code></li>
 *  <li><code>
 *  ::com::sun::star::accessibility::XAccessibleValue</code></li>
 *  <li><code>
 *  ::com::sun::star::accessibility::XAccessibleAction</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see com.sun.star.accessibility.XAccessibleComponent
 * @see com.sun.star.accessibility.XAccessibleExtendedComponent
 * @see com.sun.star.accessibility.XAccessibleValue
 * @see com.sun.star.accessibility.XAccessibleAction
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleContext
 * @see ifc.accessibility._XAccessibleComponent
 * @see ifc.accessibility.XAccessibleExtendedComponent
 * @see ifc.accessibility.XAccessibleValue
 * @see ifc.accessibility.XAccessibleAction
 */
public class AccessibleStatusBar extends TestCase {
    private static XTextDocument xTextDoc;

    /**
     * Disposes the document, if exists, created in
     * <code>createTestEnvironment</code> method.
     */
    protected void cleanup(TestParameters Param, PrintWriter log) {
        log.println("disposing xTextDoc");

        if (xTextDoc != null) {
            util.DesktopTools.closeDoc(xTextDoc);
            ;
        }
    }

    /**
     * Creates a text document.
     * Then obtains an accessible object with
     * the role <code>AccessibleRole.SCROLLBAR</code>.
     * Object relations created :
     * <ul>
     *  <li> <code>'EventProducer'</code> for
     *      {@link ifc.accessibility._XAccessibleEventBroadcaster}:
     *      grabs focus </li>
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

        if (xTextDoc != null) {
            util.DesktopTools.closeDoc(xTextDoc);
        }

        ;

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                     (XMultiServiceFactory) tParam.getMSF());

        XInterface toolkit = null;

        try {
            log.println("creating a text document");
            xTextDoc = SOF.createTextDoc(null);
            toolkit = (XInterface) ((XMultiServiceFactory) tParam.getMSF()).createInstance(
                              "com.sun.star.awt.Toolkit");
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }

        XInterface oObj = null;

        AccessibilityTools at = new AccessibilityTools();

        final XWindow xWindow =
            UnoRuntime.queryInterface(XModel.class, xTextDoc).
            getCurrentController().getFrame().getContainerWindow();

        XAccessible xRoot = at.getAccessibleObject(xWindow);

        oObj = at.getAccessibleObjectForRole(xRoot, AccessibleRole.STATUS_BAR);

        log.println("ImplementationName: " + util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        final XExtendedToolkit tk = (XExtendedToolkit) UnoRuntime.queryInterface(
                                            XExtendedToolkit.class, toolkit);

        tEnv.addObjRelation("EventProducer",
                            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
            public void fireEvent() {
                Rectangle newPosSize = xWindow.getPosSize();
                newPosSize.Width = newPosSize.Width - 20;
                newPosSize.Height = newPosSize.Height - 20;
                newPosSize.X = newPosSize.X + 20;
                newPosSize.Y = newPosSize.Y + 20;
                xWindow.setPosSize(
                    newPosSize.X, newPosSize.Y, newPosSize.Width,
                    newPosSize.Height, PosSize.POSSIZE);
            }
        });

        return tEnv;
    }
}