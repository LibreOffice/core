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

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XScenariosSupplier;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.CellAddress;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;

import ifc.sheet._XCellRangesQuery;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.SOfficeFactory;
import util.ValueComparer;


/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.Spreadsheet</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::style::ParagraphProperties</code></li>
*  <li> <code>com::sun::star::table::XTableChartsSupplier</code></li>
*  <li> <code>com::sun::star::sheet::XSpreadsheet</code></li>
*  <li> <code>com::sun::star::table::CellProperties</code></li>
*  <li> <code>com::sun::star::sheet::XSheetAnnotationsSupplier</code></li>
*  <li> <code>com::sun::star::table::XCellRange</code></li>
*  <li> <code>com::sun::star::sheet::XCellRangeAddressable</code></li>
*  <li> <code>com::sun::star::sheet::XDataPilotTablesSupplier</code></li>
*  <li> <code>com::sun::star::sheet::Spreadsheet</code></li>
*  <li> <code>com::sun::star::sheet::XCellRangeMovement</code></li>
*  <li> <code>com::sun::star::style::CharacterProperties</code></li>
*  <li> <code>com::sun::star::sheet::XSheetOperation</code></li>
*  <li> <code>com::sun::star::sheet::XScenariosSupplier</code></li>
*  <li> <code>com::sun::star::sheet::XSheetPageBreak</code></li>
*  <li> <code>com::sun::star::sheet::XArrayFormulaRange</code></li>
*  <li> <code>com::sun::star::sheet::XSheetCellRange</code></li>
*  <li> <code>com::sun::star::container::XNamed</code></li>
*  <li> <code>com::sun::star::sheet::SheetCellRange</code></li>
*  <li> <code>com::sun::star::chart::XChartData</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::util::XMergeable</code></li>
*  <li> <code>com::sun::star::table::XColumnRowRange</code></li>
* </ul>
* @see com.sun.star.style.ParagraphProperties
* @see com.sun.star.table.XTableChartsSupplier
* @see com.sun.star.sheet.XSpreadsheet
* @see com.sun.star.table.CellProperties
* @see com.sun.star.sheet.XSheetAnnotationsSupplier
* @see com.sun.star.table.XCellRange
* @see com.sun.star.sheet.XCellRangeAddressable
* @see com.sun.star.sheet.XDataPilotTablesSupplier
* @see com.sun.star.sheet.Spreadsheet
* @see com.sun.star.sheet.XCellRangeMovement
* @see com.sun.star.style.CharacterProperties
* @see com.sun.star.sheet.XSheetOperation
* @see com.sun.star.sheet.XScenariosSupplier
* @see com.sun.star.sheet.XSheetPageBreak
* @see com.sun.star.sheet.XArrayFormulaRange
* @see com.sun.star.sheet.XSheetCellRange
* @see com.sun.star.container.XNamed
* @see com.sun.star.sheet.SheetCellRange
* @see com.sun.star.chart.XChartData
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.util.XMergeable
* @see com.sun.star.table.XColumnRowRange
* @see ifc.style._ParagraphProperties
* @see ifc.table._XTableChartsSupplier
* @see ifc.sheet._XSpreadsheet
* @see ifc.table._CellProperties
* @see ifc.sheet._XSheetAnnotationsSupplier
* @see ifc.table._XCellRange
* @see ifc.sheet._XCellRangeAddressable
* @see ifc.sheet._XDataPilotTablesSupplier
* @see ifc.sheet._Spreadsheet
* @see ifc.sheet._XCellRangeMovement
* @see ifc.style._CharacterProperties
* @see ifc.sheet._XSheetOperation
* @see ifc.sheet._XScenariosSupplier
* @see ifc.sheet._XSheetPageBreak
* @see ifc.sheet._XArrayFormulaRange
* @see ifc.sheet._XSheetCellRange
* @see ifc.container._XNamed
* @see ifc.sheet._SheetCellRange
* @see ifc.chart._XChartData
* @see ifc.beans._XPropertySet
* @see ifc.util._XMergeable
* @see ifc.table._XColumnRowRange
*/
public class ScTableSheetObj extends TestCase {
    private XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    protected void initialize(TestParameters tParam, PrintWriter log) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                     (XMultiServiceFactory) tParam.getMSF());

        try {
            log.println("creating a sheetdocument");
            xSheetDoc = SOF.createCalcDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }
    }

    /**
    * Disposes Spreadsheet document.
    */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xSheetDoc ");

        try {
            XCloseable oCloser = UnoRuntime.queryInterface(
                                         XCloseable.class, xSheetDoc);
            oCloser.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            log.println("Couldn't close document");
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("Document already disposed");
        } catch (java.lang.NullPointerException e) {
            log.println("Couldn't get XCloseable");
        }

        try {
            log.println("Sleeping 500 Millis");
            Thread.sleep(500);
        } catch (InterruptedException ex) {
        }

        log.println("... Done");
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves a collection of spreadsheets from the document and takes one of
    * them. Fills some cell in the spreadsheet. The retrieved spreadsheet is the
    * instance of the service <code>com.sun.star.sheet.Spreadsheet</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'noArray'</code> for
    *      {@link ifc.sheet._XArrayFormulaRange}(to avoid the test of
    *      the interface <code>XArrayFormulaRange</code>)</li>
    * </ul>
    * @see com.sun.star.sheet.XArrayFormulaRange
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param,
                                                                 PrintWriter log) {
        XInterface oObj = null;

        log.println("getting sheets");

        XSpreadsheets xSpreadsheets = xSheetDoc.getSheets();

        log.println("getting a sheet");

        XSpreadsheet oSheet = null;
        XIndexAccess oIndexAccess = UnoRuntime.queryInterface(
                                            XIndexAccess.class, xSpreadsheets);

        try {
            oSheet = (XSpreadsheet) AnyConverter.toObject(
                             new Type(XSpreadsheet.class),
                             oIndexAccess.getByIndex(0));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get a spreadsheet", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get a spreadsheet", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get a spreadsheet", e);
        }

        log.println("filling some cells");

        try {
            oSheet.getCellByPosition(5, 5).setValue(15);
            oSheet.getCellByPosition(1, 4).setValue(10);
            oSheet.getCellByPosition(2, 0).setValue(-5.15);
            oSheet.getCellByPosition(8, 8).setFormula("= B5 + C1");
            // fill cells for XSheetOtline::autoutline
            oSheet.getCellByPosition(6, 6).setValue(3);
            oSheet.getCellByPosition(7, 6).setValue(3);
            oSheet.getCellByPosition(8, 6).setFormula("= SUM(G7:H7)");
            oSheet.getCellByPosition(9, 6).setFormula("= G7*I7");
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException("Exception occurred while filling cells", e);
        }

        oObj = UnoRuntime.queryInterface(XInterface.class, oSheet);

        log.println("creating a new environment for object");

        TestEnvironment tEnv = new TestEnvironment(oObj);

        // do not execute com::sun::star::sheets::XCellSeries::fillAuto()
        tEnv.addObjRelation("XCELLSERIES_FILLAUTO", new Boolean(false));

        // set the address ranges of the cells (see values set above): for e.g. XSheetOutline test
        tEnv.addObjRelation("CellRangeAddress",
            new CellRangeAddress((short)0, 6, 6, 8, 8));
        tEnv.addObjRelation("CellRangeSubAddress",
            new CellRangeAddress((short)0, 6, 6, 7, 8));
        // pick a cell with a formula for XSheetAuditing, a dependent cell and a precedent cell
        tEnv.addObjRelation("XSheetAuditing.CellAddress", new CellAddress((short)0, 8, 6));
        tEnv.addObjRelation("XSheetAuditing.PrecedentCellAddress", new CellAddress((short)0, 7, 6));
        tEnv.addObjRelation("XSheetAuditing.DependentCellAddress", new CellAddress((short)0, 9, 6));

        // add an existing sheet for linking
        tEnv.addObjRelation("XSheetLinkable.LinkSheet", "ScSheetLinksObj.ods");

        //adding Scenario and with that a ScenarioSheet-Relation for Scenario and XScenarioEnhanced
        XScenariosSupplier scene = UnoRuntime.queryInterface(
                                           XScenariosSupplier.class,
                                           tEnv.getTestObject());
        scene.getScenarios()
             .addNewByName("Scenario",
                           new CellRangeAddress[] {
            new CellRangeAddress((short) 0, 0, 0, 10, 10)
        }, "Comment");

        XSpreadsheet sSheet = null;

        try {
            sSheet = UnoRuntime.queryInterface(
                             XSpreadsheet.class,
                             xSpreadsheets.getByName("Scenario"));
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("Couldn't get Scenario");
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Couldn't get Scenario");
        }

        tEnv.addObjRelation("ScenarioSheet", sSheet);

        log.println("adding ObjRelation 'noArray' to avoid the test" +
                    " 'XArrayFormulaRange'");
        tEnv.addObjRelation("noArray", "ScTableSheetObj");

        XPropertySet PropSet = UnoRuntime.queryInterface(
                                       XPropertySet.class, oObj);
        tEnv.addObjRelation("PropSet", PropSet);
        tEnv.addObjRelation("SHEET", oSheet);

        // add expected results for the XCellRangesQuery interface test
        String[] expectedResults = new String[7];
        expectedResults[_XCellRangesQuery.QUERYCOLUMNDIFFERENCES] = "Sheet1.B5;Sheet1.C1";
        expectedResults[_XCellRangesQuery.QUERYCONTENTCELLS] = "Sheet1.B5;Sheet1.C1;Sheet1.F6";
        expectedResults[_XCellRangesQuery.QUERYEMPTYCELLS] = "Sheet1.A1 ... Sheet1.B1 ... Sheet1.B6 ... Sheet1.C2 ... Sheet1.D1 ... Sheet1.F1 ... Sheet1.F7 ... Sheet1.G1";
        expectedResults[_XCellRangesQuery.QUERYFORMULACELLS] = "Sheet1.I7:J7;Sheet1.I9";
        expectedResults[_XCellRangesQuery.QUERYINTERSECTION] = "Sheet1.D4";
        expectedResults[_XCellRangesQuery.QUERYROWDIFFERENCES] = "Sheet1.A5;Sheet1.C1";
        expectedResults[_XCellRangesQuery.QUERYVISIBLECELLS] = "Sheet1.A2";
        tEnv.addObjRelation("XCellRangesQuery.EXPECTEDRESULTS",
                            expectedResults);

        // for XFormulaQuery interface test
        try {
            tEnv.addObjRelation("MAKEENTRYINCELL",
                                oSheet.getCellByPosition(15, 15));
            tEnv.addObjRelation("RANGEINDICES", new int[] { 0, 0 });
            tEnv.addObjRelation("EXPECTEDDEPENDENTVALUES",
                                new int[] { 0, 255, 0, 65535 });
            tEnv.addObjRelation("EXPECTEDPRECEDENTVALUES",
                                new int[] { 0, 255, 0, 65535 });
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println(
                    "Cannot add the necessary object relation for XFormulaQuery test.");
        }

        // XSearchable interface test
        try {
            tEnv.addObjRelation("XSearchable.MAKEENTRYINCELL",
                                new XCell[] {
                oSheet.getCellByPosition(15, 15),
                oSheet.getCellByPosition(15, 16)
            });
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            log.println(
                    "Cannot make required object relation 'XSearchable.MAKEENTRYINCELL'.");
        }

        //Adding relation for util.XSortable
        final PrintWriter finalLog = log;
        final XCellRange oTable = oSheet;
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
}
