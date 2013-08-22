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
import com.sun.star.sheet.XDataPilotDescriptor;
import com.sun.star.sheet.XDataPilotTables;
import com.sun.star.sheet.XDataPilotTablesSupplier;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.CellAddress;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.DataPilotFields</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNameAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
* </ul>
* @see com.sun.star.sheet.DataPilotFields
* @see com.sun.star.container.XNameAccess
* @see com.sun.star.container.XElementAccess
* @see ifc.container._XNameAccess
* @see ifc.container._XElementAccess
*/
public class ScDataPilotFieldsObj extends TestCase {
    private XSpreadsheetDocument xSheetDoc = null;

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
        XComponent oComp = UnoRuntime.queryInterface(XComponent.class, xSheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves a collection of spreadsheets from a document
    * and takes one of them. Fills some table in the spreadsheet.
    * Obtains the collection of data pilot tables using the interface
    * <code>XDataPilotTablesSupplier</code>. Creates a data pilot descriptor
    * for the filled table and inserts new data pilot table with this descriptor
    * to the collection. Obtains the collection of all the data pilot fields
    * using the interface <code>XDataPilotDescriptor</code>. This collection
    * is the instance of the service <code>com.sun.star.sheet.DataPilotFields</code>.
    * @see com.sun.star.sheet.DataPilotFields
    * @see com.sun.star.sheet.XDataPilotTablesSupplier
    * @see com.sun.star.sheet.XDataPilotDescriptor
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "Creating a test environment" );

        // create testobject here

        log.println("getting sheets");
        XSpreadsheets xSpreadsheets = xSheetDoc.getSheets();

        log.println("getting a sheet");
        XSpreadsheet oSheet = null;
        XIndexAccess oIndexAccess = UnoRuntime.queryInterface(XIndexAccess.class, xSpreadsheets);
        try {
            oSheet = (XSpreadsheet) AnyConverter.toObject(
                    new Type(XSpreadsheet.class),oIndexAccess.getByIndex(0));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get a spreadsheet", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get a spreadsheet", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get a spreadsheet", e);
        }

        try {
            log.println("Filing a table");
            for (int i = 1; i < 4; i++) {
                oSheet.getCellByPosition(i, 0).setFormula("Col" + i);
                oSheet.getCellByPosition(0, i).setFormula("Row" + i);
            }

            for (int i = 1; i < 4; i++)
                for (int j = 1; j < 4; j++) {
                    oSheet.getCellByPosition(i, j).setValue(i * (j + 1));
                }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't fill some cells", e);
        }

        XDataPilotTablesSupplier DPTS = UnoRuntime.queryInterface(XDataPilotTablesSupplier.class, oSheet);

        log.println("Getting test object ") ;

        XDataPilotTables DPT = DPTS.getDataPilotTables();
        XDataPilotDescriptor DPDsc = DPT.createDataPilotDescriptor();
        DPDsc.setSourceRange(new CellRangeAddress((short)0, 0, 0, 4, 4));
        DPT.insertNewByName(
            "DataPilotTable",
            new CellAddress((short)0, 5, 5),
            DPDsc);

        oObj = DPDsc.getDataPilotFields();
        log.println("Creating object - " +
                                    ((oObj == null) ? "FAILED" : "OK"));

        TestEnvironment tEnv = new TestEnvironment( oObj );

        // Other parameters required for interface tests

        return tEnv;
    }

}
