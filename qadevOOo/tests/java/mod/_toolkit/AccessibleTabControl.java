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
import com.sun.star.accessibility.XAccessibleSelection;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;

import java.io.PrintWriter;

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
 *  ::com::sun::star::accessibility::XAccessibleSelection
 *  </code></li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see com.sun.star.accessibility.XAccessibleComponent
 * @see com.sun.star.accessibility.XAccessibleExtendedComponent
 * @see com.sun.star.accessibility.XAccessibleSelection
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleContext
 * @see ifc.accessibility._XAccessibleComponent
 * @see ifc.accessibility._XAccessibleExtendedComponent
 * @see ifc.accessibility._XAccessibleSelection
 */
public class AccessibleTabControl extends TestCase {
    private static XTextDocument xTextDoc;
    private static XAccessibleAction accCloseButton = null;

    /**
     * Creates the Desktop service (<code>com.sun.star.frame.Desktop</code>).
     */
    @Override
    protected void initialize(TestParameters Param, PrintWriter log) throws Exception {
        UnoRuntime.queryInterface(XDesktop.class,
                                                        DesktopTools.createDesktop(
                                                                Param.getMSF()));
    }

    /**
     * Closes an InsertFields dialog, disposes the document, if exists, created in
     * <code>createTestEnvironment</code> method.
     */
    @Override
    protected void cleanup(TestParameters Param, PrintWriter log) {
        try {
            if (accCloseButton != null) {
                log.println("closing InsertFields Dialog");
                accCloseButton.doAccessibleAction(0);
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
        }

        log.println("disposing xTextDoc");

        if (xTextDoc != null) {
            util.DesktopTools.closeDoc(xTextDoc);
        }
    }

    /**
     * Creates a text document, opens an InsertField dialog.
     * Then obtains an accessible object with
     * the role <code>AccessibleRole.PAGETABLIST</code>.
     * Object relations created :
     * <ul>
     *  <li> <code>'EventProducer'</code> for
     *      {@link ifc.accessibility._XAccessibleEventBroadcaster}:
     *      selects items </li>
     *   <li> <code>'XAccessibleSelection.multiSelection'</code>
     *  of type <code>Boolean</code> for
     *  {@link ifc.accessibility._XAccessibleSelection}:
     *   indicates that component supports single selection mode.</li>
     * </ul>
     *
     * @param tParam test parameters
     * @param log writer to log information while testing
     *
     * @see com.sun.star.awt.Toolkit
     * @see com.sun.star.accessibility.AccessibleRole
     * @see ifc.accessibility._XAccessibleEventBroadcaster
     * @see ifc.accessibility._XAccessibleSelection
     * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
     * @see com.sun.star.accessibility.XAccessibleSelection
     */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters tParam,
                                                    PrintWriter log) throws Exception {
        log.println("creating a test environment");

        if (xTextDoc != null) {
            util.DesktopTools.closeDoc(xTextDoc);
        }

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                     tParam.getMSF());

        log.println("creating a text document");
        xTextDoc = SOF.createTextDoc(null);

        util.utils.waitForEventIdle(tParam.getMSF());

        XModel aModel1 = UnoRuntime.queryInterface(XModel.class,
                                                            xTextDoc);

        XController secondController = aModel1.getCurrentController();

        XDispatchProvider aProv = UnoRuntime.queryInterface(
                                          XDispatchProvider.class,
                                          secondController);

        XURLTransformer urlTransf = null;

        XInterface transf = (XInterface) tParam.getMSF().createInstance(
                                    "com.sun.star.util.URLTransformer");
        urlTransf = UnoRuntime.queryInterface(
                            XURLTransformer.class, transf);

        XDispatch getting = null;
        log.println("opening InsertField dialog");

        URL[] url = new URL[1];
        url[0] = new URL();
        url[0].Complete = ".uno:InsertField";
        urlTransf.parseStrict(url);
        getting = aProv.queryDispatch(url[0], "", 0);

        PropertyValue[] noArgs = new PropertyValue[0];
        getting.dispatch(url[0], noArgs);

        util.utils.waitForEventIdle(tParam.getMSF());

        XInterface oObj = (XInterface) tParam.getMSF().createInstance(
                       "com.sun.star.awt.Toolkit");

        XExtendedToolkit tk = UnoRuntime.queryInterface(
                                      XExtendedToolkit.class, oObj);

        util.utils.waitForEventIdle(tParam.getMSF());

        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class,
                                                              tk.getActiveTopWindow());

        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        oObj = AccessibilityTools.getAccessibleObjectForRole(xRoot,
                                             AccessibleRole.PAGE_TAB_LIST);

        XAccessibleContext closeButton = AccessibilityTools.getAccessibleObjectForRole(xRoot,
                                                                       AccessibleRole.PUSH_BUTTON,
                                                                       "Close");

        accCloseButton = UnoRuntime.queryInterface(
                                 XAccessibleAction.class, closeButton);

        log.println("ImplementationName: " + util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        final XAccessibleSelection selection = UnoRuntime.queryInterface(
                                                                              XAccessibleSelection.class,
                                                                              oObj);

        tEnv.addObjRelation("EventProducer",
                            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
            public void fireEvent() {
                try {
                    selection.selectAccessibleChild(1);
                    selection.selectAccessibleChild(0);
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                }
            }
        });

        tEnv.addObjRelation("XAccessibleSelection.OneAlwaysSelected",
                            Boolean.TRUE);

        tEnv.addObjRelation("XAccessibleSelection.multiSelection",
                            Boolean.FALSE);

        return tEnv;
    }
}
