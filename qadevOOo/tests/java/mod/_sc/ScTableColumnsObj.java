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

import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCellRange;
import com.sun.star.table.XColumnRowRange;
import com.sun.star.table.XTableColumns;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.table.TableColumns</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XIndexAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::container::XNameAccess</code></li>
*  <li> <code>com::sun::star::table::XTableColumns</code></li>
* </ul>
* @see com.sun.star.table.TableColumns
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.container.XNameAccess
* @see com.sun.star.table.XTableColumns
* @see ifc.container._XIndexAccess
* @see ifc.container._XElementAccess
* @see ifc.container._XNameAccess
* @see ifc.table._XTableColumns
*/
public class ScTableColumnsObj extends TestCase {
    static XSpreadsheetDocument xSheetDoc = null;

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
    * Retrieves a collection of spreadsheets from the document and takes one of
    * them. Obtaines the collection of columns in the range using the interface
    * <code>XColumnRowRange</code>. This collection is the instance of the
    * service <code>com.sun.star.table.TableColumns</code>.
    * @see com.sun.star.table.XColumnRowRange
    * @see com.sun.star.table.TableColumns
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        // creation of the testobject here
        // first we write what we are intend to do to log file
        log.println("craeting a test environment");

        log.println("getting column");
        XSpreadsheets xSpreadsheets = xSheetDoc.getSheets();
        XNameAccess oNames = UnoRuntime.queryInterface( XNameAccess.class, xSpreadsheets );
        XSpreadsheet xSpreadsheet = null;
        try {
            xSpreadsheet = (XSpreadsheet) AnyConverter.toObject(
                new Type(XSpreadsheet.class),
                    oNames.getByName(oNames.getElementNames()[0]));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get spreadsheet", e);
        } catch (com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get spreadsheet", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get spreadsheet", e);
        }

        XColumnRowRange oColumnRowRange = UnoRuntime.queryInterface(XColumnRowRange.class, xSpreadsheet);
        XTableColumns oColumns = oColumnRowRange.getColumns();
        oObj = oColumns;

        log.println("creating a new environment for object");
        TestEnvironment tEnv = new TestEnvironment(oObj);

        // adding relation for XTableColumns
        tEnv.addObjRelation("XTableColumns.XCellRange",
            UnoRuntime.queryInterface(XCellRange.class, xSpreadsheet));

        return tEnv;
    }
}    // finish class ScTableColumnsObj
