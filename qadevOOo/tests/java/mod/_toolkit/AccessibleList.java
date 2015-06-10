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
import com.sun.star.accessibility.XAccessibleSelection;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XController;
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

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.SOfficeFactory;
import util.utils;


public class AccessibleList extends TestCase {
    private static XTextDocument xTextDoc = null;
    private static XAccessibleAction action = null;
    private static XMultiServiceFactory msf = null;

    /**
     * Opens 'Insert Hyperlink' dialog using document dispatch provider.
     * Finds active top window (the dialog
     * window) and finds button 'Close' (for closing this dialog when
     * disposing) walking through the accessible component tree.
     * Then the TREE component is found and the 'New Document' tab is
     * selected to make list box visible. After that list box is obtained.
     */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param,
                                                    PrintWriter log) throws Exception {
        XInterface oObj = (XInterface) msf.createInstance("com.sun.star.awt.Toolkit");

        XExtendedToolkit tk = UnoRuntime.queryInterface(
                                      XExtendedToolkit.class, oObj);

        util.utils.waitForEventIdle(Param.getMSF());

        XModel aModel1 = UnoRuntime.queryInterface(XModel.class,
                                                            xTextDoc);

        XController secondController = aModel1.getCurrentController();

        XDispatchProvider aProv = UnoRuntime.queryInterface(
                                          XDispatchProvider.class,
                                          secondController);

        XURLTransformer urlTransf = null;

        XInterface transf = (XInterface) msf.createInstance(
                                    "com.sun.star.util.URLTransformer");
        urlTransf = UnoRuntime.queryInterface(
                            XURLTransformer.class, transf);

        XDispatch getting = null;
        log.println("opening HyperlinkDialog");

        URL[] url = new URL[1];
        url[0] = new URL();
        url[0].Complete = ".uno:HyperlinkDialog";
        urlTransf.parseStrict(url);
        getting = aProv.queryDispatch(url[0], "", 0);

        PropertyValue[] noArgs = new PropertyValue[0];
        getting.dispatch(url[0], noArgs);

        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class,
                                                              tk.getActiveTopWindow());

        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);


        AccessibilityTools.printAccessibleTree(log, xRoot, Param.getBool(util.PropertyName.DEBUG_IS_ACTIVE));
        // obtaining 'Close' button
        oObj = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.PUSH_BUTTON,
                                             "Close");
        action = UnoRuntime.queryInterface(
                         XAccessibleAction.class, oObj);

        // Selecting 'New Document' tab
        oObj = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.TREE);

        XAccessibleSelection xAccSel = UnoRuntime.queryInterface(
                                               XAccessibleSelection.class,
                                               oObj);
        xAccSel.selectAccessibleChild(3);
        util.utils.waitForEventIdle(Param.getMSF());

        log.println("# Getting the ListBox");

        XAccessibleContext parent = AccessibilityTools.getAccessibleObjectForRole(xRoot,
                                                                  AccessibleRole.PANEL,
                                                                  "",
                                                                  "com.sun.star.comp.toolkit.AccessibleListBox");

        log.println("# Getting the first child");

        try {
            oObj = parent.getAccessibleChild(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
        }

        log.println("ImplementationName " + utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("XAccessibleSelection.multiSelection",
                            Boolean.FALSE);

        final XAccessibleComponent acomp = UnoRuntime.queryInterface(
                                                   XAccessibleComponent.class,
                                                   oObj);
        final XAccessibleComponent acomp1 = UnoRuntime.queryInterface(
                                                    XAccessibleComponent.class,
                                                    action);

        tEnv.addObjRelation("EventProducer",
                            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
            public void fireEvent() {
                System.out.println("Grabbing focus ... ");
                acomp1.grabFocus();
                acomp.grabFocus();
            }
        });

        return tEnv;
    }

    /**
     * Closes dialog using action of button 'Close'
     */
    @Override
    protected void cleanup(TestParameters Param, PrintWriter log) {
        log.println("    Closing dialog ... ");

        try {
            action.doAccessibleAction(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException ioe) {
            log.println("Couldn't close dialog");
        } catch (com.sun.star.lang.DisposedException de) {
            log.println("Dialog already disposed");
        }

        util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
     * Creates writer document
     */
    @Override
    protected void initialize(TestParameters Param, PrintWriter log) throws Exception {
        msf = Param.getMSF();

        SOfficeFactory SOF = SOfficeFactory.getFactory(msf);
        xTextDoc = SOF.createTextDoc(null);
    }
}