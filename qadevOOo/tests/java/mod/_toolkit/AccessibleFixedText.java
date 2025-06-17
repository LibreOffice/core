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
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleText;
import com.sun.star.awt.PosSize;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlContainer;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XFixedText;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XLayoutConstrains;
import com.sun.star.awt.Size;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.utils;


/**
 * Test for object which is represented by accessible component
 * of the fixed text label in 'Hyperlink' Dialog. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleExtendedComponent</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleEventBroadcaster</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleComponent</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleContext</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleText</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleExtendedComponent
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see com.sun.star.accessibility.XAccessibleComponent
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see com.sun.star.accessibility.XAccessibleText
 * @see ifc.accessibility._XAccessibleExtendedComponent
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleComponent
 * @see ifc.accessibility._XAccessibleContext
 * @see ifc.accessibility._XAccessibleText
 */
public class AccessibleFixedText extends TestCase {
    private static XWindow xWinDlg = null;

    /**
     * Creates a new dialog adds fixed text control to it and
     * displays it. Then the text's accessible component is
     * obtained.
     */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param,
                                                    PrintWriter log) throws Exception {
        XMultiServiceFactory xMSF = Param.getMSF();
        XControlModel dlgModel = null;

        XControl txtControl = null;
        XControlModel txtModel = null;

        try {
            dlgModel = UnoRuntime.queryInterface(
                               XControlModel.class,
                               xMSF.createInstance(
                                       "com.sun.star.awt.UnoControlDialogModel"));

            XControl dlgControl = UnoRuntime.queryInterface(
                                          XControl.class,
                                          xMSF.createInstance(
                                                  "com.sun.star.awt.UnoControlDialog"));

            dlgControl.setModel(dlgModel);

            txtModel = UnoRuntime.queryInterface(
                               XControlModel.class,
                               xMSF.createInstance(
                                       "com.sun.star.awt.UnoControlFixedTextModel"));

            txtControl = UnoRuntime.queryInterface(XControl.class,
                                                              xMSF.createInstance(
                                                                      "com.sun.star.awt.UnoControlFixedText"));

            txtControl.setModel(txtModel);

            XFixedText xFT = UnoRuntime.queryInterface(
                                     XFixedText.class, txtControl);
            xFT.setText("FxedText");

            /* Set the text control to its preferred size, otherwise it
             * defaults to the size hard coded in its constructor (100 x 12) */
            XLayoutConstrains xLCTxt = UnoRuntime.queryInterface(
                XLayoutConstrains.class, txtControl);
            Size textSize = xLCTxt.getPreferredSize();
            XWindow xWinTxt = UnoRuntime.queryInterface(
                XWindow.class, txtControl);
            xWinTxt.setPosSize(0, 0, textSize.Width, textSize.Height,
                               PosSize.SIZE);

            XControlContainer ctrlCont = UnoRuntime.queryInterface(
                                                 XControlContainer.class,
                                                 dlgControl);

            ctrlCont.addControl("Text", txtControl);

            xWinDlg = UnoRuntime.queryInterface(XWindow.class,
                                                          dlgControl);

            xWinDlg.setVisible(true);

            xWinDlg.setPosSize(0, 0, 200, 100, PosSize.SIZE);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Error creating dialog :");
            e.printStackTrace(log);
        }

        util.utils.waitForEventIdle(Param.getMSF());

        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWinDlg);

        AccessibilityTools.printAccessibleTree(log, xRoot, Param.getBool(util.PropertyName.DEBUG_IS_ACTIVE));

        XAccessibleContext xLabel = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.LABEL);

        log.println("ImplementationName " + utils.getImplName(xLabel));

        TestEnvironment tEnv = new TestEnvironment(xLabel);

        final XWindow xWin = UnoRuntime.queryInterface(XWindow.class,
                                                                 txtControl);

        tEnv.addObjRelation("EventProducer",
                            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
            public void fireEvent() {
                xWin.setEnable(false);
                xWin.setEnable(true);
            }
        });

        XAccessibleText text = UnoRuntime.queryInterface(
                                       XAccessibleText.class, xLabel);

        tEnv.addObjRelation("XAccessibleText.Text", text.getText());

        tEnv.addObjRelation("EditOnly",
                            "This method isn't supported in this component");

        tEnv.addObjRelation("LimitedBounds", "yes");

        return tEnv;
    }

    /**
     * Closes dialog using action of button 'Close'
     */
    @Override
    protected void cleanup(TestParameters Param, PrintWriter log) {
        log.println("    Closing dialog ... ");
        if (xWinDlg != null)
            xWinDlg.dispose();
    }
}
