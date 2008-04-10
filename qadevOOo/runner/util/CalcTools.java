/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CalcTools.java,v $
 * $Revision: 1.3 $
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
 ************************************************************************
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
 * This class contains some usefull mathods to handle Calc documents
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

            XCellRangeData xRangeData = (XCellRangeData) UnoRuntime.queryInterface(XCellRangeData.class, xRange);

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
            XSpreadsheetDocument xSpreadsheetDoc = (XSpreadsheetDocument)
                    UnoRuntime.queryInterface(XSpreadsheetDocument.class, xSheetDoc);

            XSpreadsheets xSpreadsheets = xSpreadsheetDoc.getSheets();

            XIndexAccess xSheetsIndexArray = (XIndexAccess)
                        UnoRuntime.queryInterface(XIndexAccess.class, xSpreadsheets);

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
