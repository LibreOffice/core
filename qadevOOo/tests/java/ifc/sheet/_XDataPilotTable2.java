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


package ifc.sheet;

import java.util.ArrayList;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNamed;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.sheet.*;
import com.sun.star.table.CellAddress;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XCell;
import com.sun.star.table.XCellCursor;
import com.sun.star.table.XCellRange;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;
import util.utils;

/**
 * Testing <code>com.sun.star.sheet.XDataPilotTable2</code>
 * interface methods :
 * <ul>
 *  <li><code> getDrillDownData()</code><li>
 *  <li><code> getPositionData()</code></li>
 *  <li><code> insertDrillDownSheet()</code></li>
 *  <li><code> getOutputRangeByType</code></li>
 * </ul>
 *
 * @see com.sun.star.sheet.XDataPilotTable2
 * @see com.sun.star.table.CellAddress
 *
 */
public class _XDataPilotTable2 extends MultiMethodTest
{
    private XSpreadsheetDocument xSheetDoc = null;
    private XDataPilotTable2 xDPTab2 = null;
    private CellRangeAddress mRangeWhole = null;
    private CellRangeAddress mRangeTable = null;
    private CellRangeAddress mRangeResult = null;
    private ArrayList<Integer> mDataFieldDims = null;
    private ArrayList<CellAddress> mResultCells = null;

    public XDataPilotTable2 oObj;

    /**
     * exception to be thrown when obtaining a result data for a cell fails
     * (probably because the cell is not a result cell).
     */
    private static class ResultCellFailure extends com.sun.star.uno.Exception {}

    @Override
    protected void before()
    {
        Object o = tEnv.getObjRelation("DATAPILOTTABLE2");
        xDPTab2 = UnoRuntime.queryInterface(
            XDataPilotTable2.class, o);

        if (xDPTab2 == null)
            throw new StatusException(Status.failed("Relation not found"));

        xSheetDoc = (XSpreadsheetDocument)tEnv.getObjRelation("SHEETDOCUMENT");

        getOutputRanges();
        buildDataFields();
        try
        {
            buildResultCells();
        }
        catch (ResultCellFailure e)
        {
            e.printStackTrace(log);
            throw new StatusException( "Failed to build result cells.", e);
        }
    }

    public void _getDrillDownData()
    {
        boolean testResult = true;
        int cellCount = mResultCells.size();
        for (int i = 0; i < cellCount; ++i)
        {
            CellAddress addr = mResultCells.get(i);
            DataPilotTablePositionData posData = xDPTab2.getPositionData(addr);
            DataPilotTableResultData resData = (DataPilotTableResultData)posData.PositionData;
            int dim = mDataFieldDims.get(resData.DataFieldIndex).intValue();
            DataResult res = resData.Result;
            double val = res.Value;

            Object[][] data = xDPTab2.getDrillDownData(addr);
            double sum = 0.0;
            if (data.length > 1)
            {
                for (int row = 1; row < data.length; ++row)
                {
                    Object o = data[row][dim];
                    if (AnyConverter.isDouble(o))
                        sum += ((Double)o).doubleValue();
                }
            }
            log.println(formatCell(addr) + ": " + data.length + " rows (" + (data.length-1) + " records)");

            if (!utils.approxEqual(val, sum))
                testResult = false;
        }
        tRes.tested("getDrillDownData()", testResult);
    }

    public void _getPositionData()
    {
        boolean testResult = false;

        do
        {
            CellAddress addr = new CellAddress();
            addr.Sheet  = mRangeTable.Sheet;

            boolean rangeGood = true;
            for (int x = mRangeTable.StartColumn; x <= mRangeTable.EndColumn && rangeGood; ++x)
            {
                for (int y = mRangeTable.StartRow; y <= mRangeTable.EndRow && rangeGood; ++y)
                {
                    addr.Column = x;
                    addr.Row = y;
                    log.println("checking " + formatCell(addr));
                    DataPilotTablePositionData posData = xDPTab2.getPositionData(addr);
                    if (posData.PositionType == DataPilotTablePositionType.NOT_IN_TABLE)
                    {
                        log.println("specified cell address not in table: " + formatCell(addr));
                        rangeGood = false;
                        continue;
                    }

                    switch (posData.PositionType)
                    {
                    case DataPilotTablePositionType.NOT_IN_TABLE:
                        break;
                    case DataPilotTablePositionType.COLUMN_HEADER:
                        printHeaderData(posData);
                        break;
                    case DataPilotTablePositionType.ROW_HEADER:
                        printHeaderData(posData);
                        break;
                    case DataPilotTablePositionType.RESULT:
                        printResultData(posData);
                        break;
                    case DataPilotTablePositionType.OTHER:
                        break;
                    default:
                        log.println("unknown position");
                    }
                }
            }

            if (!rangeGood)
            {
                log.println("table range check failed");
                break;
            }

            testResult = true;
        }
        while (false);

        tRes.tested("getPositionData()", testResult);
    }

    public void _insertDrillDownSheet()
    {
        boolean testResult = true;
        int cellCount = mResultCells.size();
        XSpreadsheets xSheets = xSheetDoc.getSheets();
        XIndexAccess xIA = UnoRuntime.queryInterface(
            XIndexAccess.class, xSheets);
        int sheetCount = xIA.getCount();
        for (int i = 0; i < cellCount && testResult; ++i)
        {
            CellAddress addr = mResultCells.get(i);

            Object[][] data = xDPTab2.getDrillDownData(addr);

            // sheet is always inserted at the current sheet position.
            xDPTab2.insertDrillDownSheet(addr);

            int newSheetCount = xIA.getCount();
            if (newSheetCount == sheetCount + 1)
            {
                log.println("drill-down sheet for " + formatCell(addr) + " inserted");
                if (data.length < 2)
                {
                    // There is no data for this result.  It should never have
                    // inserted a drill-down sheet.
                    log.println("new sheet inserted; however, there is no data for this result");
                    testResult = false;
                    continue;
                }

                // Retrieve the object of the sheet just inserted.
                XSpreadsheet xSheet = null;
                try
                {
                    xSheet = UnoRuntime.queryInterface(
                        XSpreadsheet.class, xIA.getByIndex(addr.Sheet));
                }
                catch (com.sun.star.uno.Exception e)
                {
                    e.printStackTrace();
                    throw new StatusException("Failed to get the spreadsheet object.", e);
                }

                // Check the integrity of the data on the inserted sheet.
                if (!checkDrillDownSheetContent(xSheet, data))
                {
                    log.println("dataintegrity check on the inserted sheet failed");
                    testResult = false;
                    continue;
                }

                log.println("  sheet data integrity check passed");

                // Remove the sheet just inserted.

                XNamed xNamed = UnoRuntime.queryInterface(XNamed.class, xSheet);
                String name = xNamed.getName();
                try
                {
                    xSheets.removeByName(name);
                }
                catch (com.sun.star.uno.Exception e)
                {
                    e.printStackTrace();
                    throw new StatusException("Failed to removed the inserted sheet named " + name + ".", e);
                }
            }
            else if (newSheetCount == sheetCount)
            {
                if (data.length > 1)
                {
                    // There is data for this result.  It should have inserted
                    // a new sheet.
                    log.println("no new sheet is inserted, despite the data being present.");
                    testResult = false;
                }
            }
            else
            {
                log.println("what just happened!?");
                testResult = false;
            }
        }

        tRes.tested("insertDrillDownSheet()", testResult);
    }

    public void _getOutputRangeByType()
    {
        boolean testResult = false;

        do
        {
            // Let's make sure this doesn't cause a crash.  A range returned for an
            // out-of-bound condition is undefined.
            try
            {
                xDPTab2.getOutputRangeByType(-1);
                log.println("exception not raised");
                break;
            }
            catch (IllegalArgumentException e)
            {
                log.println("exception raised on invalid range type (good)");
            }

            try
            {
                xDPTab2.getOutputRangeByType(100);
                log.println("exception not raised");
                break;
            }
            catch (IllegalArgumentException e)
            {
                log.println("exception raised on invalid range type (good)");
            }

            // Check to make sure the whole range is not empty.
            if (mRangeWhole.EndColumn - mRangeWhole.StartColumn <= 0 ||
                mRangeWhole.EndRow - mRangeWhole.EndColumn <= 0)
            {
                log.println("whole range is empty");
                break;
            }

            log.println("whole range is not empty (good)");

            // Table range must be of equal width with the whole range, and the same
            // bottom.
            if (mRangeTable.Sheet != mRangeWhole.Sheet ||
                mRangeTable.StartColumn != mRangeWhole.StartColumn ||
                mRangeTable.EndColumn != mRangeWhole.EndColumn ||
                mRangeTable.EndRow != mRangeWhole.EndRow)
            {
                log.println("table range is incorrect");
                break;
            }

            log.println("table range is correct");

            // Result range must be smaller than the table range, and must share the
            // same lower-right corner.
            if (mRangeResult.Sheet != mRangeTable.Sheet ||
                mRangeResult.StartColumn < mRangeTable.StartColumn ||
                mRangeResult.StartRow < mRangeTable.StartRow ||
                mRangeResult.EndColumn != mRangeTable.EndColumn ||
                mRangeResult.EndRow != mRangeTable.EndRow)
                break;

            log.println("result range is correct");

            testResult = true;
        }
        while (false);

        tRes.tested("getOutputRangeByType()", testResult);
    }

    private void printHeaderData(DataPilotTablePositionData posData)
    {
        DataPilotTableHeaderData header = (DataPilotTableHeaderData)posData.PositionData;
        String posType = "";
        if (posData.PositionType == DataPilotTablePositionType.COLUMN_HEADER)
            posType = "column header";
        else if (posData.PositionType == DataPilotTablePositionType.ROW_HEADER)
            posType = "row header";

        log.println(posType + "; member name: " + header.MemberName + "; dimension: " +
                    header.Dimension + "; hierarchy: " + header.Hierarchy +
                    "; level: " + header.Level);
    }

    private void printResultData(DataPilotTablePositionData posData)
    {
        DataPilotTableResultData resultData = (DataPilotTableResultData)posData.PositionData;
        int dataId = resultData.DataFieldIndex;
        DataResult res = resultData.Result;
        double val = res.Value;
        int flags = res.Flags;
        int filterCount = resultData.FieldFilters.length;
        log.println("result; data field index: " + dataId + "; value: " + val + "; flags: " + flags +
                    "; filter count: " + filterCount);

        for (int i = 0; i < filterCount; ++i)
        {
            DataPilotFieldFilter fil = resultData.FieldFilters[i];
            log.println("  field name: " + fil.FieldName + "; match value: " + fil.MatchValue);
        }
    }

    private String formatCell(CellAddress addr)
    {
        String str = "(" + addr.Column + "," + addr.Row + ")";
        return str;
    }

    private void printRange(String text, CellRangeAddress rangeAddr)
    {
        log.println(text + ": (" + rangeAddr.StartColumn + "," + rangeAddr.StartRow + ") - (" +
                    rangeAddr.EndColumn + "," + rangeAddr.EndRow + ")");
    }

    private void buildResultCells() throws ResultCellFailure
    {
        if (mResultCells != null)
            return;

        getOutputRanges();

        mResultCells = new ArrayList<CellAddress>();
        for (int x = mRangeResult.StartColumn; x <= mRangeResult.EndColumn; ++x)
        {
            for (int y = mRangeResult.StartRow; y <= mRangeResult.EndRow; ++y)
            {
                CellAddress addr = new CellAddress();
                addr.Sheet = mRangeResult.Sheet;
                addr.Column = x;
                addr.Row = y;
                DataPilotTablePositionData posData = xDPTab2.getPositionData(addr);
                if (posData.PositionType != DataPilotTablePositionType.RESULT)
                {
                    log.println(formatCell(addr) + ": this is not a result cell");
                    throw new ResultCellFailure();
                }
                mResultCells.add(addr);
            }
        }
    }

    private void buildDataFields()
    {
        mDataFieldDims = new ArrayList<Integer>();
        XDataPilotDescriptor xDesc = UnoRuntime.queryInterface(
            XDataPilotDescriptor.class, xDPTab2);

        XIndexAccess xFields = xDesc.getDataPilotFields();
        int fieldCount = xFields.getCount();
        for (int i = 0; i < fieldCount; ++i)
        {
            try
            {
                Object field = xFields.getByIndex(i);
                XPropertySet propSet = UnoRuntime.queryInterface(
                    XPropertySet.class, field);
                DataPilotFieldOrientation orient =
                    (DataPilotFieldOrientation)propSet.getPropertyValue("Orientation");
                if (orient == DataPilotFieldOrientation.DATA)
                {
                    Integer item = Integer.valueOf(i);
                    mDataFieldDims.add(item);
                }
            }
            catch (com.sun.star.uno.Exception e)
            {
                e.printStackTrace(log);
                throw new StatusException( "Failed to get a field.", e);
            }
        }
    }

    private void getOutputRanges()
    {
        if (mRangeWhole != null && mRangeTable != null && mRangeResult != null)
            return;

        try
        {
            mRangeWhole = xDPTab2.getOutputRangeByType(DataPilotOutputRangeType.WHOLE);
            printRange("whole range ", mRangeWhole);
            mRangeTable = xDPTab2.getOutputRangeByType(DataPilotOutputRangeType.TABLE);
            printRange("table range ", mRangeTable);
            mRangeResult = xDPTab2.getOutputRangeByType(DataPilotOutputRangeType.RESULT);
            printRange("result range", mRangeResult);
        }
        catch (IllegalArgumentException e)
        {
            e.printStackTrace(log);
            throw new StatusException( "Failed to get output range by type.", e);
        }
    }

    private boolean checkDrillDownSheetContent(XSpreadsheet xSheet, Object[][] data)
    {
        CellAddress lastCell = getLastUsedCellAddress(xSheet, 0, 0);
        if (data.length <= 0 || lastCell.Row == 0 || lastCell.Column == 0)
        {
            log.println("empty data condition");
            return false;
        }

        if (data.length != lastCell.Row + 1 || data[0].length != lastCell.Column + 1)
        {
            log.println("data size differs");
            return false;
        }

        XCellRange xCR = null;
        try
        {
            xCR = xSheet.getCellRangeByPosition(0, 0, lastCell.Column, lastCell.Row);
        }
        catch (com.sun.star.lang.IndexOutOfBoundsException e)
        {
            return false;
        }

        XCellRangeData xCRD = UnoRuntime.queryInterface(
            XCellRangeData.class, xCR);

        Object[][] sheetData = xCRD.getDataArray();
        for (int x = 0; x < sheetData.length; ++x)
        {
            for (int y = 0; y < sheetData[x].length; ++y)
            {
                Object cell1 = sheetData[x][y];
                Object cell2 = data[x][y];
                if (AnyConverter.isString(cell1) && AnyConverter.isString(cell2))
                {
                    String s1 = (String)cell1, s2 = (String)(cell2);
                    if (!s1.equals(s2))
                    {
                        log.println("string cell values differ");
                        return false;
                    }
                }
                else if (AnyConverter.isDouble(cell1) && AnyConverter.isDouble(cell2))
                {
                    double f1 = 0.0, f2 = 0.0;
                    try
                    {
                        f1 = AnyConverter.toDouble(cell1);
                        f2 = AnyConverter.toDouble(cell2);
                    }
                    catch (com.sun.star.lang.IllegalArgumentException e)
                    {
                        log.println("failed to convert cells to double");
                        return false;
                    }

                    if (f1 != f2)
                    {
                        log.println("numerical cell values differ");
                        return false;
                    }
                }
                else
                {
                    log.println("cell types differ");
                    return false;
                }
            }
        }

        return true;
    }

    private CellAddress getLastUsedCellAddress(XSpreadsheet xSheet, int nCol, int nRow)
    {
        try
        {
            XCellRange xRng = xSheet.getCellRangeByPosition(nCol, nRow, nCol, nRow);
            XSheetCellRange xSCR = UnoRuntime.queryInterface(
                XSheetCellRange.class, xRng);

            XSheetCellCursor xCursor = xSheet.createCursorByRange(xSCR);
            XCellCursor xCellCursor = UnoRuntime.queryInterface(
                XCellCursor.class, xCursor);

            xCellCursor.gotoEnd();
            XCell xCell = xCursor.getCellByPosition(0, 0);
            XCellAddressable xCellAddr = UnoRuntime.queryInterface(
                XCellAddressable.class, xCell);

            return xCellAddr.getCellAddress();
        }
        catch (com.sun.star.lang.IndexOutOfBoundsException ex)
        {
        }
        return null;
    }
}

