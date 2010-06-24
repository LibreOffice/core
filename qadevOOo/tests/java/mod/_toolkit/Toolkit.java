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

import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XShape;
import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
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
import util.SOfficeFactory;
import util.WriterTools;
import util.utils;


/**
* Test for <code>com.sun.star.awt.Toolkit</code> service.
*/
public class Toolkit extends TestCase {
    private static XTextDocument xTextDoc;

    protected void initialize(TestParameters Param, PrintWriter log) {
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                     (XMultiServiceFactory) Param.getMSF());

        try {
            log.println("creating a textdocument");
            xTextDoc = SOF.createTextDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }
    }

    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");
        util.DesktopTools.closeDoc(xTextDoc);
        ;
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates <code>com.sun.star.awt.Toolkit</code> service.
    */
    public TestEnvironment createTestEnvironment(TestParameters Param,
                                                 PrintWriter log)
                                          throws StatusException {
        XInterface oObj = null;
        XWindowPeer the_win = null;
        XWindow win = null;

        //Insert a ControlShape and get the ControlModel
        XControlShape aShape = FormTools.createControlShape(xTextDoc, 3000,
                                                            4500, 15000, 10000,
                                                            "CommandButton");

        WriterTools.getDrawPage(xTextDoc).add((XShape) aShape);

        XControlModel the_Model = aShape.getControl();

        //Try to query XControlAccess
        XControlAccess the_access = (XControlAccess) UnoRuntime.queryInterface(
                                            XControlAccess.class,
                                            xTextDoc.getCurrentController());
        XController cntrlr = (XController) UnoRuntime.queryInterface(
                                     XController.class,
                                     xTextDoc.getCurrentController());

        //now get the toolkit
        try {
            win = cntrlr.getFrame().getContainerWindow();


            //win = (XWindow) UnoRuntime.queryInterface(XWindow.class, ctrl) ;
            the_win = the_access.getControl(the_Model).getPeer();
            oObj = (XInterface) ((XMultiServiceFactory) Param.getMSF()).createInstance(
                           "com.sun.star.awt.Toolkit");
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get toolkit");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get toolkit", e);
        }

        XModel xModel = (XModel)UnoRuntime.queryInterface(XModel.class, xTextDoc);

        log.println("    creating a new environment for toolkit object");

        TestEnvironment tEnv = new TestEnvironment(oObj);

        log.println("Implementation Name: " + utils.getImplName(oObj));

        tEnv.addObjRelation("WINPEER", the_win);

        tEnv.addObjRelation("XModel", xModel);


        // adding relation for XDataTransferProviderAccess
        tEnv.addObjRelation("XDataTransferProviderAccess.XWindow", win);

        return tEnv;
    } // finish method getTestEnvironment
} // finish class Toolkit
