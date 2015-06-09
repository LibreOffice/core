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
import com.sun.star.frame.XController;
import com.sun.star.frame.XFrame;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.view.XControlAccess;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.FormTools;
import util.WriterTools;
import util.utils;


public class UnoControlContainer extends TestCase {
    private static XTextDocument xTextDoc;
    private static XTextDocument xTD2;
    private static XControl xCtrl1;
    private static XControl xCtrl2;

    @Override
    protected void initialize(TestParameters Param, PrintWriter log) throws Exception {
        log.println("creating a textdocument");
        xTD2 = WriterTools.createTextDoc(
                Param.getMSF());
        xTextDoc = WriterTools.createTextDoc(
                Param.getMSF());
    }

    @Override
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");

        util.DesktopTools.closeDoc(xTextDoc);
        util.DesktopTools.closeDoc(xTD2);
    }

    @Override
    public TestEnvironment createTestEnvironment(TestParameters param,
                                                 PrintWriter log) throws Exception {
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
        WriterTools.getDrawPage(xTextDoc).add(shape);
        model = shape.getControl();
        access = UnoRuntime.queryInterface(
                         XControlAccess.class, xTextDoc.getCurrentController());

        access.getControl(model);


        // create second XControl
        shape = FormTools.createControlShape(xTextDoc, 3000, 4500, 15000,
                                             10000, "TextField");
        WriterTools.getDrawPage(xTextDoc).add(shape);
        model = shape.getControl();
        access = UnoRuntime.queryInterface(
                         XControlAccess.class, xTextDoc.getCurrentController());

        xCtrl1 = access.getControl(model);


        // create third XControl
        shape = FormTools.createControlShape(xTextDoc, 3000, 4500, 15000,
                                             10000, "CommandButton");
        WriterTools.getDrawPage(xTextDoc).add(shape);
        model = shape.getControl();
        access = UnoRuntime.queryInterface(
                         XControlAccess.class, xTextDoc.getCurrentController());

        xCtrl2 = access.getControl(model);

        // create XToolkit, XWindowPeer, XDevice
        //Insert a ControlShape and get the ControlModel
        XControlShape aShape = FormTools.createUnoControlShape(xTextDoc, 3000,
                                                               4500, 15000,
                                                               10000,
                                                               "CommandButton",
                                                               "UnoControlButton");

        WriterTools.getDrawPage(xTD2).add(aShape);

        XControlModel the_Model = aShape.getControl();

        //Try to query XControlAccess
        XControlAccess the_access = UnoRuntime.queryInterface(
                                            XControlAccess.class,
                                            xTD2.getCurrentController());

        //get the ButtonControl for the needed Object relations
        the_win = the_access.getControl(the_Model).getPeer();
        the_kit = the_win.getToolkit();

        XDevice aDevice = the_kit.createScreenCompatibleDevice(200, 200);
        aGraphic = aDevice.createGraphics();

        XController aController = xTD2.getCurrentController();
        XFrame aFrame = aController.getFrame();
        anotherWindow = aFrame.getComponentWindow();

        // finished create Object Relations -----------------------------------
        // create the UnoControlContainer
        oObj = (XInterface) param.getMSF().createInstance(
                       "com.sun.star.awt.UnoControlContainer");

        XControl xCtrl = UnoRuntime.queryInterface(
                                 XControl.class, oObj);
        xCtrl.setModel(the_Model);

        ctrlCont = UnoRuntime.queryInterface(
                           XControlContainer.class, oObj);
        ctrlCont.addControl("jupp", access.getControl(aShape.getControl()));

        log.println(
                "creating a new environment for UnoControlContainer object");

        TestEnvironment tEnv = new TestEnvironment(oObj);

        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, oObj);
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
