/*************************************************************************
 *
 *  $RCSfile: UnoControlContainer.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 13:04:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
package mod._toolkit;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.FormTools;
import util.WriterTools;
import util.utils;

import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
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
import com.sun.star.util.XCloseable;
import com.sun.star.view.XControlAccess;


public class UnoControlContainer extends TestCase {
    XTextDocument xTextDoc;
    XTextDocument xTD2;
    XControl xCtrl1;
    XControl xCtrl2;

    protected void initialize(TestParameters param, PrintWriter log) {
        try {
            log.println("creating a textdocument");
            xTD2 = WriterTools.createTextDoc( (XMultiServiceFactory) param.getMSF());
            xTextDoc = WriterTools.createTextDoc( (XMultiServiceFactory) param.getMSF());
        } catch (Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }
    }

    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");

        try {
            XCloseable closer = (XCloseable) UnoRuntime.queryInterface(
                                        XCloseable.class, xTextDoc);
            closer.close(true);

            closer = (XCloseable) UnoRuntime.queryInterface(XCloseable.class,
                                                            xTD2);
            closer.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            log.println("couldn't close document");
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("couldn't close document");
        }
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


        // create 2 XControls
        // create first XControl
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


        // create second XControl
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
            //aDevice = the_kit.createScreenCompatibleDevice(200, 200);
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
            oObj = (XInterface) ( (XMultiServiceFactory) param.getMSF())
                                     .createInstance("com.sun.star.awt.UnoControlContainer");

            XControl xCtrl = (XControl) UnoRuntime.queryInterface(
                                     XControl.class, oObj);
            xCtrl.setModel(the_Model);
        } catch (Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create UnoControlContainer", e);
        }

        log.println(
                "creating a new environment for UnoControlContainer object");

        TestEnvironment tEnv = new TestEnvironment(oObj);

        String objName = "UnoControlContainer";
        tEnv.addObjRelation("OBJNAME", "toolkit." + objName);


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