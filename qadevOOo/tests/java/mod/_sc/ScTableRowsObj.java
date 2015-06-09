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

import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCellRange;
import com.sun.star.table.XColumnRowRange;
import com.sun.star.table.XTableRows;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.table.TableRows</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XIndexAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::table::XTableRows</code></li>
* </ul>
* @see com.sun.star.table.TableRows
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.table.XTableRows
* @see ifc.container._XIndexAccess
* @see ifc.container._XElementAccess
* @see ifc.table._XTableRows
*/
public class ScTableRowsObj extends TestCase {
    XSpreadsheetDocument xSheetDoc = null;

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
    * Retrieves a collection of spreadsheets from the document and takes one of
    * them. Obtains the collection of rows using the interface
    * <code>XColumnRowRange</code>. The obtained collection is the instance of
    * the service <code>com.sun.star.table.TableRows</code>.
    * @see com.sun.star.table.XColumnRowRange
    * @see com.sun.star.table.TableRows
    */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) throws Exception {

        XInterface oObj = null;

        log.println("getting row");
        XSpreadsheet xSpreadsheet = null;

        XSpreadsheets xSpreadsheets = xSheetDoc.getSheets();
        XNameAccess oNames = UnoRuntime.queryInterface( XNameAccess.class, xSpreadsheets );
        xSpreadsheet = (XSpreadsheet) AnyConverter.toObject(
            new Type(XSpreadsheet.class),
                oNames.getByName(oNames.getElementNames()[0]));

        XColumnRowRange oColumnRowRange = UnoRuntime.queryInterface(XColumnRowRange.class, xSpreadsheet);

        XTableRows oRows = oColumnRowRange.getRows();
        oObj = oRows;

        log.println("creating a new environment for object");
        TestEnvironment tEnv = new TestEnvironment(oObj);

        // adding relation for XTableRows
        tEnv.addObjRelation("XTableRows.XCellRange",
            UnoRuntime.queryInterface(XCellRange.class, xSpreadsheet));

        return tEnv;
    }
}    // finish class ScTableRowsObj

