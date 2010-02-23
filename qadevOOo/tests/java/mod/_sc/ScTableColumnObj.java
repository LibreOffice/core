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

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XColumnRowRange;
import com.sun.star.table.XTableColumns;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.table.TableColumn</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNamed</code></li>
*  <li> <code>com::sun::star::table::TableColumn</code></li>
*  <li> <code>com::sun::star::table::XCellRange</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
* </ul>
* @see com.sun.star.container.XNamed
* @see com.sun.star.table.TableColumn
* @see com.sun.star.table.XCellRange
* @see com.sun.star.beans.XPropertySet
* @see ifc.container._XNamed
* @see ifc.table._TableColumn
* @see ifc.table._XCellRange
* @see ifc.beans._XPropertySet
*/
public class ScTableColumnObj extends TestCase {
    static XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());

        try {
            log.println( "creating a sheetdocument" );
            xSheetDoc = SOF.createCalcDoc(null);;
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
        XComponent oComp = (XComponent)
            UnoRuntime.queryInterface (XComponent.class, xSheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves a collection of spreadsheets from the document and takes one of
    * them. Obtaines the collection of columns in the range using the interface
    * <code>XColumnRowRange</code>. Retrieves the column with index 10 that is
    * the instance of the service <code>com.sun.star.table.TableColumn</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'setName'</code> for
    *      {@link ifc.container._XNamed} </li>
    *  <li> <code>'ValidRange'</code> for
    *      {@link ifc.table._XCellRange} </li>
    * </ul>
    * @see com.sun.star.table.XColumnRowRange
    * @see com.sun.star.table.TableColumn
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        // creation of the testobject here
        // first we write what we are intend to do to log file
        log.println("creating a test environment");

        XSpreadsheet xSpreadsheet = null;

        XSpreadsheets xSpreadsheets = (XSpreadsheets)xSheetDoc.getSheets();
        XNameAccess oNames = (XNameAccess)
            UnoRuntime.queryInterface( XNameAccess.class, xSpreadsheets );
        try {
            xSpreadsheet = (XSpreadsheet) AnyConverter.toObject(
                new Type(XSpreadsheet.class),
                    oNames.getByName(oNames.getElementNames()[0]));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get element by name", e);
        } catch (com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get element by name", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get element by name", e);
        }

        XColumnRowRange oColumnRowRange = (XColumnRowRange)
            UnoRuntime.queryInterface(XColumnRowRange.class, xSpreadsheet);
        XTableColumns oColumns = (XTableColumns) oColumnRowRange.getColumns();
        XIndexAccess oIndexAccess = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class, oColumns);
        try {
            oObj = (XInterface) AnyConverter.toObject(
                    new Type(XInterface.class),oIndexAccess.getByIndex(10));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get by index", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get by index", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get by index", e);
        }

        log.println("creating a new environment for object");
        TestEnvironment tEnv = new TestEnvironment(oObj);

        // a valid Range for XCellRange
        tEnv.addObjRelation("ValidRange","K1:K1");

        //since TableColumnsNames are fixed XNamed::setName() should always be OK
        tEnv.addObjRelation("setName",new Boolean(true));
        return tEnv;
    }
}


