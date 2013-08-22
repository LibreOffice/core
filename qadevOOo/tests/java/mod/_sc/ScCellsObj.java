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

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XCellRangesQuery;
import com.sun.star.sheet.XSheetCellRanges;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;
import com.sun.star.text.XTextRange;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.Cells</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XEnumerationAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
* </ul>
* @see com.sun.star.sheet.Cells
* @see com.sun.star.container.XEnumerationAccess
* @see com.sun.star.container.XElementAccess
* @see ifc.container._XEnumerationAccess
* @see ifc.container._XElementAccess
*/
public class ScCellsObj extends TestCase {
    private XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());

        try {
            log.println( "creating a sheetdocument" );
            xSheetDoc = SOF.createCalcDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
    * Disposes Spreadsheet document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = UnoRuntime.queryInterface (XComponent.class, xSheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves a collection of spreadsheets from a document,
    * and takes one of them. Replaces text of some cells.
    * Retrives a cell range of the visible cells using the interface
    * <code>XCellRangesQuery</code>. Retrieves a collection of cells from
    * this cell range, this collection is instance of the service
    * <code>com.sun.star.sheet.Cells</code>.
    * @see com.sun.star.sheet.XCellRangesQuery
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;
        Object cellArr[] = new Object[3];

        // creation of testobject here
        XSpreadsheets oSheets = xSheetDoc.getSheets();
        XIndexAccess oIndexAccess = UnoRuntime.queryInterface(XIndexAccess.class, oSheets);
        XCellRange oSheet = null;
        try {
            oSheet = (XCellRange) AnyConverter.toObject(
                    new Type(XCellRange.class),oIndexAccess.getByIndex(0));

            XCell oCell_1 = oSheet.getCellByPosition(0, 0);
            XTextRange oTextRange = UnoRuntime.queryInterface(XTextRange.class, oCell_1);

            oTextRange.setString("ScCellsObj test 1");

            XCell oCell_2 = oSheet.getCellByPosition(5, 1);
            oCell_2.setValue(15);

            XCell oCell_3 = oSheet.getCellByPosition(3, 9);
            oTextRange = UnoRuntime.queryInterface(XTextRange.class, oCell_3);

            oTextRange.setString("ScCellsObj test 2");

            cellArr[0] = oCell_1;
            cellArr[2] = oCell_2;
            cellArr[1] = oCell_3;
        } catch(com.sun.star.lang.WrappedTargetException e) {
            log.println ("Exception occurred while creating test Object.");
            e.printStackTrace(log);
            throw new StatusException("Couldn't create test object", e);
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println ("Exception occurred while creating test Object.");
            e.printStackTrace(log);
            throw new StatusException("Couldn't create test object", e);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println ("Exception occurred while creating test Object.");
            e.printStackTrace(log);
            throw new StatusException("Couldn't create test object", e);
        }

        XCellRangesQuery oCellRangesQuery = UnoRuntime.queryInterface(XCellRangesQuery.class, oSheet);
        XSheetCellRanges oSheetCellRanges = oCellRangesQuery.queryVisibleCells();

        oObj = oSheetCellRanges.getCells();

        TestEnvironment tEnv = new TestEnvironment(oObj) ;
        log.println ("Object created.") ;

        return tEnv;
    }

}    // finish class ScCellsObj

