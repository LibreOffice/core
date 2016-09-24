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

package mod._sc;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleAction;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleText;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class ScAccessibleCsvRuler extends TestCase {

    static XAccessibleAction accAction = null;

    /**
     * Called to create an instance of <code>TestEnvironment</code> with an
     * object to test and related objects. Subclasses should implement this
     * method to provide the implementation and related objects. The method is
     * called from <code>getTestEnvironment()</code>.
     *
     * @param Param test parameters
     * @param log writer to log information while testing
     *
     * @see TestEnvironment
     * @see #getTestEnvironment
     */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param,
                                                    PrintWriter log) throws Exception {
        XMultiServiceFactory msf = Param.getMSF();

        XInterface oObj = (XInterface) msf.createInstance("com.sun.star.awt.Toolkit");

        XExtendedToolkit tk = UnoRuntime.queryInterface(
                                      XExtendedToolkit.class, oObj);

        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class,
                                                              tk.getActiveTopWindow());

        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        oObj = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.PUSH_BUTTON,
                                             "Cancel");

        accAction = UnoRuntime.queryInterface(
                            XAccessibleAction.class, oObj);

        XAccessibleContext acc = AccessibilityTools.getAccessibleObjectForRole(xRoot,
                                                               AccessibleRole.RADIO_BUTTON);

        log.println("Click on: " + acc.getAccessibleName());

        XAccessibleAction accAction2 = UnoRuntime.queryInterface(
                                               XAccessibleAction.class, acc);

        try {
            accAction2.doAccessibleAction(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException iae) {
        }


        AccessibilityTools.printAccessibleTree(log, xRoot);
        oObj = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.TEXT,
                                             "Ruler", true);

        log.println("ImplementationName " + utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("EditOnly",
                            "This method isn't supported in this dialog");

        XAccessibleText text = UnoRuntime.queryInterface(
                                       XAccessibleText.class, oObj);

        int lastone = 100;

        for (int i = 0; i < 1000; i++) {
            try {
                text.getCharacterBounds(i);
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                lastone = i - 1;

                break;
            }
        }

        tEnv.addObjRelation("LimitedBounds", Integer.valueOf(lastone));
        tEnv.addObjRelation("PreviousUsed",new int[]{11,22,33,44,55,66,77,88,99});

        return tEnv;
    }

    /**
    * Called while disposing a <code>TestEnvironment</code>.
    * Disposes calc document.
    * @param Param test parameters
    * @param log writer to log information while testing
    */
    @Override
    protected void cleanup( TestParameters Param, PrintWriter log) {
    }

    /**
     * Called while the <code>TestCase</code> initialization. In the
     * implementation does nothing. Subclasses can override to initialize
     * objects shared among all <code>TestEnvironment</code>s.
     *
     * @param Param test parameters
     * @param log writer to log information while testing
     *
     * @see #initializeTestCase
     */
    @Override
    protected void initialize(TestParameters Param, PrintWriter log) throws Exception {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( Param.getMSF());

        log.println("opening dialog");

        PropertyValue[] args = new PropertyValue[1];
        try {
            args[0] = new PropertyValue();
            args[0].Name = "InteractionHandler";
            args[0].Value = Param.getMSF().createInstance(
                "com.sun.star.comp.uui.UUIInteractionHandler");
        } catch(com.sun.star.uno.Exception e) {
        }

        String url= utils.getFullTestURL("10test.csv");
        log.println("loading "+url);
        SOF.loadDocument(url,args);
    }

}
