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

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.FormTools;
import util.SOfficeFactory;
import util.WriterTools;

import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XDevice;
import com.sun.star.awt.XGraphics;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.drawing.XControlShape;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;
import com.sun.star.view.XControlAccess;


/**
 * Test for object which is represented by default controller
 * of the <code>com.sun.star.form.component.ImageButton</code>
 * component. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::lang::XComponent</code></li>
 *  <li> <code>com::sun::star::awt::XWindow</code></li>
 *  <li> <code>com::sun::star::form::XApproveActionBroadcaster</code></li>
 *  <li> <code>com::sun::star::awt::XControl</code></li>
 *  <li> <code>com::sun::star::awt::XLayoutConstrains</code></li>
 *  <li> <code>com::sun::star::awt::XView</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 *
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.awt.XWindow
 * @see com.sun.star.form.XApproveActionBroadcaster
 * @see com.sun.star.awt.XControl
 * @see com.sun.star.awt.XLayoutConstrains
 * @see com.sun.star.awt.XView
 * @see ifc.lang._XComponent
 * @see ifc.awt._XWindow
 * @see ifc.form._XApproveActionBroadcaster
 * @see ifc.awt._XControl
 * @see ifc.awt._XLayoutConstrains
 * @see ifc.awt._XView
 */
public class OImageButtonControl extends TestCase {
    XTextDocument xTextDoc;

    /**
     * Creates a new text document.
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        SOfficeFactory SOF = SOfficeFactory.getFactory(((XMultiServiceFactory) Param.getMSF()));

        try {
            log.println("creating a textdocument");
            xTextDoc = SOF.createTextDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }
    }

    /**
     * Disposes the text document created before
     */
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

    /**
     * Creates two components and inserts them to the form of
     * text document. One component
     * (<code>com.sun.star.form.component.ImageButton</code>) is created
     * for testing, another to be passed as relation. Using a controller
     * of the text document the controller of the first component is
     * obtained and returned in environment as a test object. <p>
     *
     *     Object relations created :
     * <ul>
     *  <li> <code>'GRAPHICS'</code> for
     *      {@link ifc.awt._XView} : a graphics component
     *      created using screen device of the window peer of
     *      the controller tested. </li>
     *  <li> <code>'CONTEXT'</code> for
     *      {@link ifc.awt._XControl} : the text document
     *      where the component is inserted. </li>
     *  <li> <code>'WINPEER'</code> for
     *      {@link ifc.awt._XControl} : Window peer of the
     *      controller tested. </li>
     *  <li> <code>'TOOLKIT'</code> for
     *      {@link ifc.awt._XControl} : toolkit of the component.</li>
     *  <li> <code>'MODEL'</code> for
     *      {@link ifc.awt._XControl} : the model of the controller.</li>
     *  <li> <code>'XWindow.AnotherWindow'</code> for
     *      {@link ifc.awt._XWindow} : the controller of another
     *      component. </li>
     * </ul>
     */
    protected TestEnvironment createTestEnvironment(TestParameters Param,
                                                    PrintWriter log) {
        XInterface oObj = null;
        Object anotherCtrl = null;
        XWindowPeer the_win = null;
        XToolkit the_kit = null;
        XDevice aDevice = null;
        XGraphics aGraphic = null;

        //Insert a ControlShape and get the ControlModel
        XControlShape aShape = FormTools.createControlShape(xTextDoc, 3000,
                                                            4500, 15000, 10000,
                                                            "ImageButton");

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

        //now get the OImageButtonControl
        try {
            oObj = the_access.getControl(the_Model);
            anotherCtrl = the_access.getControl(the_Model2);
            the_win = the_access.getControl(the_Model).getPeer();
            the_kit = the_win.getToolkit();
            aDevice = the_kit.createScreenCompatibleDevice(200, 200);
            aGraphic = aDevice.createGraphics();
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("Couldn't get OImageButtonControl");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get OImageButtonControl", e);
        }

        log.println(
                "creating a new environment for OImageButtonControl object");

        TestEnvironment tEnv = new TestEnvironment(oObj);


        //Adding ObjRelation for XView
        tEnv.addObjRelation("GRAPHICS", aGraphic);


        //Adding ObjRelation for XControl
        tEnv.addObjRelation("CONTEXT", xTextDoc);
        tEnv.addObjRelation("WINPEER", the_win);
        tEnv.addObjRelation("TOOLKIT", the_kit);
        tEnv.addObjRelation("MODEL", the_Model);

        // Adding relation for XWindow
        XWindow forObjRel = UnoRuntime.queryInterface(XWindow.class,
                                                                anotherCtrl);

        tEnv.addObjRelation("XWindow.AnotherWindow", forObjRel);

        return tEnv;
    } // finish method getTestEnvironment
} // finish class OImageButtonControl
