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

import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XDevice;
import com.sun.star.awt.XGraphics;
import com.sun.star.awt.XTextComponent;
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
import util.SOfficeFactory;
import util.WriterTools;
import util.utils;


public class UnoControlDateField extends TestCase {
    private static XTextDocument xTextDoc;
    private static XTextDocument xTD2;

    @Override
    protected void initialize(TestParameters Param, PrintWriter log) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                     Param.getMSF());

        log.println("creating a textdocument");
        xTextDoc = SOF.createTextDoc(null);
        xTD2 = WriterTools.createTextDoc(
                       Param.getMSF());
    }

    @Override
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");

        util.DesktopTools.closeDoc(xTextDoc);
        util.DesktopTools.closeDoc(xTD2);
    }

    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param,
                                                    PrintWriter log) throws Exception {
        XInterface oObj = null;
        XWindowPeer the_win = null;
        XToolkit the_kit = null;
        XDevice aDevice = null;
        XGraphics aGraphic = null;
        XWindow anotherWindow = null;

        //Insert a ControlShape and get the ControlModel
        XControlShape aShape = FormTools.createUnoControlShape(xTextDoc, 3000,
                                                               4500, 15000,
                                                               10000,
                                                               "DateField",
                                                               "UnoControlDateField");

        WriterTools.getDrawPage(xTextDoc).add(aShape);

        XControlModel the_Model = aShape.getControl();

        //Try to query XControlAccess
        XControlAccess the_access = UnoRuntime.queryInterface(
                                            XControlAccess.class,
                                            xTextDoc.getCurrentController());

        //get the DateFieldControl for the needed Object relations
        oObj = the_access.getControl(the_Model);
        the_win = the_access.getControl(the_Model).getPeer();
        the_kit = the_win.getToolkit();
        aDevice = the_kit.createScreenCompatibleDevice(200, 200);
        aGraphic = aDevice.createGraphics();

        log.println(
                "creating a new environment for UnoControlDateField object");

        TestEnvironment tEnv = new TestEnvironment(oObj);


        //Adding ObjRelation for XView
        tEnv.addObjRelation("GRAPHICS", aGraphic);


        //Adding ObjRelation for XControl
        tEnv.addObjRelation("CONTEXT", xTextDoc);
        tEnv.addObjRelation("WINPEER", the_win);
        tEnv.addObjRelation("TOOLKIT", the_kit);
        tEnv.addObjRelation("MODEL", the_Model);

        System.out.println("ImplementationName: " + utils.getImplName(oObj));

        XController aController = xTD2.getCurrentController();
        XFrame aFrame = aController.getFrame();
        anotherWindow = aFrame.getComponentWindow();


        // Object Relation for XWindow
        tEnv.addObjRelation("XWindow.AnotherWindow", anotherWindow);
        tEnv.addObjRelation("XWindow.ControlShape", aShape);

        // Adding relation for XTextListener
        ifc.awt._XTextListener.TestTextListener listener =
                new ifc.awt._XTextListener.TestTextListener();
        XTextComponent textComp = UnoRuntime.queryInterface(
                                          XTextComponent.class, oObj);
        textComp.addTextListener(listener);
        tEnv.addObjRelation("TestTextListener", listener);

        return tEnv;
    } // finish method getTestEnvironment
} // finish class UnoControlDateField
