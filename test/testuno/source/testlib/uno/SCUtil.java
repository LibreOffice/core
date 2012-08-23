/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


package testlib.uno;

import java.util.HashMap;

import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.Testspace;
import org.openoffice.test.uno.UnoApp;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheetView;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;
import com.sun.star.table.XColumnRowRange;
import com.sun.star.table.XTableColumns;
import com.sun.star.table.XTableRows;
import com.sun.star.text.XText;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCloseable;


/**
 * Utilities of Spreadsheet
 *
 */

public class SCUtil {

    private static final String scTempDir = "output/sc/"; //Spreadsheet temp file directory
    private static HashMap filterName = new HashMap();

    private SCUtil() {

    }

    /**
     * Get spreadsheet document object
     * @param xSpreadsheetComponent
     * @return
     * @throws Exception
     */
    public static XSpreadsheetDocument getSCDocument(XComponent xSpreadsheetComponent) throws Exception {
        XSpreadsheetDocument xSpreadsheetDocument =
                (XSpreadsheetDocument) UnoRuntime.queryInterface(XSpreadsheetDocument.class, xSpreadsheetComponent);

        return xSpreadsheetDocument;
    }

    /**
     * Get sheet object by sheet name
     * @param xSpreadsheetDocument
     * @param sheetName
     * @return
     * @throws Exception
     */
    public static XSpreadsheet getSCSheetByName(XSpreadsheetDocument xSpreadsheetDocument, String sheetName) throws Exception {
        XSpreadsheets xSpreadsheets = xSpreadsheetDocument.getSheets();
        XSpreadsheet xSpreadsheet =
                (XSpreadsheet) UnoRuntime.queryInterface(XSpreadsheet.class, xSpreadsheets.getByName(sheetName));

        return xSpreadsheet;
    }

    /**
     * Get sheet object by sheet index
     * @param xSpreadsheetDocument
     * @param index   (Short) 0,1,2,...
     * @return
     * @throws Exception
     */
    public static XSpreadsheet getSCSheetByIndex(XSpreadsheetDocument xSpreadsheetDocument, short index) throws Exception {
        XSpreadsheets xSpreadsheets = xSpreadsheetDocument.getSheets();
        XIndexAccess xIndexAccess =
                (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xSpreadsheets);
        XSpreadsheet xSpreadsheet =
                (XSpreadsheet) UnoRuntime.queryInterface(XSpreadsheet.class, xIndexAccess.getByIndex(index));

        return xSpreadsheet;
    }

    /**
     * Get rows object
     * @param xSpreadsheet
     * @return
     * @throws Exception
     */
    public static XTableRows getSCRows(XSpreadsheet xSpreadsheet) throws Exception {
        XColumnRowRange xColumnRowRange =
                (XColumnRowRange) UnoRuntime.queryInterface(XColumnRowRange.class, xSpreadsheet);
        XTableRows xTableRows = xColumnRowRange.getRows();

        return xTableRows;
    }

    /**
     * Get columns object
     * @param xSpreadsheet
     * @return
     * @throws Exception
     */
    public static XTableColumns getSCColumns(XSpreadsheet xSpreadsheet) throws Exception {
        XColumnRowRange xColumnRowRange =
                (XColumnRowRange) UnoRuntime.queryInterface(XColumnRowRange.class, xSpreadsheet);
        XTableColumns xTableColumns = xColumnRowRange.getColumns();

        return xTableColumns;
    }

    /**
     * Set floating number into specific cell
     * @param xSpreadsheet
     * @param column
     * @param row
     * @param value
     * @throws Exception
     */
    public static void setValueToCell(XSpreadsheet xSpreadsheet, int column, int row, double value) throws Exception {
        XCell xCell = xSpreadsheet.getCellByPosition(column, row);
        xCell.setValue(value);
    }

    /**
     * Set text into specific cell
     * @param xSpreadsheet
     * @param column
     * @param row
     * @param text
     * @throws Exception
     */
    public static void setTextToCell(XSpreadsheet xSpreadsheet, int column, int row, String text) throws Exception {
        XCell xCell = xSpreadsheet.getCellByPosition(column, row);
        XText xText = (XText) UnoRuntime.queryInterface(XText.class, xCell);
        xText.setString(text);
    }

    /**
     * Set text into specific cell
     * @param xCell
     * @param text
     * @throws Exception
     */
    public static void setTextToCell(XCell xCell, String text) throws Exception {
        XText xText = (XText) UnoRuntime.queryInterface(XText.class, xCell);
        xText.setString(text);
    }

    /**
     * Set formula into specific cell
     * @param xSpreadsheet
     * @param column
     * @param row
     * @param formula
     * @throws Exception
     */
    public static void setFormulaToCell(XSpreadsheet xSpreadsheet, int column, int row, String formula) throws Exception {
        XCell xCell = xSpreadsheet.getCellByPosition(column, row);
        xCell.setFormula(formula);
    }

    /**
     * Get value from specific cell
     * @param xSpreadsheet
     * @param column
     * @param row
     * @return
     * @throws Exception
     */
    public static double getValueFromCell(XSpreadsheet xSpreadsheet, int column, int row) throws Exception {
        XCell xCell = xSpreadsheet.getCellByPosition(column, row);
        double cellValue = xCell.getValue();

        return cellValue;
    }

    /**
     * Get text from specific cell
     * @param xSpreadsheet
     * @param column
     * @param row
     *
     * @return
     * @throws Exception
     */
    public static String getTextFromCell(XSpreadsheet xSpreadsheet, int column, int row) throws Exception {
        XCell xCell = xSpreadsheet.getCellByPosition(column, row);
        XText xText = (XText) UnoRuntime.queryInterface(XText.class, xCell);

        return xText.getString();
    }

    /**
     * Get formula string from specific cell
     * @param xSpreadsheet
     * @param column
     * @param row
     * @return
     * @throws Exception
     */
    public static String getFormulaFromCell(XSpreadsheet xSpreadsheet, int column, int row) throws Exception {
        XCell xCell = xSpreadsheet.getCellByPosition(column, row);
        String cellFormula = xCell.getFormula();

        return cellFormula;
    }

    /**
     * Set numbers into a cell range
     * @param xSpreadsheet
     * @param start_col
     * @param start_row
     * @param end_col
     * @param end_row
     * @param values
     * @throws Exception
     */
    public static void setValueToCellRange(XSpreadsheet xSpreadsheet, int start_col, int start_row, int end_col, int end_row,  double[][] values) throws Exception {
        XCellRange xCellRange = xSpreadsheet.getCellRangeByPosition(start_col, start_row, end_col, end_row);
        XCell xCell = null;
        for (int i = 0; i <= (end_row - start_row); i++ ) {
            for(int j = 0; j <= (end_col - start_col); j++) {
                xCell = xCellRange.getCellByPosition(j, i);
                xCell.setValue(values[i][j]);
            }
        }
    }

    /**
     * Set text into a cell range
     * @param xSpreadsheet
     * @param start_col
     * @param start_row
     * @param end_col
     * @param end_row
     * @param texts
     * @throws Exception
     */
    public static void setTextToCellRange(XSpreadsheet xSpreadsheet, int start_col, int start_row, int end_col, int end_row,  String[][] texts) throws Exception {
        XCellRange xCellRange = xSpreadsheet.getCellRangeByPosition(start_col, start_row, end_col, end_row);
        XCell xCell = null;
        XText xText = null;
        for (int i = 0; i <= (end_row - start_row); i++ ) {
            for(int j = 0; j <= (end_col - start_col); j++) {
                xCell = xCellRange.getCellByPosition(j, i);
                xText = (XText) UnoRuntime.queryInterface(XText.class, xCell);
                xText.setString(texts[i][j]);
            }
        }
    }

    /**
     * Get number content from a cell range
     * @param xSpreadsheet
     * @param start_col
     * @param start_row
     * @param end_col
     * @param end_row
     * @return
     * @throws Exception
     */
    public static double[][] getValueFromCellRange(XSpreadsheet xSpreadsheet, int start_col, int start_row, int end_col, int end_row) throws Exception {
        XCellRange xCellRange = xSpreadsheet.getCellRangeByPosition(start_col, start_row, end_col, end_row);
        XCell xCell = null;
        double[][] cellValues = new double[end_row - start_row+1][end_col - start_col +1];

        for (int i = 0; i <= (end_row - start_row); i++ ) {
            for(int j = 0; j <= (end_col - start_col); j++) {
                xCell = xCellRange.getCellByPosition(j, i);
                cellValues[i][j] = xCell.getValue();
            }
        }

        return cellValues;
    }

    /**
     * Get text content from a cell range
     * @param xSpreadsheet
     * @param start_col
     * @param start_row
     * @param end_col
     * @param end_row
     * @return
     * @throws Exception
     */
    public static String[][] getTextFromCellRange(XSpreadsheet xSpreadsheet, int start_col, int start_row, int end_col, int end_row) throws Exception {
        XCellRange xCellRange = xSpreadsheet.getCellRangeByPosition(start_col, start_row, end_col, end_row);
        XCell xCell = null;
        XText xText = null;
        String[][] cellTexts = new String[end_row - start_row+1][end_col - start_col +1];

        for (int i = 0; i <= (end_row - start_row); i++ ) {
            for (int j = 0; j <= (end_col - start_col); j++) {
                xCell = xCellRange.getCellByPosition(j, i);
                xText = (XText) UnoRuntime.queryInterface(XText.class, xCell);
                cellTexts[i][j] = xText.getString();
            }
        }

        return cellTexts;
    }

    //TODO ZS - public static String[][] getAllFromCellRange

    /**
     * Switch to specific sheet
     * @param xSpreadsheetDocument
     * @param xSpreadsheet
     */
    public static void setCurrentSheet(XSpreadsheetDocument xSpreadsheetDocument, XSpreadsheet xSpreadsheet) throws Exception {
        XModel xModel = (XModel) UnoRuntime.queryInterface(XModel.class, xSpreadsheetDocument);
        XController xController = xModel.getCurrentController();
        XSpreadsheetView xSpreadsheetView = (XSpreadsheetView) UnoRuntime.queryInterface(XSpreadsheetView.class, xController);
        xSpreadsheetView.setActiveSheet(xSpreadsheet);
    }

    /**
     * Get sheet object of current active sheet
     * @param xSpreadsheetDocument
     * @return
     */
    public static XSpreadsheet getCurrentSheet(XSpreadsheetDocument xSpreadsheetDocument) throws Exception {
        XModel xModel = (XModel) UnoRuntime.queryInterface(XModel.class, xSpreadsheetDocument);
        XController xController = xModel.getCurrentController();
        XSpreadsheetView xSpreadsheetView = (XSpreadsheetView) UnoRuntime.queryInterface(XSpreadsheetView.class, xController);
        XSpreadsheet xSpreadsheet = xSpreadsheetView.getActiveSheet();

        return xSpreadsheet;
    }

    /**
     * Set value of specific property from a cell
     * @param xCell
     * @param propName
     * @param value
     * @throws Exception
     */
    public static void setCellProperties(XCell xCell, String propName, Object value) throws Exception {

        XPropertySet xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xCell);
        xPropertySet.setPropertyValue(propName, value);
    }

    /**
     * Get value of specific property from a cell
     * @param xCell
     * @param propName
     * @return
     * @throws Exception
     */
    public static Object getCellProperties(XCell xCell, String propName) throws Exception {
        XPropertySet xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xCell);
        Object value = xPropertySet.getPropertyValue(propName);

        return value;
    }

    /**
     * Clear temp file directory
     */
    public static void clearTempDir() {
        FileUtil.deleteFile(Testspace.getFile(Testspace.getPath(scTempDir)));
    }

    /**
     * Save file as specific file format into spreadsheet temp file folder.
     * @param scComponent
     * @param fileName  File name string without extension name (e.g. "sampleFile")
     * @param extName ("ods", "ots", "xls", "xlt", "csv")
     * @throws Exception
     */
    public static void saveFileAs(XComponent scComponent, String fileName, String extName) throws Exception {

        initFilterName();

        String storeUrl = Testspace.getUrl(scTempDir + fileName + "." + extName);

        PropertyValue[] storeProps = new PropertyValue[2];
        storeProps[0] = new PropertyValue();
        storeProps[0].Name = "FilterName";
        storeProps[0].Value = filterName.get(extName);
        storeProps[1] = new PropertyValue();
        storeProps[1].Name = "Overwrite";
        storeProps[1].Value = new Boolean(true);

        XStorable scStorable =
                (XStorable) UnoRuntime.queryInterface(XStorable.class, scComponent);
        scStorable.storeAsURL(storeUrl, storeProps);
    }

    /**
     * Close specific opening spreadsheet file which has been saved
     * @param xSpreadsheetDocument
     * @throws Exception
     */
    public static void closeFile(XSpreadsheetDocument xSpreadsheetDocument) throws Exception {
        XCloseable xCloseable = (XCloseable) UnoRuntime.queryInterface(XCloseable.class, xSpreadsheetDocument);
        xCloseable.close(false);
    }

    /**
     * Close a opening file saved in spreadsheet temp file direction and reopen it in Spreadsheet. For save&reload test scenario only.
     * @param unoApp
     * @param xSpreadsheetDocument
     * @param fullFileName   File name with the extension name. (e.g. "sc.ods")
     * @return
     * @throws Exception
     */
    public static XSpreadsheetDocument reloadFile(UnoApp unoApp, XSpreadsheetDocument xSpreadsheetDocument, String fullFileName) throws Exception {
        closeFile(xSpreadsheetDocument);

        String filePath = Testspace.getPath(scTempDir + fullFileName);
        XSpreadsheetDocument xScDocument = UnoRuntime.queryInterface(XSpreadsheetDocument.class, unoApp.loadDocument(filePath));

        return xScDocument;
    }

    /**
     * Initial the filter name list
     * @throws Exception
     */
    private static void initFilterName() throws Exception {
        if (filterName.size() > 0) {
            return;
        }

        filterName.put("ods", "calc8");
        filterName.put("ots", "calc8_template");
        filterName.put("xls", "MS Excel 97");
        filterName.put("xlt", "MS Excel 97 Vorlage/Template");
        filterName.put("csv", "Text - txt - csv (StarCalc)");
    }

}
