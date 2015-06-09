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
import util.SOfficeFactory;

import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class ScIndexEnumeration_SheetCellRangesEnumeration extends TestCase {
    private XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF());
        log.println( "creating a sheetdocument" );
        xSheetDoc = SOF.createCalcDoc(null);
    }

    /**
    * Disposes Spreadsheet document.
    */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp =
            UnoRuntime.queryInterface (XComponent.class, xSheetDoc);
        util.DesktopTools.closeDoc(oComp);
    }

    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) throws Exception {

        XInterface oObj = null;
        Object oRange = null ;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "Creating a test environment" );

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( Param.getMSF() );

        log.println("Getting test object ");

        XComponent oComp = UnoRuntime.queryInterface (XComponent.class, xSheetDoc);

        oObj = (XInterface)
            SOF.createInstance(oComp, "com.sun.star.sheet.SheetCellRanges");

        XSpreadsheets oSheets = xSheetDoc.getSheets() ;
        XIndexAccess oIndSheets = UnoRuntime.queryInterface (XIndexAccess.class, oSheets);
        XSpreadsheet oSheet = null;
        oSheet = (XSpreadsheet) AnyConverter.toObject(
                new Type(XSpreadsheet.class),oIndSheets.getByIndex(0));
        XNameContainer oRanges = UnoRuntime.queryInterface(XNameContainer.class, oObj);

        oRange = oSheet.getCellRangeByName("C1:D4");
        oRanges.insertByName("Range1", oRange);
        oRange = oSheet.getCellRangeByName("E2:F5");
        oRanges.insertByName("Range2", oRange);
        oRange = oSheet.getCellRangeByName("G2:H3");
        oRanges.insertByName("Range3", oRange);
        oRange = oSheet.getCellRangeByName("I7:J8");
        oRanges.insertByName("Range4", oRange);

        log.println("filling some cells");
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 5; j++) {
                oSheet.getCellByPosition(i, j).setFormula("a");
            }
        }
        for (int i = 0; i < 10; i++) {
            for (int j = 5; j < 10; j++) {
                oSheet.getCellByPosition(i, j).setValue(i + j);
            }
        }

        XEnumerationAccess ea = UnoRuntime.queryInterface(XEnumerationAccess.class,oObj);

        oObj = ea.createEnumeration();

        log.println("ImplementationName: "+util.utils.getImplName(oObj));
        // creating test environment
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("ENUM",ea);

        return tEnv ;
    }

}

