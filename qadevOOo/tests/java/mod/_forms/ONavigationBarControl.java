/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ONavigationBarControl.java,v $
 * $Revision: 1.4 $
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
package mod._forms;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.FormTools;
import util.SOfficeFactory;
import util.WriterTools;

import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XDevice;
import com.sun.star.awt.XGraphics;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XShape;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.view.XControlAccess;
import util.DesktopTools;

public class ONavigationBarControl extends TestCase {
    XTextDocument xTextDoc;

    /**
     * Creates a text document.
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        SOfficeFactory SOF = SOfficeFactory.getFactory((XMultiServiceFactory) Param.getMSF());

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

        DesktopTools.closeDoc(xTextDoc);
    }

    /**
     * Creates two components and inserts them to the form of
     * text document. One component
     * (<code>com.sun.star.form.component.CommandButton</code>) is created
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
        XWindowPeer the_win = null;
        XToolkit the_kit = null;
        XDevice aDevice = null;
        XGraphics aGraphic = null;
        XControl aControl = null;

        //Insert a ControlShape and get the ControlModel
        XControlShape aShape = FormTools.createControlShape(xTextDoc, 3000,
                                                            4500, 15000, 10000,
                                                            "NavigationToolBar");

        WriterTools.getDrawPage(xTextDoc).add((XShape) aShape);

        XControlModel the_Model = aShape.getControl();

        XControlShape aShape2 = FormTools.createControlShape(xTextDoc, 3000,
                                                             4500, 5000, 10000,
                                                             "TextField");

        WriterTools.getDrawPage(xTextDoc).add((XShape) aShape2);

        XControlModel the_Model2 = aShape2.getControl();

        //Try to query XControlAccess
        XControlAccess the_access = (XControlAccess) UnoRuntime.queryInterface(
                                            XControlAccess.class,
                                            xTextDoc.getCurrentController());

        //now get the OButtonControl
        try {
            oObj = the_access.getControl(the_Model);
            aControl = the_access.getControl(the_Model2);
            the_win = the_access.getControl(the_Model).getPeer();
            the_kit = the_win.getToolkit();
            aDevice = the_kit.createScreenCompatibleDevice(200, 200);
            aGraphic = aDevice.createGraphics();
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("Couldn't get ONavigationBarControl");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get ONavigationBarControl", e);
        }

        log.println("creating a new environment for ONavigationBarControl object");

        TestEnvironment tEnv = new TestEnvironment(oObj);


        //Adding ObjRelation for XView
        tEnv.addObjRelation("GRAPHICS", aGraphic);


        //Adding ObjRelation for XControl
        tEnv.addObjRelation("CONTEXT", xTextDoc);
        tEnv.addObjRelation("WINPEER", the_win);
        tEnv.addObjRelation("TOOLKIT", the_kit);
        tEnv.addObjRelation("MODEL", the_Model);

        XWindow forObjRel = (XWindow) UnoRuntime.queryInterface(XWindow.class,
                                                                aControl);

        tEnv.addObjRelation("XWindow.AnotherWindow", forObjRel);
        tEnv.addObjRelation("XWindow.ControlShape", aShape);

        return tEnv;
    } // finish method getTestEnvironment
} // finish class ONavigationBarControl
