/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: UnoControlContainer.java,v $
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
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlContainer;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XDevice;
import com.sun.star.awt.XGraphics;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XShape;
import com.sun.star.frame.XController;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.view.XControlAccess;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.FormTools;
import util.WriterTools;
import util.utils;


public class UnoControlContainer extends TestCase {
    private static XTextDocument xTextDoc;
    private static XTextDocument xTD2;
    private static XControl xCtrl;
    private static XControl xCtrl1;
    private static XControl xCtrl2;

    protected void initialize(TestParameters param, PrintWriter log) {
        try {
            log.println("creating a textdocument");
            xTD2 = WriterTools.createTextDoc(
                           (XMultiServiceFactory) param.getMSF());
            xTextDoc = WriterTools.createTextDoc(
                               (XMultiServiceFactory) param.getMSF());
        } catch (Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }
    }

    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");

        util.DesktopTools.closeDoc(xTextDoc);
        util.DesktopTools.closeDoc(xTD2);
    }

    public TestEnvironment createTestEnvironment(TestParameters param,
                                                 PrintWriter log) {
        // create Object Relations -------------------------------------------
        XInterface oObj = null;
        XControlShape shape = null;
        XControlModel model = null;
        XControlAccess access = null;
        XWindow anotherWindow = null;

        // for XControl
        XWindowPeer the_win = null;
        XToolkit the_kit = null;

        XControlContainer ctrlCont = null;

        XGraphics aGraphic = null;


        // create 3 XControls
        // create first XControl
        shape = FormTools.createControlShape(xTextDoc, 3000, 4500, 15000,
                                             10000, "TextField");
        WriterTools.getDrawPage(xTextDoc).add((XShape) shape);
        model = shape.getControl();
        access = (XControlAccess) UnoRuntime.queryInterface(
                         XControlAccess.class, xTextDoc.getCurrentController());

        try {
            xCtrl = access.getControl(model);
        } catch (Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create XControl", e);
        }


        // create second XControl
        shape = FormTools.createControlShape(xTextDoc, 3000, 4500, 15000,
                                             10000, "TextField");
        WriterTools.getDrawPage(xTextDoc).add((XShape) shape);
        model = shape.getControl();
        access = (XControlAccess) UnoRuntime.queryInterface(
                         XControlAccess.class, xTextDoc.getCurrentController());

        try {
            xCtrl1 = access.getControl(model);
        } catch (Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create XControl", e);
        }


        // create third XControl
        shape = FormTools.createControlShape(xTextDoc, 3000, 4500, 15000,
                                             10000, "CommandButton");
        WriterTools.getDrawPage(xTextDoc).add((XShape) shape);
        model = shape.getControl();
        access = (XControlAccess) UnoRuntime.queryInterface(
                         XControlAccess.class, xTextDoc.getCurrentController());

        try {
            xCtrl2 = access.getControl(model);
        } catch (Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create XControl", e);
        }

        // create XToolkit, XWindowPeer, XDevice
        //Insert a ControlShape and get the ControlModel
        XControlShape aShape = FormTools.createUnoControlShape(xTextDoc, 3000,
                                                               4500, 15000,
                                                               10000,
                                                               "CommandButton",
                                                               "UnoControlButton");

        WriterTools.getDrawPage(xTD2).add((XShape) aShape);

        XControlModel the_Model = aShape.getControl();

        //Try to query XControlAccess
        XControlAccess the_access = (XControlAccess) UnoRuntime.queryInterface(
                                            XControlAccess.class,
                                            xTD2.getCurrentController());

        //get the ButtonControl for the needed Object relations
        try {
            the_win = the_access.getControl(the_Model).getPeer();
            the_kit = the_win.getToolkit();

            XDevice aDevice = the_kit.createScreenCompatibleDevice(200, 200);
            aGraphic = aDevice.createGraphics();
        } catch (Exception e) {
            log.println("Couldn't get ButtonControl");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get ButtonControl", e);
        }

        try {
            XController aController = xTD2.getCurrentController();
            XFrame aFrame = aController.getFrame();
            anotherWindow = aFrame.getComponentWindow();
        } catch (Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create XWindow", e);
        }

        // finished create Object Relations -----------------------------------
        // create the UnoControlContainer
        try {
            oObj = (XInterface) ((XMultiServiceFactory) param.getMSF()).createInstance(
                           "com.sun.star.awt.UnoControlContainer");

            XControl xCtrl = (XControl) UnoRuntime.queryInterface(
                                     XControl.class, oObj);
            xCtrl.setModel(the_Model);

            ctrlCont = (XControlContainer) UnoRuntime.queryInterface(
                               XControlContainer.class, oObj);
            ctrlCont.addControl("jupp", access.getControl(aShape.getControl()));
        } catch (Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create UnoControlContainer", e);
        }

        log.println(
                "creating a new environment for UnoControlContainer object");

        TestEnvironment tEnv = new TestEnvironment(oObj);

        XWindow xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, oObj);
        Rectangle ps = xWindow.getPosSize();
        xWindow.setPosSize(ps.X+10, ps.Y+10, ps.Width+10, ps.Height+10, PosSize.POSSIZE);

        String objName = "UnoControlContainer";
        tEnv.addObjRelation("OBJNAME", "toolkit." + objName);


        // Object relation for XContainer
        tEnv.addObjRelation("XContainer.Container", ctrlCont);
        tEnv.addObjRelation("INSTANCE", xCtrl);


        //Adding ObjRelation for XView
        tEnv.addObjRelation("GRAPHICS", aGraphic);


        // Object Relation for XControlContainer
        tEnv.addObjRelation("CONTROL1", xCtrl1);
        tEnv.addObjRelation("CONTROL2", xCtrl2);


        // Object Relation for XControl
        tEnv.addObjRelation("CONTEXT", xTD2);
        tEnv.addObjRelation("WINPEER", the_win);
        tEnv.addObjRelation("TOOLKIT", the_kit);
        tEnv.addObjRelation("MODEL", the_Model);


        // Object Relation for XWindow
        tEnv.addObjRelation("XWindow.AnotherWindow", anotherWindow);
        System.out.println("ImplementationName: " + utils.getImplName(oObj));

        return tEnv;
    }
}
