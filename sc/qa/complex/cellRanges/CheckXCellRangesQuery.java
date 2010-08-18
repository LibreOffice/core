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

package complex.cellRanges;

import com.sun.star.container.XIndexAccess;
import complexlib.ComplexTestCase;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.CellFlags;
import com.sun.star.sheet.XCellRangesQuery;
import com.sun.star.sheet.XSheetCellRanges;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.CellAddress;
import com.sun.star.table.XColumnRowRange;
import com.sun.star.table.XTableColumns;
import com.sun.star.table.XTableRows;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import util.SOfficeFactory;

/**
 * Check the XCellRangesQuery interface on the SheetCell service. test was
 * created for bug i20044.
 */
public class CheckXCellRangesQuery extends ComplexTestCase {
    XSpreadsheetDocument m_xSheetDoc = null;
    XCellRangesQuery m_xCell = null;
    XSpreadsheet m_xSpreadSheet = null;

    /**
     * Get all test methods.
     * @return The test methods.
     */
    public String[] getTestMethodNames() {
        return new String[] {"checkEmptyCell", "checkFilledCell"};
    }

    /**
    * Creates Spreadsheet document and the test object,
    * before the actual test starts.
    */
    public void before() {
        // create a calc document
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)param.getMSF() );

        try {
            log.println( "creating a Spreadsheet document" );
            m_xSheetDoc = SOF.createCalcDoc(null);
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( (PrintWriter)log );
            failed( "Couldn?t create document");
        }
        XInterface oObj = null;

        try {
            log.println("Getting spreadsheet") ;
            XSpreadsheets oSheets = m_xSheetDoc.getSheets() ;
            XIndexAccess oIndexSheets = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class, oSheets);
            m_xSpreadSheet = (XSpreadsheet) AnyConverter.toObject(
                    new Type(XSpreadsheet.class),oIndexSheets.getByIndex(0));

            // get the cell
            log.println("Getting a cell from sheet") ;
            oObj = m_xSpreadSheet.getCellByPosition(2, 3);
            m_xCell = (XCellRangesQuery)UnoRuntime.queryInterface(XCellRangesQuery.class, oObj);

        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace((PrintWriter)log);
            failed("Error getting cell object from spreadsheet document");
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace((PrintWriter)log);
            failed("Error getting cell object from spreadsheet document");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace((PrintWriter)log);
            failed("Error getting cell object from spreadsheet document");
        }

        // set one value for comparison.
        try {
            m_xSpreadSheet.getCellByPosition(1, 1).setValue(15);
            m_xSpreadSheet.getCellByPosition(1, 3).setValue(5);
            m_xSpreadSheet.getCellByPosition(2, 1).setFormula("=B2+B4");
/*            m_xSpreadSheet.getCellByPosition(2, 1).setFormula("=B2+B3");
            m_xSpreadSheet.getCellByPosition(3, 2).setFormula("");
            m_xSpreadSheet.getCellByPosition(3, 3).setFormula("");            */
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace((PrintWriter)log);
            failed("Could not fill cell (1, 1) with a value.");
        }

    }

    /**
     * Perform some tests on an empty cell:
     * <ol>
     * <li>compare an empty cell with a cell with a value in the same column</li>
     * <li>compare an empty cell with a cell with a value in the same row</li>
     * <li>query for empty cells</li>
     * <ol>
     */
    public void checkEmptyCell() {
        log.println("Checking an empty cell...");
        // compare an empty cell with a cell with a value
        assure("\tQuery column differences did not return the correct value.", _queryColumnDifferences("Sheet1.C4"), true);
        // compare an empty cell with a cell with a value
        assure("\tQuery column differences did not return the correct value.", _queryRowDifferences("Sheet1.C4"), true);
        // try to get this cell
        assure("\tQuery empty cells did not return the correct value.", _queryEmptyCells("Sheet1.C4"), true);
        log.println("...done");
    }

    /**
     * Perform some tests on a filled cell:
     * <ol>
     * <li>compare an cell with value 5 with a cell with value 15 in the same column</li>
     * <li>compare an cell with value 5 with a cell with value 15 in the same row</li>
     * <li>query for an empty cell.</li>
     * <ol>
     */
    public void checkFilledCell() {
        log.println("Checking a filled cell...");

        // fill the cell with a value
        try {
            m_xSpreadSheet.getCellByPosition(2, 3).setValue(15);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace((PrintWriter)log);
            failed("Could not fill cell (2, 3) with a value.");
        }

        // compare an cell with value 5 with a cell with value 15
        assure("\tQuery column differences did not return the correct value.", _queryColumnDifferences("Sheet1.C4"), true);
        // compare an cell with value 5 with a cell with value 15
        assure("\tQuery column differences did not return the correct value.", _queryRowDifferences("Sheet1.C4"), true);
        // try to get nothing
        assure("\tQuery empty cells did not return the correct value.", _queryEmptyCells(""), true);
        log.println("...done");
    }


    /**
     *  Query column differences between my cell(2,3) and (1,1).
     *  @param expected The expected outcome value.
     *  @return True, if the result equals the expected result.
     */
    public boolean _queryColumnDifferences(String expected) {
        log.println("\tQuery column differences");
        XSheetCellRanges ranges = m_xCell.queryColumnDifferences(
                                          new CellAddress((short) 0, 1, 1));
        String getting = ranges.getRangeAddressesAsString();

        if (!getting.equals(expected)) {
            log.println("\tGetting: " + getting);
            log.println("\tShould have been: " + expected);
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
        log.println("\tQuery empty cells");
        XSheetCellRanges ranges = m_xCell.queryEmptyCells();
        String getting = ranges.getRangeAddressesAsString();

        if (!getting.equals(expected)) {
            log.println("\tGetting: " + getting);
            log.println("\tShould have been: " + expected);
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
        log.println("\tQuery row differences");
        XSheetCellRanges ranges = m_xCell.queryRowDifferences(
                                          new CellAddress((short) 0, 1, 1));
        String getting = ranges.getRangeAddressesAsString();

        if (!getting.equals(expected)) {
            log.println("\tGetting: " + getting);
            log.println("\tShould have been: " + expected);
            return false;
        }

        return true;
    }

}
