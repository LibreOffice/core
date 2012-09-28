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
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XColumnRowRange;
import com.sun.star.table.XTableRows;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.table.TableRow</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::table::TableRow</code></li>
*  <li> <code>com::sun::star::table::XCellRange</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
* </ul>
* @see com.sun.star.table.TableRow
* @see com.sun.star.table.XCellRange
* @see com.sun.star.beans.XPropertySet
* @see ifc.table._TableRow
* @see ifc.table._XCellRange
* @see ifc.beans._XPropertySet
*/
public class ScTableRowObj extends TestCase {
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
    * them. Obtains the collection of rows using the interface
    * <code>XColumnRowRange</code>. Obtains the row with index 6 from the
    * collection and this is the instance of the service
    * <code>com.sun.star.table.TableRow</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'ValidRange'</code> for
    *      {@link ifc.table._XCellRange} </li>
    * </ul>
    * @see com.sun.star.table.XColumnRowRange
    * @see com.sun.star.table.TableRow
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        // creation of the testobject here
        // first we write what we are intend to do to log file
        log.println("creating a test environment");

        XSpreadsheet xSpreadsheet = null;
        XSpreadsheets xSpreadsheets = xSheetDoc.getSheets();
        XNameAccess oNames = UnoRuntime.queryInterface( XNameAccess.class, xSpreadsheets );
        try {
            xSpreadsheet = (XSpreadsheet) AnyConverter.toObject(
                new Type(XSpreadsheet.class),
                    oNames.getByName(oNames.getElementNames()[0]));

            XColumnRowRange oColumnRowRange = UnoRuntime.queryInterface(XColumnRowRange.class, xSpreadsheet);
            XTableRows oRows = oColumnRowRange.getRows();
            XIndexAccess oIndexAccess = UnoRuntime.queryInterface(XIndexAccess.class, oRows);
            oObj = (XInterface) AnyConverter.toObject(
                    new Type(XInterface.class),oIndexAccess.getByIndex(6));
        } catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Exception during creating Testenvironment", e);
        } catch(com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Exception during creating Testenvironment", e);
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Exception during creating Testenvironment", e);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Exception during creating Testenvironment", e);
        }

        log.println("creating a new environment for object");
        TestEnvironment tEnv = new TestEnvironment(oObj);
        tEnv.addObjRelation("ValidRange","A7:A7");
        return tEnv;
    }
}

