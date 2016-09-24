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
import util.ValueComparer;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import ifc.sheet._XCellRangesQuery;


/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.SheetCellRange</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::table::CellProperties</code></li>
*  <li> <code>com::sun::star::util::XMergeable</code></li>
*  <li> <code>com::sun::star::sheet::XCellRangeAddressable</code></li>
*  <li> <code>com::sun::star::sheet::XArrayFormulaRange</code></li>
*  <li> <code>com::sun::star::chart::XChartData</code></li>
*  <li> <code>com::sun::star::table::XColumnRowRange</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::style::CharacterProperties</code></li>
*  <li> <code>com::sun::star::sheet::XSheetOperation</code></li>
*  <li> <code>com::sun::star::table::XCellRange</code></li>
*  <li> <code>com::sun::star::style::ParagraphProperties</code></li>
*  <li> <code>com::sun::star::sheet::SheetCellRange</code></li>
* </ul>
* @see com.sun.star.sheet.SheetCellRange
* @see com.sun.star.table.CellProperties
* @see com.sun.star.util.XMergeable
* @see com.sun.star.sheet.XCellRangeAddressable
* @see com.sun.star.sheet.XArrayFormulaRange
* @see com.sun.star.chart.XChartData
* @see com.sun.star.table.XColumnRowRange
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.style.CharacterProperties
* @see com.sun.star.sheet.XSheetOperation
* @see com.sun.star.table.XCellRange
* @see com.sun.star.style.ParagraphProperties
* @see com.sun.star.sheet.SheetCellRange
* @see ifc.table._CellProperties
* @see ifc.util._XMergeable
* @see ifc.sheet._XCellRangeAddressable
* @see ifc.sheet._XArrayFormulaRange
* @see ifc.chart._XChartData
* @see ifc.table._XColumnRowRange
* @see ifc.beans._XPropertySet
* @see ifc.style._CharacterProperties
* @see ifc.sheet._XSheetOperation
* @see ifc.table._XCellRange
* @see ifc.style._ParagraphProperties
* @see ifc.sheet._SheetCellRange
*/
public class ScCellRangeObj extends TestCase {
    XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    @Override
    protected void initialize(TestParameters tParam, PrintWriter log) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF());

        log.println("creating a Spreadsheet document");
        xSheetDoc = SOF.createCalcDoc(null);
    }

    /**
    * Disposes Spreadsheet document.
    */
    @Override
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xSheetDoc ");

        XComponent oComp = UnoRuntime.queryInterface(
                                   XComponent.class, xSheetDoc);
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Retrieves a collection of spreadsheets from a document,
    * and takes one of them. Retrieves some cell range from the spreadsheet.
    * The retrieved cell range is instance of the service
    * <code>com.sun.star.sheet.SheetCellRange</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'SHEET'</code> for
    *      {@link ifc.sheet._XArrayFormulaRange} (the spreadsheet which the cell
    *      range was retrieved from)</li>
    * </ul>
    * @see com.sun.star.sheet.XSpreadsheet
    */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param,
                                                    PrintWriter log) throws Exception {
        XInterface oObj = null;
        XCellRange testRange;


        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println("Creating a test environment");

        XSpreadsheets oSpreadsheets = UnoRuntime.queryInterface(
                                               XSpreadsheetDocument.class,
                                               xSheetDoc).getSheets();
        XNameAccess oNames = UnoRuntime.queryInterface(
                                     XNameAccess.class, oSpreadsheets);

        XSpreadsheet oSheet = null;

        oSheet = (XSpreadsheet) AnyConverter.toObject(
                         new Type(XSpreadsheet.class),
                         oNames.getByName(oNames.getElementNames()[0]));

        oObj = oSheet.getCellRangeByPosition(0, 0, 3, 4);
        testRange = UnoRuntime.queryInterface(
                            XCellRange.class, oObj);

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("SHEET", oSheet);
        // add expected results for the XCellRangesQuery interface test
        String[]expectedResults = new String[7];
        expectedResults[_XCellRangesQuery.QUERYCOLUMNDIFFERENCES] = "Sheet1.B1:C1;Sheet1.B3";
        expectedResults[_XCellRangesQuery.QUERYCONTENTCELLS] = "Sheet1.B2:B3";
        expectedResults[_XCellRangesQuery.QUERYEMPTYCELLS] = "Sheet1.A1 ... Sheet1.B1 ... Sheet1.B5 ... Sheet1.C3 ... Sheet1.D1";
        expectedResults[_XCellRangesQuery.QUERYFORMULACELLS] = "Sheet1.C2";
        expectedResults[_XCellRangesQuery.QUERYINTERSECTION] = "Sheet1.D4";
        expectedResults[_XCellRangesQuery.QUERYROWDIFFERENCES] = "Sheet1.A2:A4;Sheet1.C2:D4";
        expectedResults[_XCellRangesQuery.QUERYVISIBLECELLS] = "Sheet1.A2";
        tEnv.addObjRelation("XCellRangesQuery.EXPECTEDRESULTS", expectedResults);
        tEnv.addObjRelation("XCellRangesQuery.CREATEENTRIES", Boolean.TRUE);

        XPropertySet PropSet = UnoRuntime.queryInterface(
                                       XPropertySet.class, oObj);
        tEnv.addObjRelation("PropSet", PropSet);

        // XSearchable: Add a cell to make a searchable entry
        try {
            tEnv.addObjRelation("XSearchable.MAKEENTRYINCELL", new XCell[] {
                        testRange.getCellByPosition(0,0), testRange.getCellByPosition(0,1)});
        }
        catch(com.sun.star.lang.IndexOutOfBoundsException e){
            e.printStackTrace(log);
            log.println("Cannot make required object relation 'XSearchable.MAKEENTRYINCELL'.");
        }

        // XCellRangeData
/*        Object[][] newData = new Object[5][4];
        for (int i=0; i<newData.length; i++) {
            for (int j=0; j<newData[i].length; j++) {
                newData[i][j] = new Double(i*10+j);
            }
        }
        tEnv.addObjRelation("NewData", newData); */

        //Adding relation for util.XSortable
        final PrintWriter finalLog = log;
        final XCellRange oTable = testRange;
        tEnv.addObjRelation("SORTCHECKER",
                            new ifc.util._XSortable.XSortChecker() {
            PrintWriter out = finalLog;

            public void setPrintWriter(PrintWriter log) {
                out = log;
            }

            public void prepareToSort() {
                try {
                    oTable.getCellByPosition(0, 0).setValue(4);
                    oTable.getCellByPosition(0, 1).setFormula("b");
                    oTable.getCellByPosition(0, 2).setValue(3);
                    oTable.getCellByPosition(0, 3).setValue(23);
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    out.println("Exception while checking sort");
                }
            }

            public boolean checkSort(boolean isSortNumbering,
                                     boolean isSortAscending) {
                out.println("Sort checking...");

                boolean res = false;
                String[] value = new String[4];

                for (int i = 0; i < 4; i++) {
                    try {
                        XCell cell = oTable.getCellByPosition(0, i);
                        value[i] = cell.getFormula();
                    } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                        out.println("Exception while checking sort");
                    }
                }

                if (isSortNumbering) {
                    if (isSortAscending) {
                        out.println("Sorting ascending");

                        String[] rightVal = { "3", "4", "23", "b" };
                        String[] vals = { value[0], value[1], value[2], value[3] };
                        res = ValueComparer.equalValue(vals, rightVal);
                        out.println("Expected 3, 4, 23, b");
                        out.println("getting: " + value[0] + ", " +
                                        value[1] + ", " + value[2] + ", " +
                                        value[3]);
                    } else {
                        String[] rightVal = { "b", "23", "4", "3" };
                        String[] vals = { value[0], value[1], value[2], value[3] };
                        res = ValueComparer.equalValue(vals, rightVal);
                        out.println("Expected b, 23, 4, 3");
                        out.println("getting: " + value[0] + ", " +
                                        value[1] + ", " + value[2] + ", " +
                                        value[3]);
                    }
                } else {
                    if (isSortAscending) {
                        String[] rightVal = { "3", "4", "23", "b" };
                        res = ValueComparer.equalValue(value, rightVal);
                        out.println("Expected 3, 4, 23, b");
                        out.println("getting: " + value[0] + ", " +
                                        value[1] + ", " + value[2] + ", " +
                                        value[3]);
                    } else {
                        String[] rightVal = { "b", "23", "4", "3" };
                        res = ValueComparer.equalValue(value, rightVal);
                        out.println("Expected b, 23, 4, 3");
                        out.println("getting: " + value[0] + ", " +
                                        value[1] + ", " + value[2] + ", " +
                                        value[3]);
                    }
                }

                if (res) {
                    out.println("Sorted correctly");
                } else {
                    out.println("Sorted uncorrectly");
                }

                return res;
            }
        });

        return tEnv;
    }
} // finish class ScCellRangeObj
