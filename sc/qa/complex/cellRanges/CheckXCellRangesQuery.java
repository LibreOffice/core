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

package complex.cellRanges;

import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XCellRangesQuery;
import com.sun.star.sheet.XSheetCellRanges;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.CellAddress;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;
import util.SOfficeFactory;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

import com.sun.star.container.XNamed;

/**
 * Check the XCellRangesQuery interface on the SheetCell service. test was
 * created for bug i20044.
 */
public class CheckXCellRangesQuery /* extends ComplexTestCase */ {
    XSpreadsheetDocument m_xSheetDoc = null;
    XCellRangesQuery m_xCell = null;
    XSpreadsheet m_xSpreadSheet = null;
    String sSheetName = "";

    /**
    * Creates Spreadsheet document and the test object,
    * before the actual test starts.
    */
    @Before public void before() {
        // create a calc document
        final XMultiServiceFactory xMsf = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        SOfficeFactory SOF = SOfficeFactory.getFactory(xMsf);

        try {
            System.out.println( "creating a Spreadsheet document" );
            m_xSheetDoc = SOF.createCalcDoc(null);
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occurs.FAILED
            e.printStackTrace(  );
            fail( "Couldn?t create document");
        }
        XInterface oObj = null;

        try {
            System.out.println("Getting spreadsheet") ;
            XSpreadsheets oSheets = m_xSheetDoc.getSheets() ;
            XIndexAccess oIndexSheets =
            UnoRuntime.queryInterface(XIndexAccess.class, oSheets);
            m_xSpreadSheet = (XSpreadsheet) AnyConverter.toObject(
                    new Type(XSpreadsheet.class),oIndexSheets.getByIndex(0));

            // get the first sheet name
            XNamed m_xNamed = (XNamed) AnyConverter.toObject(new Type(XNamed.class),m_xSpreadSheet);
            sSheetName = m_xNamed.getName();

            // get the cell
            System.out.println("Getting a cell from sheet") ;
            oObj = m_xSpreadSheet.getCellByPosition(2, 3);
            m_xCell = UnoRuntime.queryInterface(XCellRangesQuery.class, oObj);

        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace();
            fail("Error getting cell object from spreadsheet document");
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace();
            fail("Error getting cell object from spreadsheet document");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace();
            fail("Error getting cell object from spreadsheet document");
        }

        // set one value for comparison.
        try {
            m_xSpreadSheet.getCellByPosition(1, 1).setValue(15);
            m_xSpreadSheet.getCellByPosition(1, 3).setValue(5);
            m_xSpreadSheet.getCellByPosition(2, 1).setFormula("=B2+B4");
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace();
            fail("Could not fill cell (1, 1) with a value.");
        }

    }

       /*
     * this method closes a calc document and resets the corresponding class variable xSheetDoc
     */
    protected boolean closeSpreadsheetDocument() {
        boolean worked = true;

        System.out.println("    disposing xSheetDoc ");

        try {
            XCloseable oCloser =  UnoRuntime.queryInterface(
                                         XCloseable.class, m_xSheetDoc);
            oCloser.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            worked = false;
            System.out.println("Couldn't close document");
        } catch (com.sun.star.lang.DisposedException e) {
            worked = false;
            System.out.println("Document already disposed");
        } catch (java.lang.NullPointerException e) {
            worked = false;
            System.out.println("Couldn't get XCloseable");
        }

        m_xSheetDoc = null;

        return worked;
    }

    @After public void after()
        {
            closeSpreadsheetDocument();
        }

    /**
     * Perform some tests on an empty cell:
     * <ol>
     * <li>compare an empty cell with a cell with a value in the same column</li>
     * <li>compare an empty cell with a cell with a value in the same row</li>
     * <li>query for empty cells</li>
     * <ol>
     */
    @Test public void checkEmptyCell() {
        System.out.println("Checking an empty cell...");
        // compare an empty cell with a cell with a value
        assertTrue("\tQuery column differences did not return the correct value.", _queryColumnDifferences(sSheetName+".C4"));
        // compare an empty cell with a cell with a value
        assertTrue("\tQuery column differences did not return the correct value.", _queryRowDifferences(sSheetName+".C4"));
        System.out.println("...done");
    }

    /**
     * Perform some tests on a filled cell:
     * <ol>
     * <li>compare an cell with value 5 with a cell with value 15 in the same column</li>
     * <li>compare an cell with value 5 with a cell with value 15 in the same row</li>
     * <li>query for an empty cell.</li>
     * <ol>
     */
    @Test public void checkFilledCell() {
        System.out.println("Checking a filled cell...");

        // fill the cell with a value
        try {
            m_xSpreadSheet.getCellByPosition(2, 3).setValue(15);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace();
            fail("Could not fill cell (2, 3) with a value.");
        }

        // compare an cell with value 5 with a cell with value 15
        assertTrue("\tQuery column differences did not return the correct value.", _queryColumnDifferences(sSheetName + ".C4"));
        // compare an cell with value 5 with a cell with value 15
        assertTrue("\tQuery column differences did not return the correct value.", _queryRowDifferences(sSheetName+".C4"));
        // try to get nothing
        assertTrue("\tQuery empty cells did not return the correct value.", _queryEmptyCells(""));
        System.out.println("...done");
    }


    /**
     *  Query column differences between my cell(2,3) and (1,1).
     *  @param expected The expected outcome value.
     *  @return True, if the result equals the expected result.
     */
    public boolean _queryColumnDifferences(String expected) {
        System.out.println("\tQuery column differences");
        XSheetCellRanges ranges = m_xCell.queryColumnDifferences(
                                          new CellAddress((short) 0, 1, 1));
        String getting = ranges.getRangeAddressesAsString();

        if (!getting.equals(expected)) {
            System.out.println("\tGetting: " + getting);
            System.out.println("\tShould have been: " + expected);
            return false;
        }
        return true;
    }

    /**
     * Query for an empty cell.
     *  @param expected The expected outcome value.
     *  @return True, if the result equals the expected result.
     */
    public boolean _queryEmptyCells(String expected) {
        System.out.println("\tQuery empty cells");
        XSheetCellRanges ranges = m_xCell.queryEmptyCells();
        String getting = ranges.getRangeAddressesAsString();

        if (!getting.equals(expected)) {
            System.out.println("\tGetting: " + getting);
            System.out.println("\tShould have been: " + expected);
            return false;
        }
        return true;
    }

    /**
     *  Query row differences between my cell(2,3) and (1,1).
     *  @param expected The expected outcome value.
     *  @return True, if the result equals the expected result.
     */
    public boolean _queryRowDifferences(String expected) {
        System.out.println("\tQuery row differences");
        XSheetCellRanges ranges = m_xCell.queryRowDifferences(
                                          new CellAddress((short) 0, 1, 1));
        String getting = ranges.getRangeAddressesAsString();

        if (!getting.equals(expected)) {
            System.out.println("\tGetting: " + getting);
            System.out.println("\tShould have been: " + expected);
            return false;
        }

        return true;
    }


    @BeforeClass public static void setUpConnection() throws Exception {
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();

}
