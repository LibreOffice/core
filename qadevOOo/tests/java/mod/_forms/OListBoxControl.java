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

package mod._forms;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.FormTools;
import util.SOfficeFactory;
import util.WriterTools;

import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XDevice;
import com.sun.star.awt.XGraphics;
import com.sun.star.awt.XListBox;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.drawing.XControlShape;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;
import com.sun.star.view.XControlAccess;

public class OListBoxControl extends TestCase {

    XTextDocument xTextDoc;

    @Override
    protected void initialize ( TestParameters Param, PrintWriter log) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( Param.getMSF() );
        log.println( "creating a textdocument" );
        xTextDoc = SOF.createTextDoc( null );
    }

    @Override
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");

        try {
            XCloseable closer = UnoRuntime.queryInterface(
                                        XCloseable.class, xTextDoc);
            closer.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            log.println("couldn't close document");
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("couldn't close document");
        }
    }

    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) throws Exception {
        XInterface oObj = null;
        Object anotherCtrl = null ;
        XWindowPeer the_win = null;
        XToolkit the_kit = null;
        XDevice aDevice = null;
        XGraphics aGraphic = null;
        //Insert a ControlShape and get the ControlModel
        XControlShape aShape = FormTools.createControlShape(
                                xTextDoc,3000,4500,15000,10000,"ListBox");

        WriterTools.getDrawPage(xTextDoc).add(aShape);

        XControlModel the_Model = aShape.getControl();

        XControlShape aShape2 = FormTools.createControlShape(
                                xTextDoc,3000,4500,5000,10000,"TextField");

        WriterTools.getDrawPage(xTextDoc).add(aShape2);

        XControlModel the_Model2 = aShape2.getControl();

        //Try to query XControlAccess
        XControlAccess the_access = UnoRuntime.queryInterface(
                        XControlAccess.class,xTextDoc.getCurrentController());

        //now get the OListBoxControl
        oObj = the_access.getControl(the_Model);
        anotherCtrl = the_access.getControl(the_Model2);
        the_win = the_access.getControl(the_Model).getPeer();
        the_kit = the_win.getToolkit();
        aDevice = the_kit.createScreenCompatibleDevice(200,200);
        aGraphic = aDevice.createGraphics();

        log.println( "creating a new environment for OListBoxControl object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        //Adding ObjRelation for XView
        tEnv.addObjRelation("GRAPHICS",aGraphic);

        //Adding ObjRelation for XControl
        tEnv.addObjRelation("CONTEXT",xTextDoc);
        tEnv.addObjRelation("WINPEER",the_win);
        tEnv.addObjRelation("TOOLKIT",the_kit);
        tEnv.addObjRelation("MODEL",the_Model);

        // Adding relation for XItemListener
        ifc.awt._XItemListener.TestItemListener listener =
            new ifc.awt._XItemListener.TestItemListener() ;
        final XListBox box = UnoRuntime.queryInterface(XListBox.class, oObj) ;
        box.addItemListener(listener) ;
        tEnv.addObjRelation("TestItemListener", listener) ;

        // Adding relation for XWindow
        XWindow forObjRel = UnoRuntime.queryInterface(XWindow.class, anotherCtrl);

        XWindow objWin = UnoRuntime.queryInterface(XWindow.class, oObj);

        tEnv.addObjRelation("XWindow.AnotherWindow",forObjRel);
        tEnv.addObjRelation("XWindow.ControlShape",aShape);

        tEnv.addObjRelation("Win1",objWin);
        tEnv.addObjRelation("Win2",forObjRel);

        tEnv.addObjRelation("CONTROL",anotherCtrl);

        // adding relation for XChangeBroadcaster
        box.addItem("Item1", (short) 0);
        box.addItem("Item2", (short) 1);

        tEnv.addObjRelation("XChangeBroadcaster.Changer",
            new ifc.form._XChangeBroadcaster.Changer() {
                public void change(){
                    box.addItem("Item1", (short) 0);
                    box.addItem("Item2", (short) 1);
                    box.selectItemPos((short) 0, true);
                    box.selectItemPos((short) 1, true);
                }
            }
        );

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class OListBoxControl

