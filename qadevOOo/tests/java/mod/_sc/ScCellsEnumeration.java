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
import com.sun.star.lang.XComponent;
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
* <code>com.sun.star.sheet.CellsEnumeration</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XEnumeration</code></li>
* </ul>
* @see com.sun.star.sheet.CellsEnumeration
* @see com.sun.star.container.XEnumeration
* @see ifc.container._XEnumeration
*/
public class ScCellsEnumeration extends TestCase {
    private XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );
        log.println( "creating a Spreadsheet document" );
        xSheetDoc = SOF.createCalcDoc(null);
    }

    /**
    * Disposes Spreadsheet document.
    */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = UnoRuntime.queryInterface (XComponent.class, xSheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Retrieves a collection of spreadsheets from a document
    * and takes one of them. Replaces text of some cells.
    * Retrieves a cell range of the visible cells using the interface
    * <code>XCellRangesQuery</code>. Retrieves a collection of cells from
    * this cell range and creates it's enumeration using the interface
    * <code>XEnumerationAccess</code>.The created enumeration is the instance
    * of the service <code>com.sun.star.sheet.CellsEnumeration</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'ENUM'</code> for
    *      {@link ifc.container._XEnumeration} (type of
    *      <code>XEnumerationAccess</code> that was retrieved from the
    *      collection of visible cells)</li>
    * </ul>
    * @see com.sun.star.sheet.XCellRangesQuery
    * @see com.sun.star.container.XEnumerationAccess
    */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) throws Exception {

        XInterface oObj = null;
        Object cellArr[] = new Object[3] ;
        XEnumerationAccess oEnum = null;

        // creation of testobject here
        XSpreadsheets oSheets = xSheetDoc.getSheets();
        XIndexAccess oIndexAccess = UnoRuntime.queryInterface(XIndexAccess.class, oSheets);
        XCellRange oSheet = null;
        oSheet = (XCellRange) AnyConverter.toObject(
                new Type(XCellRange.class),oIndexAccess.getByIndex(0));

        XCell oCell_1 = oSheet.getCellByPosition(0, 0);
        XTextRange oTextRange = UnoRuntime.queryInterface(XTextRange.class, oCell_1);
        oTextRange.setString("Test string 1");

        XCell oCell_2 = oSheet.getCellByPosition(5, 1);
        oCell_2.setValue(15);

        XCell oCell_3 = oSheet.getCellByPosition(3, 9);
        oTextRange = UnoRuntime.queryInterface(XTextRange.class, oCell_3);
        oTextRange.setString("test 2");
        cellArr[0] = oCell_1;
        cellArr[1] = oCell_2;
        cellArr[2] = oCell_3;

        XCellRangesQuery oCellRangesQuery = UnoRuntime.queryInterface(XCellRangesQuery.class, oSheet);
        XSheetCellRanges oSheetCellRanges = oCellRangesQuery.queryVisibleCells();
        oEnum = oSheetCellRanges.getCells();
        oObj = oSheetCellRanges.getCells().createEnumeration();

        TestEnvironment tEnv = new TestEnvironment(oObj);

        // ENUM: XEnumeration
        tEnv.addObjRelation("ENUM", oEnum);

        return tEnv;
    }

}    // finish class ScCellsEnumeration

