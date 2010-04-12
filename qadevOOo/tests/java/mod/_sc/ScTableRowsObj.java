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
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a Spreadsheet document" );
            xSheetDoc = SOF.createCalcDoc(null);
        } catch ( com.sun.star.uno.Exception e ) {
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
        XComponent oComp = (XComponent)
            UnoRuntime.queryInterface (XComponent.class, xSheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves a collection of spreadsheets from the document and takes one of
    * them. Obtains the collection of rows using the interface
    * <code>XColumnRowRange</code>. The obtained collection is the instance of
    * the service <code>com.sun.star.table.TableRows</code>.
    * @see com.sun.star.table.XColumnRowRange
    * @see com.sun.star.table.TableRows
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        log.println("getting row");
        XSpreadsheet xSpreadsheet = null;

        XSpreadsheets xSpreadsheets = (XSpreadsheets)xSheetDoc.getSheets();
        XNameAccess oNames = (XNameAccess)
            UnoRuntime.queryInterface( XNameAccess.class, xSpreadsheets );
        try {
            xSpreadsheet = (XSpreadsheet) AnyConverter.toObject(
                new Type(XSpreadsheet.class),
                    oNames.getByName(oNames.getElementNames()[0]));
        } catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get spreadsheet", e);
        } catch(com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get spreadsheet", e);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get spreadsheet", e);
        }

        XColumnRowRange oColumnRowRange = (XColumnRowRange)
            UnoRuntime.queryInterface(XColumnRowRange.class, xSpreadsheet);

        XTableRows oRows = (XTableRows) oColumnRowRange.getRows();
        oObj = oRows;

        log.println("creating a new environment for object");
        TestEnvironment tEnv = new TestEnvironment(oObj);

        // adding relation for XTableRows
        tEnv.addObjRelation("XTableRows.XCellRange",
            UnoRuntime.queryInterface(XCellRange.class, xSpreadsheet));

        return tEnv;
    }
}    // finish class ScTableRowsObj

