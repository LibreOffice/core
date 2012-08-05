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
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.Point;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;


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
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see com.sun.star.accessibility.XAccessibleComponent
 * @see com.sun.star.accessibility.XAccessibleExtendedComponent
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleContext
 * @see ifc.accessibility._XAccessibleComponent
 * @see ifc.accessibility.XAccessibleExtendedComponent
 */
public class AccessibleTabPage extends TestCase {
    private static XTextDocument xTextDoc;
    private static XAccessibleAction accCloseButton = null;

    /**
     * Creates the Desktop service (<code>com.sun.star.frame.Desktop</code>).
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        UnoRuntime.queryInterface(XDesktop.class,
                                                        DesktopTools.createDesktop(
                                                                (XMultiServiceFactory) Param.getMSF()));
    }

    /**
     * Closes an InsertFields dialog, disposes the document, if exists, created in
     * <code>createTestEnvironment</code> method.
     */
    protected void cleanup(TestParameters Param, PrintWriter log) {
        try {
            if (accCloseButton != null) {
                log.println("closing InsertFields Dialog");
                accCloseButton.doAccessibleAction(0);
                shortWait();
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("Couldn't close dialog " + e.getMessage());
        }

        log.println("disposing xTextDoc");

        if (xTextDoc != null) {
            closeDoc();
        }
    }

    /**
     * Creates a text document, opens an InsertField dialog.
     * Then obtains an accessible object with
     * the role <code>AccessibleRole.PAGETAB</code>.
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

        try {
            if (accCloseButton != null) {
                log.println("closing InsertFields Dialog");
                accCloseButton.doAccessibleAction(0);
                shortWait();
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("Couldn't close dialog " + e.getMessage());
        }

        log.println("disposing xTextDoc");

        if (xTextDoc != null) {
            closeDoc();
        }

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                     (XMultiServiceFactory) tParam.getMSF());

        try {
            log.println("creating a text document");
            xTextDoc = SOF.createTextDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }

        shortWait();

        XModel aModel1 = UnoRuntime.queryInterface(XModel.class,
                                                            xTextDoc);

        XController secondController = aModel1.getCurrentController();

        XDispatchProvider aProv = UnoRuntime.queryInterface(
                                          XDispatchProvider.class,
                                          secondController);

        XURLTransformer urlTransf = null;

        try {
            XInterface transf = (XInterface) ((XMultiServiceFactory) tParam.getMSF()).createInstance(
                                        "com.sun.star.util.URLTransformer");
            urlTransf = UnoRuntime.queryInterface(
                                XURLTransformer.class, transf);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create URLTransformer", e);
        }

        XDispatch getting = null;
        log.println("opening InsertField dialog");

        URL[] url = new URL[1];
        url[0] = new URL();
        url[0].Complete = ".uno:InsertField";
        urlTransf.parseStrict(url);
        getting = aProv.queryDispatch(url[0], "", 0);

        PropertyValue[] noArgs = new PropertyValue[0];
        getting.dispatch(url[0], noArgs);

        shortWait();

        XInterface oObj = null;

        try {
            oObj = (XInterface) ((XMultiServiceFactory) tParam.getMSF()).createInstance(
                           "com.sun.star.awt.Toolkit");
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get toolkit");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get toolkit", e);
        }

        XExtendedToolkit tk = UnoRuntime.queryInterface(
                                      XExtendedToolkit.class, oObj);

        AccessibilityTools at = new AccessibilityTools();

        shortWait();

        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class,
                                                              tk.getActiveTopWindow());

        XAccessible xRoot = at.getAccessibleObject(xWindow);

        XAccessibleContext closeButton = at.getAccessibleObjectForRole(xRoot,
                                                                       AccessibleRole.PUSH_BUTTON,
                                                                       "Close");

        accCloseButton = UnoRuntime.queryInterface(
                                 XAccessibleAction.class, closeButton);

        oObj = at.getAccessibleObjectForRole(xRoot, AccessibleRole.PAGE_TAB);

        log.println("ImplementationName: " + util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("EditOnly", "toolkit.AccessibleTabPage");
        tEnv.addObjRelation("LimitedBounds", "toolkit.AccessibleTabPage");

        XAccessibleComponent accComp = UnoRuntime.queryInterface(
                                               XAccessibleComponent.class,
                                               oObj);
        accComp.getLocationOnScreen();

        shortWait();

        XInterface xEventInt = at.getAccessibleObjectForRole(xRoot, AccessibleRole.PAGE_TAB, "Variables");
        final XAccessibleComponent eventAccComp = UnoRuntime.queryInterface(
                                               XAccessibleComponent.class,
                                               xEventInt);

        tEnv.addObjRelation("EventProducer",
                            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
            public void fireEvent() {
               eventAccComp.grabFocus();
            }
        });

        return tEnv;
    }

    /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e);
        }
    }

    protected void closeDoc() {
        util.DesktopTools.closeDoc(xTextDoc);
    }
}
