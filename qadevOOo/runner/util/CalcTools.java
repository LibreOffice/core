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

package util;

import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.XCellRangeData;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCellRange;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;

/**
 * This class contains some useful mathods to handle Calc documents
 * and its sheets.
 */
public class CalcTools {

    /**
     * fills a range of a calc sheet with computed data of type
     * <CODE>Double</CODE>.
     * @param xSheetDoc the Clac documents wich should be filled
     * @param sheetNumber the number of the sheet of <CODE>xSheetDoc</CODE>
     * @param startCellX the cell number of the X start point (row) of the range to fill
     * @param startCellY the cell number of the Y start point (column) of the range to fill
     * @param rangeLengthX the size of the range expansion in X-direction
     * @param rangeLengthY the size of the range expansion in Y-direction
     * @throws java.lang.Exception on any error an <CODE>java.lang.Exception</CODE> was thrown
     */
    public static void fillCalcSheetWithContent(XComponent xSheetDoc, int sheetNumber,
                        int startCellX, int startCellY, int rangeLengthX, int rangeLengthY)
                  throws java.lang.Exception {
        try{
            XSpreadsheet xSheet = getSpreadSheetFromSheetDoc(xSheetDoc, sheetNumber);

            fillCalcSheetWithContent(xSheet, startCellX, startCellY, rangeLengthX, rangeLengthY);

        } catch (Exception e){
                throw new Exception(
                        "Couldn't fill CalcSheet with content: " + e.toString());
        }
    }

    /**
     * fills a range of a calc sheet with computed data of type
     * <CODE>Double</CODE>.
     * @param xSheet the sheet to fill with content
     * @param startCellX the cell number of the X start point (row) of the range to fill
     * @param startCellY the cell number of the Y start point (column) of the range to fill
     * @param rangeLengthX the size of the range expansion in X-direction
     * @param rangeLengthY the size of the range expansion in Y-direction
     * @throws java.lang.Exception on any error an <CODE>java.lang.Exception</CODE> was thrown
     */
    public static void fillCalcSheetWithContent(XSpreadsheet xSheet,
                        int startCellX, int startCellY, int rangeLengthX, int rangeLengthY)
                  throws java.lang.Exception {

        try{
            // create a range with content
            Object[][] newData = new Object[rangeLengthY][rangeLengthX];
            for (int i=0; i<rangeLengthY; i++) {
                for (int j=0; j<rangeLengthX; j++) {
                    newData[i][j] = new Double(10*i +j);
                }
            }
            XCellRange xRange = null;
            try {
                xRange = xSheet.getCellRangeByPosition(startCellX, startCellY,
                                    startCellX+rangeLengthX-1, startCellY+rangeLengthY-1);
            } catch ( IndexOutOfBoundsException e){
                    throw new Exception(
                            "Couldn't get CellRange from sheett: " + e.toString());
            }

            XCellRangeData xRangeData = UnoRuntime.queryInterface(XCellRangeData.class, xRange);

            xRangeData.setDataArray(newData);
        } catch (Exception e){
                throw new Exception(
                        "Couldn't fill CalcSheet with content: " + e.toString());
        }
    }

    /**
     *
     * returns an <CODE>XSpreadsheet</CODE> from a Calc document.
     * @param xSheetDoc the Calc docuent which containes the sheet
     * @param sheetNumber the number of the sheet to return
     * @throws java.lang.Exception on any error an <CODE>java.lang.Exception</CODE> was thrown
     * @return calc sheet
     * @see com.sun.star.sheet.XSpreadsheet
     */
    public static XSpreadsheet getSpreadSheetFromSheetDoc(XComponent xSheetDoc, int sheetNumber)
                  throws java.lang.Exception {

        XSpreadsheet xSheet = null;

        try{
            XSpreadsheetDocument xSpreadsheetDoc = UnoRuntime.queryInterface(XSpreadsheetDocument.class, xSheetDoc);

            XSpreadsheets xSpreadsheets = xSpreadsheetDoc.getSheets();

            XIndexAccess xSheetsIndexArray = UnoRuntime.queryInterface(XIndexAccess.class, xSpreadsheets);

            try{
                xSheet = (XSpreadsheet) AnyConverter.toObject(
                        new Type(XSpreadsheet.class),xSheetsIndexArray.getByIndex(sheetNumber));

            } catch (IllegalArgumentException e){
                throw new Exception(
                        "Couldn't get sheet '" +sheetNumber + "' : " + e.toString());
            } catch (IndexOutOfBoundsException e){
                throw new Exception(
                        "Couldn't get sheet '" +sheetNumber + "' : " + e.toString());
            } catch (WrappedTargetException e){
                throw new Exception(
                        "Couldn't get sheet '" +sheetNumber + "' : " + e.toString());
            }
        } catch (Exception e){
            throw new Exception(
                "Couldn't get sheet '" +sheetNumber + "' : " + e.toString());
        }
         return xSheet;
    }
}
