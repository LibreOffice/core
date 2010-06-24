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
import com.sun.star.accessibility.XAccessibleAction;
import com.sun.star.accessibility.XAccessibleText;
import com.sun.star.awt.PosSize;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlContainer;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XFixedText;
import com.sun.star.awt.XWindow;
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
import util.utils;


/**
 * Test for object which is represented by accesible component
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
    private static XTextDocument xTextDoc = null;
    private static XAccessibleAction action = null;
    private static XWindow xWinDlg = null;

    /**
     * Creates a new dialog adds fixed text control to it and
     * displays it. Then the text's accessible component is
     * obtained.
     */
    protected TestEnvironment createTestEnvironment(TestParameters Param,
                                                    PrintWriter log) {
        XInterface oObj = null;
        XMultiServiceFactory xMSF = (XMultiServiceFactory) Param.getMSF();
        XControlModel dlgModel = null;

        XControl txtControl = null;
        XControlModel txtModel = null;

        try {
            dlgModel = (XControlModel) UnoRuntime.queryInterface(
                               XControlModel.class,
                               xMSF.createInstance(
                                       "com.sun.star.awt.UnoControlDialogModel"));

            XControl dlgControl = (XControl) UnoRuntime.queryInterface(
                                          XControl.class,
                                          xMSF.createInstance(
                                                  "com.sun.star.awt.UnoControlDialog"));

            dlgControl.setModel(dlgModel);

            txtModel = (XControlModel) UnoRuntime.queryInterface(
                               XControlModel.class,
                               xMSF.createInstance(
                                       "com.sun.star.awt.UnoControlFixedTextModel"));

            txtControl = (XControl) UnoRuntime.queryInterface(XControl.class,
                                                              xMSF.createInstance(
                                                                      "com.sun.star.awt.UnoControlFixedText"));

            txtControl.setModel(txtModel);

            XFixedText xFT = (XFixedText) UnoRuntime.queryInterface(
                                     XFixedText.class, txtControl);
            xFT.setText("FxedText");

            XControlContainer ctrlCont = (XControlContainer) UnoRuntime.queryInterface(
                                                 XControlContainer.class,
                                                 dlgControl);

            ctrlCont.addControl("Text", txtControl);

            xWinDlg = (XWindow) UnoRuntime.queryInterface(XWindow.class,
                                                          dlgControl);

            xWinDlg.setVisible(true);

            xWinDlg.setPosSize(0, 0, 200, 100, PosSize.SIZE);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Error creating dialog :");
            e.printStackTrace(log);
        }

        try {
            oObj = (XInterface) ((XMultiServiceFactory) Param.getMSF()).createInstance(
                           "com.sun.star.awt.Toolkit");
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get toolkit");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get toolkit", e);
        }

        shortWait();

        AccessibilityTools at = new AccessibilityTools();

        XAccessible xRoot = at.getAccessibleObject(xWinDlg);

        at.printAccessibleTree(log, xRoot, Param.getBool(util.PropertyName.DEBUG_IS_ACTIVE));

        oObj = at.getAccessibleObjectForRole(xRoot, AccessibleRole.LABEL);

        log.println("ImplementationName " + utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        final XWindow xWin = (XWindow) UnoRuntime.queryInterface(XWindow.class,
                                                                 txtControl);

        tEnv.addObjRelation("EventProducer",
                            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
            public void fireEvent() {
                xWin.setEnable(false);
                xWin.setEnable(true);
            }
        });

        XAccessibleText text = (XAccessibleText) UnoRuntime.queryInterface(
                                       XAccessibleText.class, oObj);

        tEnv.addObjRelation("XAccessibleText.Text", text.getText());

        tEnv.addObjRelation("EditOnly",
                            "This method isn't supported in this component");

        tEnv.addObjRelation("LimitedBounds", "yes");

        return tEnv;
    }

    /**
     * Closes dialog using action of button 'Close'
     */
    protected void cleanup(TestParameters Param, PrintWriter log) {
        log.println("    Closing dialog ... ");
        xWinDlg.dispose();
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