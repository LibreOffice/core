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

package mod._fwk;

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XController;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import lib.*;
import util.DesktopTools;
import util.SOfficeFactory;


import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.text.XTextDocument;

/** Test for object that implements the following interfaces :
 * <ul>
 *  <li><code>com::sun::star::frame::XModuleManager</code></li>
 *  <li><code>com::sun::star::container::XNameAccess</code></li>
 *  <li><code>com::sun::star::container::XElementAccess</code></li>
 * </ul><p>
 * @see com.sun.star.frame.XModuleManager
 * @see com.sun.star.container.XNameAccess
 * @see com.sun.star.container.XElementAccess
 * @see ifc.frame._XModuleManager
 * @see ifc.container._XNameAccess
 * @see ifc.container._XElementAccess
 */
public class ModuleManager extends TestCase {

    XTextDocument xTextDoc = null;
    XComponent xWebDoc = null;
    XComponent xGlobalDoc = null;
    XSpreadsheetDocument xSheetDoc = null;
    XComponent xDrawDoc = null;
    XComponent xImpressDoc = null;
    XComponent xMathDoc = null;

    /** Creating a TestEnvironment for the interfaces to be tested.
     * @param Param test parameters
     * @param log simple logger
     * @throws StatusException was thrown on error
     * @return the test environment
     */
    @Override
    public TestEnvironment createTestEnvironment( TestParameters Param,
        PrintWriter log ) throws Exception {

        XInterface oObj = (XInterface)Param.getMSF().createInstance(
            "com.sun.star.comp.framework.ModuleManager");
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                     Param.getMSF());

        // get text document interfaces
        log.println("creating a text document");
        xTextDoc = SOF.createTextDoc(null);
        XModel xTextMode1 = UnoRuntime.queryInterface(XModel.class,
                                                       xTextDoc);
        XController xTextController = xTextMode1.getCurrentController();
        XFrame xTextFrame = xTextController.getFrame();


        // get webdoc interfaces
        log.println("creating a web document");
        xWebDoc = SOF.loadDocument("private:factory/swriter/web");
        XModel xWebMode1 = UnoRuntime.queryInterface(XModel.class,
                                                       xWebDoc);
        XController xWebController = xWebMode1.getCurrentController();
        XFrame xWebFrame = xWebController.getFrame();

        // get global document interfaces
        log.println("creating a global document");
        xGlobalDoc = SOF.loadDocument("private:factory/swriter/GlobalDocument");
        XModel xGlobalMode1 = UnoRuntime.queryInterface(XModel.class,
                                                       xGlobalDoc);
        XController xGlobalController = xGlobalMode1.getCurrentController();
        XFrame xGlobalFrame = xGlobalController.getFrame();

        // get clac interfaces
        log.println("creating a spreadsheetdocument");
        xSheetDoc = SOF.createCalcDoc(null);
        XModel xSheetMode1 = UnoRuntime.queryInterface(XModel.class,
                                                       xSheetDoc);

        XController xSheetController = xSheetMode1.getCurrentController();
        XFrame xSheetFrame = xSheetController.getFrame();
        // get draw interfaces
        log.println("creating a draw document");
        xDrawDoc = SOF.createDrawDoc(null);
        XModel xDrawMode1 = UnoRuntime.queryInterface(XModel.class,
                                                       xDrawDoc);
        XController xDrawController = xDrawMode1.getCurrentController();
        XFrame xDrawFrame = xDrawController.getFrame();

        // get impress interfaces
        log.println("creating an impress document");
        xImpressDoc = SOF.createImpressDoc(null);
        XModel xImpressMode1 = UnoRuntime.queryInterface(XModel.class,
                                                       xImpressDoc);
        XController xImpressController = xImpressMode1.getCurrentController();
        XFrame xImpressFrame = xImpressController.getFrame();

        // get math interfaces
        log.println("creating a math document");
        xMathDoc = SOF.createMathDoc(null);
        XModel xMathMode1 = UnoRuntime.queryInterface(XModel.class,
                                                       xMathDoc);
        XController xMathController = xMathMode1.getCurrentController();
        XFrame xMathFrame = xMathController.getFrame();

        PropertyValue[] xFrameSeq = new PropertyValue[7];
        xFrameSeq[0] = new PropertyValue();
        xFrameSeq[0].Name = "com.sun.star.text.TextDocument";
        xFrameSeq[0].Value = xTextFrame;
        xFrameSeq[1] = new PropertyValue();
        xFrameSeq[1].Name = "com.sun.star.text.WebDocument";
        xFrameSeq[1].Value = xWebFrame;
        xFrameSeq[2] = new PropertyValue();
        xFrameSeq[2].Name = "com.sun.star.text.GlobalDocument";
        xFrameSeq[2].Value = xGlobalFrame;
        xFrameSeq[3] = new PropertyValue();
        xFrameSeq[3].Name = "com.sun.star.sheet.SpreadsheetDocument";
        xFrameSeq[3].Value = xSheetFrame;
        xFrameSeq[4] = new PropertyValue();
        xFrameSeq[4].Name = "com.sun.star.drawing.DrawingDocument";
        xFrameSeq[4].Value = xDrawFrame;
        xFrameSeq[5] = new PropertyValue();
        xFrameSeq[5].Name = "com.sun.star.formula.FormulaProperties";
        xFrameSeq[5].Value = xMathFrame;
        xFrameSeq[6] = new PropertyValue();
        xFrameSeq[6].Name = "com.sun.star.presentation.PresentationDocument";
        xFrameSeq[6].Value = xImpressFrame;

        PropertyValue[] xControllerSeq = new PropertyValue[7];
        xControllerSeq[0] = new PropertyValue();
        xControllerSeq[0].Name = "com.sun.star.text.TextDocument";
        xControllerSeq[0].Value = xTextController;
        xControllerSeq[1] = new PropertyValue();
        xControllerSeq[1].Name = "com.sun.star.text.WebDocument";
        xControllerSeq[1].Value = xWebController;
        xControllerSeq[2] = new PropertyValue();
        xControllerSeq[2].Name = "com.sun.star.text.GlobalDocument";
        xControllerSeq[2].Value = xGlobalController;
        xControllerSeq[3] = new PropertyValue();
        xControllerSeq[3].Name = "com.sun.star.sheet.SpreadsheetDocument";
        xControllerSeq[3].Value = xSheetController;
        xControllerSeq[4] = new PropertyValue();
        xControllerSeq[4].Name = "com.sun.star.drawing.DrawingDocument";
        xControllerSeq[4].Value = xDrawController;
        xControllerSeq[5] = new PropertyValue();
        xControllerSeq[5].Name = "com.sun.star.formula.FormulaProperties";
        xControllerSeq[5].Value = xMathController;
        xControllerSeq[6] = new PropertyValue();
        xControllerSeq[6].Name = "com.sun.star.presentation.PresentationDocument";
        xControllerSeq[6].Value = xImpressController;

        PropertyValue[] xModelSeq = new PropertyValue[7];
        xModelSeq[0] = new PropertyValue();
        xModelSeq[0].Name = "com.sun.star.text.TextDocument";
        xModelSeq[0].Value = xTextMode1;
        xModelSeq[1] = new PropertyValue();
        xModelSeq[1].Name = "com.sun.star.text.WebDocument";
        xModelSeq[1].Value = xWebMode1;
        xModelSeq[2] = new PropertyValue();
        xModelSeq[2].Name = "com.sun.star.text.GlobalDocument";
        xModelSeq[2].Value = xGlobalMode1;
        xModelSeq[3] = new PropertyValue();
        xModelSeq[3].Name = "com.sun.star.sheet.SpreadsheetDocument";
        xModelSeq[3].Value = xSheetMode1;
        xModelSeq[4] = new PropertyValue();
        xModelSeq[4].Name = "com.sun.star.drawing.DrawingDocument";
        xModelSeq[4].Value = xDrawMode1;
        xModelSeq[5] = new PropertyValue();
        xModelSeq[5].Name = "com.sun.star.presentation.PresentationDocument";
        xModelSeq[5].Value = xImpressMode1;
        xModelSeq[6] = new PropertyValue();
        xModelSeq[6].Name = "com.sun.star.formula.FormulaProperties";
        xModelSeq[6].Value = xMathMode1;

        TestEnvironment tEnv = new TestEnvironment( oObj );


        tEnv.addObjRelation("XModuleManager.XFrame", xFrameSeq);
        tEnv.addObjRelation("XModuleManager.XController", xControllerSeq);
        tEnv.addObjRelation("XModuleManager.XModel", xModelSeq);
        return tEnv;
    } // finish method getTestEnvironment

    /** Disposes text document.
     * @param tParam test parameters
     * @param log simple logger
     */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    closing all documents " );
        DesktopTools.closeDoc(xTextDoc);
        DesktopTools.closeDoc(xWebDoc);
        DesktopTools.closeDoc(xGlobalDoc);
        DesktopTools.closeDoc(xSheetDoc);
        DesktopTools.closeDoc(xDrawDoc);
        DesktopTools.closeDoc(xImpressDoc);
        DesktopTools.closeDoc(xMathDoc);
    }
}
