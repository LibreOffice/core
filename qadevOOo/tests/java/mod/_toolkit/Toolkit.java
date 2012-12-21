/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
