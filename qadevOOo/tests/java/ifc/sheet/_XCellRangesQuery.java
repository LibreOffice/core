/*************************************************************************
 *
 *  $RCSfile: _XCellRangesQuery.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-11-18 16:24:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
package ifc.sheet;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.XPropertySet;
import com.sun.star.sheet.CellFlags;
import com.sun.star.sheet.FormulaResult;
import com.sun.star.sheet.XCellRangesQuery;
import com.sun.star.sheet.XSheetCellRanges;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.table.CellAddress;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XColumnRowRange;
import com.sun.star.table.XTableColumns;
import com.sun.star.table.XTableRows;
import com.sun.star.uno.UnoRuntime;

/**
 * Test the XCellRangesQuery interface.
 * Needed object relations:
 * <ul>
 *   <li>"SHEET": an XSpreadSheet object
 *   </li>
 *   <li>"XCellRangesQuery.EXPECTEDRESULTS": the expected results for the test
 *       methods as a String array.<br>
 *       @see mod._sc.ScCellCurserObj or
 *       @see mod._sc.ScCellObj for an example how this should look like.
 *   </li>
 * </ul>
 */
public class _XCellRangesQuery extends MultiMethodTest {
    public XCellRangesQuery oObj;
    protected XSpreadsheet oSheet;
    protected XTableRows oRows;
    protected XTableColumns oColumns;
    protected String[] mExpectedResults = null;
    protected boolean bMakeEntriesAndDispose = false;
    String getting = "";
    String expected = "";
    // provide the object with constants to fill the expected results array
    public static final int QUERYCOLUMNDIFFERENCES = 0;
    public static final int QUERYCONTENTCELLS = 1;
    public static final int QUERYEMPTYCELLS = 2;
    public static final int QUERYFORMULACELLS = 3;
    public static final int QUERYINTERSECTION = 4;
    public static final int QUERYROWDIFFERENCES = 5;
    public static final int QUERYVISIBLECELLS = 6;

    protected void before() {
        oSheet = (XSpreadsheet) tEnv.getObjRelation("SHEET");

        if (oSheet == null) {
            log.println("Object relation oSheet is missing");
            log.println("Trying to query the needed Interface");
            oSheet = (XSpreadsheet) UnoRuntime.queryInterface(
                             XSpreadsheet.class, tEnv.getTestObject());

            if (oSheet == null) {
                throw new StatusException(Status.failed(
                                                  "Object relation oSheet is missing"));
            }
        }

        // expected results
        mExpectedResults = (String[])tEnv.getObjRelation(
                                            "XCellRangesQuery.EXPECTEDRESULTS");

        XColumnRowRange oColumnRowRange = (XColumnRowRange) UnoRuntime.queryInterface(
                                                  XColumnRowRange.class,
                                                  oSheet);
        oRows = (XTableRows)oColumnRowRange.getRows();
        oColumns = (XTableColumns) oColumnRowRange.getColumns();

        // set this in object if the interface has to make its own settings
        // and the environment has to be disposed: this is necessary for objects
        // that do not make entries on the sheet themselves
        Object o = tEnv.getObjRelation("XCellRangesQuery.CREATEENTRIES");
        if (o != null && o instanceof Boolean) {
            bMakeEntriesAndDispose = ((Boolean)o).booleanValue();
        }
        if(bMakeEntriesAndDispose) {
            oRows.removeByIndex(4, oRows.getCount() - 4);
            oColumns.removeByIndex(4, oColumns.getCount() - 4);

            try {
                oSheet.getCellByPosition(1, 1).setValue(5);
                oSheet.getCellByPosition(1, 2).setValue(15);
                oSheet.getCellByPosition(2, 1).setFormula("=B2+B3");
                oSheet.getCellByPosition(1, 3).setFormula("=B2+B4");
                oSheet.getCellByPosition(3, 2).setFormula("");
                oSheet.getCellByPosition(3, 3).setFormula("");
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                log.println("Couldn't fill cells " + e.getLocalizedMessage());
            }
        }

    }

    /**
     * Tested method returns each cell of each column that is different to the
     * cell in a given row
     */
    public void _queryColumnDifferences() {
        boolean res = true;
        XSheetCellRanges ranges = oObj.queryColumnDifferences(
                                          new CellAddress((short) 0, 1, 1));
        getting = ranges.getRangeAddressesAsString();
        expected = mExpectedResults[QUERYCOLUMNDIFFERENCES];

        if (!getting.startsWith(expected)) {
            log.println("Getting: " + getting);
            log.println("Should have started with: " + expected);
            res = false;
        }

        tRes.tested("queryColumnDifferences()", res);
    }

    /**
     * Tested method returns all cells of a given type, defind in
     * CellFlags
     * @see com.sun.star.sheet.CellFlags
     */
    public void _queryContentCells() {
        boolean res = true;
        XSheetCellRanges ranges = oObj.queryContentCells(
                                          (short) CellFlags.VALUE);
        getting = ranges.getRangeAddressesAsString();
        expected = mExpectedResults[QUERYCONTENTCELLS];

        if (!getting.startsWith(expected)) {
            log.println("Getting: " + getting);
            log.println("Should have started with: " + expected);
            res = false;
        }

        tRes.tested("queryContentCells()", res);
    }

    /**
     * Tested method returns all empty cells of the range
     */
    public void _queryEmptyCells() {
        boolean res = true;
        XSheetCellRanges ranges = oObj.queryEmptyCells();
        getting = ranges.getRangeAddressesAsString();
        expected = mExpectedResults[QUERYEMPTYCELLS];

        int startIndex = 0;
        int endIndex = -5;
        String checkString = null;

        while (endIndex != -1) {
            startIndex = endIndex + 5;
            endIndex = expected.indexOf(" ... ", startIndex);
            if (endIndex == -1) {
                checkString = expected.substring(startIndex);
            }
            else {
                checkString = expected.substring(startIndex, endIndex);
            }
            res &= (getting.indexOf(checkString) > -1);
        }

        if (!res) {
            log.println("Getting: " + getting);
            log.println("Should have contained: " + expected);
        }

        tRes.tested("queryEmptyCells()", res);
    }

    /**
     * Tested method returns all cells of a given type, defind in
     * FormulaResult
     * @see com.sun.star.sheet.FormulaResult
     */
    public void _queryFormulaCells() {
        boolean res = true;
        XSheetCellRanges ranges = oObj.queryFormulaCells(
                                          (short) FormulaResult.VALUE);
        getting = ranges.getRangeAddressesAsString();
        expected = mExpectedResults[QUERYFORMULACELLS];

        if (!getting.equals(expected)) {
            log.println("Getting: " + getting);
            log.println("Expected: " + expected);
            res = false;
        }

        tRes.tested("queryFormulaCells()", res);
    }

    public void _queryIntersection() {
        boolean res = true;
        XSheetCellRanges ranges = oObj.queryIntersection(
                                          new CellRangeAddress((short) 0, 3, 3, 7, 7));
        getting = ranges.getRangeAddressesAsString();
        expected = mExpectedResults[QUERYINTERSECTION];

        if (!getting.startsWith(expected)) {
            log.println("Getting: " + getting);
            log.println("Should have started with: " + expected);
            res = false;
        }

        tRes.tested("queryIntersection()", res);
    }

    /**
     * Tested method returns each cell of each row that is different to the
     * cell in a given column
     */
    public void _queryRowDifferences() {
        boolean res = true;
        XSheetCellRanges ranges = oObj.queryRowDifferences(
                                          new CellAddress((short) 0, 1, 1));
        getting = ranges.getRangeAddressesAsString();
        expected = mExpectedResults[QUERYROWDIFFERENCES];

        if (!getting.startsWith(expected)) {
            log.println("Getting: " + getting);
            log.println("Should have started with: " + expected);
            res = false;
        }

        tRes.tested("queryRowDifferences()", res);
    }

    public void _queryVisibleCells() {
        setRowVisible(false);

        boolean res = true;
        XSheetCellRanges ranges = oObj.queryVisibleCells();
        getting = ranges.getRangeAddressesAsString();
        expected = mExpectedResults[QUERYVISIBLECELLS];

        if (!getting.startsWith(expected)) {
            log.println("Getting: " + getting);
            log.println("Should have started with: " + expected);
            res = false;
        }

        setRowVisible(true);
        tRes.tested("queryVisibleCells()", res);
    }

    protected void setRowVisible(boolean vis) {
        try {
            XPropertySet rowProp = (XPropertySet) UnoRuntime.queryInterface(
                                           XPropertySet.class,
                                           oRows.getByIndex(0));
            rowProp.setPropertyValue("IsVisible", new Boolean(vis));
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("couldn't get Row " + e.getLocalizedMessage());
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("problems setting Property 'isVisible' " +
                        e.getLocalizedMessage());
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("problems setting Property 'isVisible' " +
                        e.getLocalizedMessage());
        } catch (com.sun.star.beans.PropertyVetoException e) {
            log.println("problems setting Property 'isVisible' " +
                        e.getLocalizedMessage());
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("problems setting Property 'isVisible' " +
                        e.getLocalizedMessage());
        }
    }

    /**
    * Forces environment recreation.
    */
    protected void after() {
        if(bMakeEntriesAndDispose) {
            disposeEnvironment();
        }
    }
}