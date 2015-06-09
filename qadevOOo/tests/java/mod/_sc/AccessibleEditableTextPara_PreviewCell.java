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

package mod._sc;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.container.XIndexAccess;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.lang.DisposedException;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCell;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;


public class AccessibleEditableTextPara_PreviewCell extends TestCase {
    XSpreadsheetDocument xSheetDoc = null;

    /**
     * Creates a spreadsheet document.
     */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );
        log.println( "creating a Spreadsheet document" );
        xSheetDoc = SOF.createCalcDoc(null);
    }

    /**
     * Disposes a spreadsheet document.
     */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = UnoRuntime.queryInterface
            (XComponent.class, xSheetDoc);
        util.DesktopTools.closeDoc(oComp);
    }


    /**
     * Creating a TestEnvironment for the interfaces to be tested.
     * Obtains the accessible object for a one of cell in preview mode.
     */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) throws Exception {

        XCell xCell = null;

        log.println("Getting spreadsheet") ;
        XSpreadsheets oSheets = xSheetDoc.getSheets() ;
        XIndexAccess oIndexSheets = UnoRuntime.queryInterface(XIndexAccess.class, oSheets);
        XSpreadsheet oSheet = (XSpreadsheet) AnyConverter.toObject(
                new Type(XSpreadsheet.class),oIndexSheets.getByIndex(0));

        log.println("Getting a cell from sheet") ;
        xCell = oSheet.getCellByPosition(0, 0);

        xCell.setFormula("Value");

        XModel xModel = UnoRuntime.queryInterface(XModel.class, xSheetDoc);

        XController xController = xModel.getCurrentController();

        //switch to 'Print Preview' mode
        XDispatchProvider xDispProv = UnoRuntime.queryInterface(XDispatchProvider.class, xController);
        XURLTransformer xParser = UnoRuntime.queryInterface(XURLTransformer.class,
                Param.getMSF().createInstance("com.sun.star.util.URLTransformer"));
        URL[] aParseURL = new URL[1];
        aParseURL[0] = new URL();
        aParseURL[0].Complete = ".uno:PrintPreview";
        xParser.parseStrict(aParseURL);
        URL aURL = aParseURL[0];
        XDispatch xDispatcher = xDispProv.queryDispatch(aURL, "", 0);
        if(xDispatcher != null)
            xDispatcher.dispatch( aURL, null );

        XAccessibleContext oObj = null;
        for (int i = 0;; ++i) {
            Thread.sleep(500);
            try {
                XAccessible xRoot = AccessibilityTools.getAccessibleObject(
                    AccessibilityTools.getCurrentWindow(
                        xModel));
                if (xRoot != null) {
                    AccessibilityTools.getAccessibleObjectForRole(
                        xRoot, AccessibleRole.TABLE_CELL, true);
                    xRoot = AccessibilityTools.SearchedAccessible;
                    if (xRoot != null) {
                        oObj = AccessibilityTools.getAccessibleObjectForRole(
                            xRoot, AccessibleRole.PARAGRAPH);
                        if (oObj != null) {
                            break;
                        }
                    }
                }
            } catch (DisposedException e) {
                log.println("Ignoring DisposedException");
            }
            if (i == 20) { // give up after 10 sec
                throw new RuntimeException(
                    "Couldn't get AccessibleRoot.HEADER object");
            }
            log.println("No TABLE_CELL/PARAGRAPH found yet, retrying");
        }

        log.println("ImplementationName " + utils.getImplName(oObj));
        log.println("AccessibleName " + oObj.getAccessibleName());
        log.println("Parent " + utils.getImplName(oObj.getAccessibleParent()));

        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("EditOnly", "AccessibleEditableTextPara_PreviewCell");
        tEnv.addObjRelation("Destroy", "AccessibleEditableTextPara_PreviewCell");

        final XCell cell_to_change = xCell;
        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer(){
                public void fireEvent() {
                    cell_to_change.setFormula("NewString");
                }
            });

        return tEnv;
    }
}
