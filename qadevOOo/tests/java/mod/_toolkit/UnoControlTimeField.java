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

import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XDevice;
import com.sun.star.awt.XGraphics;
import com.sun.star.awt.XTextComponent;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.drawing.XControlShape;
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


public class UnoControlTimeField extends TestCase {
    private static XTextDocument xTextDoc;

    @Override
    protected void initialize(TestParameters Param, PrintWriter log) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                     Param.getMSF());

        log.println("creating a textdocument");
        xTextDoc = SOF.createTextDoc(null);
    }

    @Override
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");

        util.DesktopTools.closeDoc(xTextDoc);
    }

    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param,
                                                    PrintWriter log) throws Exception {
        XInterface oObj = null;
        XWindowPeer the_win = null;
        XToolkit the_kit = null;
        XDevice aDevice = null;
        XGraphics aGraphic = null;
        XControl aControl = null;

        //Insert a ControlShape and get the ControlModel
        XControlShape aShape = FormTools.createUnoControlShape(xTextDoc, 3000,
                                                               4500, 15000,
                                                               10000,
                                                               "TimeField",
                                                               "UnoControlTimeField");

        WriterTools.getDrawPage(xTextDoc).add(aShape);

        XControlModel the_Model = aShape.getControl();

        XControlShape aShape2 = FormTools.createControlShape(xTextDoc, 3000,
                                                             4500, 5000, 10000,
                                                             "TextField");

        WriterTools.getDrawPage(xTextDoc).add(aShape2);

        XControlModel the_Model2 = aShape2.getControl();

        //Try to query XControlAccess
        XControlAccess the_access = UnoRuntime.queryInterface(
                                            XControlAccess.class,
                                            xTextDoc.getCurrentController());

        //get the TimeFieldControl for the needed Object relations
        oObj = the_access.getControl(the_Model);
        aControl = the_access.getControl(the_Model2);
        the_win = the_access.getControl(the_Model).getPeer();
        the_kit = the_win.getToolkit();
        aDevice = the_kit.createScreenCompatibleDevice(200, 200);
        aGraphic = aDevice.createGraphics();

        log.println(
                "creating a new environment for UnoControlTimeField object");

        TestEnvironment tEnv = new TestEnvironment(oObj);


        //Adding ObjRelation for XView
        tEnv.addObjRelation("GRAPHICS", aGraphic);


        //Adding ObjRelation for XControl
        tEnv.addObjRelation("CONTEXT", xTextDoc);
        tEnv.addObjRelation("WINPEER", the_win);
        tEnv.addObjRelation("TOOLKIT", the_kit);
        tEnv.addObjRelation("MODEL", the_Model);

        XWindow forObjRel = UnoRuntime.queryInterface(XWindow.class,
                                                                aControl);

        tEnv.addObjRelation("XWindow.AnotherWindow", forObjRel);
        tEnv.addObjRelation("XWindow.ControlShape", aShape);

        // Adding relation for XTextListener
        ifc.awt._XTextListener.TestTextListener listener =
                new ifc.awt._XTextListener.TestTextListener();
        XTextComponent textComp = UnoRuntime.queryInterface(
                                          XTextComponent.class, oObj);
        textComp.addTextListener(listener);
        tEnv.addObjRelation("TestTextListener", listener);

        log.println("ImplementationName: " + utils.getImplName(oObj));

        return tEnv;
    } // finish method getTestEnvironment
} // finish class UnoControlTimeField
