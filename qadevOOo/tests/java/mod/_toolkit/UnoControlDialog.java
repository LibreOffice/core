/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: UnoControlDialog.java,v $
 * $Revision: 1.9 $
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

import com.sun.star.awt.PosSize;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlContainer;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XDevice;
import com.sun.star.awt.XGraphics;
import com.sun.star.awt.XTabController;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.SOfficeFactory;


/**
* Test for object which is represented by service
* <code>com.sun.star.awt.UnoControlDialog</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::lang::XComponent</code></li>
*  <li> <code>com::sun::star::awt::XWindow</code></li>
*  <li> <code>com::sun::star::awt::XDialog</code></li>
*  <li> <code>com::sun::star::awt::XControl</code></li>
*  <li> <code>com::sun::star::awt::XTopWindow</code></li>
*  <li> <code>com::sun::star::awt::XControlContainer</code></li>
*  <li> <code>com::sun::star::awt::XView</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.lang.XComponent
* @see com.sun.star.awt.XWindow
* @see com.sun.star.awt.XDialog
* @see com.sun.star.awt.XControl
* @see com.sun.star.awt.XTopWindow
* @see com.sun.star.awt.XControlContainer
* @see com.sun.star.awt.XView
* @see ifc.lang._XComponent
* @see ifc.awt._XWindow
* @see ifc.awt._XDialog
* @see ifc.awt._XControl
* @see ifc.awt._XTopWindow
* @see ifc.awt._XControlContainer
* @see ifc.awt._XView
*/
public class UnoControlDialog extends TestCase {
    private static XWindow xWinDlg = null;
    private static XTextDocument xTextDoc;

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates a Dialog Control and Model instance defines Model
    * for Control, adds to Dialog a button, sets its size and
    * sets the dialog visible. <p>
    */
    public synchronized TestEnvironment createTestEnvironment(TestParameters Param,
                                                              PrintWriter log) {
        XInterface oObj = null;
        XMultiServiceFactory xMSF = (XMultiServiceFactory) Param.getMSF();
        XControlModel dlgModel = null;

        XWindowPeer the_win = null;
        XToolkit the_kit = null;
        XDevice aDevice = null;
        XGraphics aGraphic = null;

        XControl butControl = null;
        XControl butControl1 = null;
        XControl butControl2 = null;
        XTabController tabControl1 = null;
        XTabController tabControl2 = null;

        XControlContainer ctrlCont = null;

        if (xWinDlg != null) {
            xWinDlg.dispose();
        }

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

            XControlModel butModel = (XControlModel) UnoRuntime.queryInterface(
                                             XControlModel.class,
                                             xMSF.createInstance(
                                                     "com.sun.star.awt.UnoControlButtonModel"));

            butControl = (XControl) UnoRuntime.queryInterface(XControl.class,
                                                              xMSF.createInstance(
                                                                      "com.sun.star.awt.UnoControlButton"));

            butControl.setModel(butModel);


            // creating additional controls for XUnoControlContainer
            tabControl1 = (XTabController) UnoRuntime.queryInterface(
                                  XTabController.class,
                                  xMSF.createInstance(
                                          "com.sun.star.awt.TabController"));

            tabControl2 = (XTabController) UnoRuntime.queryInterface(
                                  XTabController.class,
                                  xMSF.createInstance(
                                          "com.sun.star.awt.TabController"));


            // creating additional controls for XControlContainer
            butModel = (XControlModel) UnoRuntime.queryInterface(
                               XControlModel.class,
                               xMSF.createInstance(
                                       "com.sun.star.awt.UnoControlButtonModel"));

            butControl1 = (XControl) UnoRuntime.queryInterface(XControl.class,
                                                               xMSF.createInstance(
                                                                       "com.sun.star.awt.UnoControlButton"));

            butControl1.setModel(butModel);

            butModel = (XControlModel) UnoRuntime.queryInterface(
                               XControlModel.class,
                               xMSF.createInstance(
                                       "com.sun.star.awt.UnoControlButtonModel"));

            butControl2 = (XControl) UnoRuntime.queryInterface(XControl.class,
                                                               xMSF.createInstance(
                                                                       "com.sun.star.awt.UnoControlButton"));

            butControl2.setModel(butModel);

            ctrlCont = (XControlContainer) UnoRuntime.queryInterface(
                               XControlContainer.class, dlgControl);

            xWinDlg = (XWindow) UnoRuntime.queryInterface(XWindow.class,
                                                          dlgControl);

            xWinDlg.setVisible(true);

            xWinDlg.setPosSize(10, 10, 220, 110, PosSize.SIZE);

            the_win = dlgControl.getPeer();
            the_kit = the_win.getToolkit();
            aDevice = the_kit.createScreenCompatibleDevice(220, 220);
            aGraphic = aDevice.createGraphics();

            oObj = dlgControl;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Error creating dialog :");
            e.printStackTrace(log);
        }

        log.println("creating a new environment for object");

        TestEnvironment tEnv = new TestEnvironment(oObj);


        //Adding ObjRelation for XView
        tEnv.addObjRelation("GRAPHICS", aGraphic);


        //Adding ObjRelation for XControl
        tEnv.addObjRelation("CONTEXT", dlgModel);
        tEnv.addObjRelation("WINPEER", the_win);
        tEnv.addObjRelation("TOOLKIT", the_kit);
        tEnv.addObjRelation("MODEL", dlgModel);

        tEnv.addObjRelation("INSTANCE", butControl);
        tEnv.addObjRelation("XContainer.Container", ctrlCont);


        // adding relations for XUnoControlContainer
        tEnv.addObjRelation("TABCONTROL1", tabControl1);
        tEnv.addObjRelation("TABCONTROL2", tabControl2);


        // adding relations for XControlContainer
        tEnv.addObjRelation("CONTROL1", butControl1);
        tEnv.addObjRelation("CONTROL2", butControl2);

        XWindow forObjRel = xTextDoc.getCurrentController().getFrame()
                                    .getComponentWindow();
        tEnv.addObjRelation("XWindow.AnotherWindow", forObjRel);

        return tEnv;
    } // finish method getTestEnvironment

    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("Disposing dialog ...");
        xWinDlg.dispose();
        log.println("    disposing xTextDoc ");

        util.DesktopTools.closeDoc(xTextDoc);
    }

    protected void initialize(TestParameters tParam, PrintWriter log) {
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                     (XMultiServiceFactory) tParam.getMSF());

        try {
            log.println("creating a textdocument");
            xTextDoc = SOF.createTextDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }
    }
}
