/* Licensed to the Apache Software Foundation (ASF) under one
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

package testcase.uno.sc.sheet;

import static org.junit.Assert.*;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.Testspace;
import org.openoffice.test.uno.UnoApp;

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNamed;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XStorable;
import com.sun.star.io.IOException;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheetView;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCell;
import com.sun.star.uno.UnoRuntime;

public class SheetBasicTest {
    UnoApp unoApp = new UnoApp();

    XSpreadsheetDocument scDocument = null;
    XComponent scComponent = null;

    @BeforeClass
    public static void setUpBeforeClass() throws Exception {

    }

    @AfterClass
    public static void tearDownAfterClass() throws Exception {
    }

    @Before
    public void setUp() throws Exception {
        unoApp.start();
        // New a SC document
        scComponent = unoApp.newDocument("scalc");
    }

    @After
    public void tearDown() throws Exception {
        unoApp.closeDocument(scComponent);
        unoApp.close();
    }

    @Test
    public void insertRenameDeleteSheet() throws Exception {
        // Insert a sheet named aa after first sheet
        String sheetname = "aa";
        scDocument = (XSpreadsheetDocument) UnoRuntime.queryInterface(
                XSpreadsheetDocument.class, scComponent);
        XSpreadsheets spreadsheets = scDocument.getSheets();
        spreadsheets.insertNewByName(sheetname, (short) 1);

        // active the sheet second sheet aa
        XIndexAccess xspreadsheetIndex = (XIndexAccess) UnoRuntime
                .queryInterface(XIndexAccess.class, spreadsheets);
        XSpreadsheet newSpreadSheet = (XSpreadsheet) UnoRuntime.queryInterface(
                XSpreadsheet.class, xspreadsheetIndex.getByIndex(1));
        XModel xSpreadsheetModel = (XModel) UnoRuntime.queryInterface(
                XModel.class, scDocument);
        XSpreadsheetView xSpeadsheetView = (XSpreadsheetView) UnoRuntime
                .queryInterface(XSpreadsheetView.class,
                        xSpreadsheetModel.getCurrentController());
        xSpeadsheetView.setActiveSheet(newSpreadSheet);

        // get the new speadsheet name
        XNamed xsheetname = (XNamed) UnoRuntime.queryInterface(XNamed.class,
                newSpreadSheet);
        assertEquals("actual should equals aa", sheetname, xsheetname.getName());

        // Change the Spreadsheet name
        String changedname = "SpeadsheetAfterChange";
        xsheetname.setName(changedname);

        // Save and reload document
        reloadSpreadsheet("TestSpreadsheet.xls");

        // Verify the changed Spreadsheet name
        assertEquals("actual should equals SpeadsheetAfterChange", changedname,
                xsheetname.getName());

        spreadsheets.removeByName(changedname);

        assertFalse("actual should equals false",
                spreadsheets.hasByName(changedname));

    }

    @Test
    public void copypastesheet() throws Exception {
        // Insert some value into cells
        scDocument = (XSpreadsheetDocument) UnoRuntime.queryInterface(
                XSpreadsheetDocument.class, scComponent);
        XSpreadsheets spreadsheets = scDocument.getSheets();
        XIndexAccess xspreadsheetIndex = (XIndexAccess) UnoRuntime
                .queryInterface(XIndexAccess.class, spreadsheets);
        XSpreadsheet spreadSheet = (XSpreadsheet) UnoRuntime.queryInterface(
                XSpreadsheet.class, xspreadsheetIndex.getByIndex(0));
        XNamed xsheetname = (XNamed) UnoRuntime.queryInterface(XNamed.class,
                spreadSheet);
        xsheetname.setName("sourcesheet");
        String[][] stringValues = { { "", "Jan", "Feb", "Mar", "Apr", "Mai" },
                { "Profit", "12.3", "43.2", "5.1", "76", "56.8" },
                { "Rival in business", "12.2", "12.6", "17.7", "20.4", "100" }, };
        // input strings into sheet1
        for (int intY = 0; intY < stringValues.length; intY++) {
            for (int intX = 0; intX < stringValues[intY].length; intX++) {
                // Insert the value to the cell, specified by intY and intX.
                this.insertIntoCell(intY, intX, stringValues[intY][intX],
                        spreadSheet, "");
            }
        }

        // copy the sheet from sourcesheet to copysheet
        String newcopysheet = "copysheet";
        spreadsheets.copyByName(xsheetname.getName(), newcopysheet, (short) 2);

        // Save and reload document
        reloadSpreadsheet("TestCopysheet.xls");
        XModel xSpreadsheetModel = (XModel) UnoRuntime.queryInterface(
                XModel.class, scDocument);
        XSpreadsheetView xSpeadsheetView = (XSpreadsheetView) UnoRuntime
                .queryInterface(XSpreadsheetView.class,
                        xSpreadsheetModel.getCurrentController());
        XSpreadsheet copysheet = (XSpreadsheet) UnoRuntime.queryInterface(
                XSpreadsheet.class, xspreadsheetIndex.getByIndex(2));
        for (int intY = 0; intY < stringValues.length; intY++) {
            for (int intX = 0; intX < stringValues[intY].length; intX++) {
                XCell xcell = null;
                xcell = copysheet.getCellByPosition(intY, intX);
                assertEquals(stringValues[intY][intX], xcell.getFormula());
            }
        }

    }

    @Test
    public void movesheet() throws Exception {

        // new sc document
        scDocument = (XSpreadsheetDocument) UnoRuntime.queryInterface(
                XSpreadsheetDocument.class, scComponent);

        // change the first sheet name and input same value into the sheet cell
        XSpreadsheets spreadsheets = scDocument.getSheets();
        XIndexAccess xspreadsheetIndex = (XIndexAccess) UnoRuntime
                .queryInterface(XIndexAccess.class, spreadsheets);
        XSpreadsheet spreadSheet = (XSpreadsheet) UnoRuntime.queryInterface(
                XSpreadsheet.class, xspreadsheetIndex.getByIndex(0));
        XNamed xsheetname = (XNamed) UnoRuntime.queryInterface(XNamed.class,
                spreadSheet);
        String sheetname = "sourcesheet";
        xsheetname.setName(sheetname);
        String[][] stringValues = { { "", "Jan", "Feb", "Mar", "Apr", "Mai" },
                { "Profit", "12.3", "43.2", "5.1", "76", "56.8" },
                { "Rival in business", "12.2", "12.6", "17.7", "20.4", "100" }, };

        for (int intY = 0; intY < stringValues.length; intY++) {
            for (int intX = 0; intX < stringValues[intY].length; intX++) {
                // Insert the value to the cell, specified by intY and intX.
                this.insertIntoCell(intY, intX, stringValues[intY][intX],
                        spreadSheet, "");
            }
        }

        // Before move, get the 2nd sheet name
        XSpreadsheet secondSheetBeforeMove = (XSpreadsheet) UnoRuntime
                .queryInterface(XSpreadsheet.class,
                        xspreadsheetIndex.getByIndex(1));
        XNamed secondSheetNameBeforeMove = (XNamed) UnoRuntime.queryInterface(
                XNamed.class, secondSheetBeforeMove);

        // move the first sheet
        spreadsheets.moveByName(sheetname, (short) 2);

        // Save and reload document
        reloadSpreadsheet("Testmovesheet.xls");

        // After move, get the first sheet name, and verify it same as 2nd sheet
        // name before move
        XSpreadsheet firstSheetAfterMove = (XSpreadsheet) UnoRuntime
                .queryInterface(XSpreadsheet.class,
                        xspreadsheetIndex.getByIndex(0));
        XNamed xfirstsheetnameAfterMove = (XNamed) UnoRuntime.queryInterface(
                XNamed.class, firstSheetAfterMove);
        assertEquals("Expect result should be Sheet2",
                secondSheetNameBeforeMove.getName(),
                xfirstsheetnameAfterMove.getName());

        // Get the target sheet name after move
        XSpreadsheet sheetAfterMove = (XSpreadsheet) UnoRuntime.queryInterface(
                XSpreadsheet.class, xspreadsheetIndex.getByIndex(1));
        XNamed xsheetnameAfterMove = (XNamed) UnoRuntime.queryInterface(
                XNamed.class, sheetAfterMove);
        assertEquals("Expect result should be sourcesheet", sheetname,
                xsheetnameAfterMove.getName());

        // Check the cell value after move
        XModel xSpreadsheetModel = (XModel) UnoRuntime.queryInterface(
                XModel.class, scDocument);
        XSpreadsheetView xSpeadsheetView = (XSpreadsheetView) UnoRuntime
                .queryInterface(XSpreadsheetView.class,
                        xSpreadsheetModel.getCurrentController());
        XSpreadsheet movesheet = (XSpreadsheet) UnoRuntime.queryInterface(
                XSpreadsheet.class, xspreadsheetIndex.getByIndex(1));
        for (int intY = 0; intY < stringValues.length; intY++) {
            for (int intX = 0; intX < stringValues[intY].length; intX++) {
                XCell xcell = null;
                xcell = movesheet.getCellByPosition(intY, intX);
                assertEquals(stringValues[intY][intX], xcell.getFormula());
            }
        }

    }

    // input value into sheet cell
    public static void insertIntoCell(int intX, int intY, String stringValue,
            XSpreadsheet xspreadsheet, String stringFlag)
            throws IndexOutOfBoundsException {
        XCell xcell = null;
        xcell = xspreadsheet.getCellByPosition(intX, intY);
        if (stringFlag.equals("V")) {
            xcell.setValue((new Float(stringValue)).floatValue());
        } else {
            xcell.setFormula(stringValue);
        }
    }

    // Save and load the document
    public XSpreadsheetDocument reloadSpreadsheet(String spreadSheetname)
            throws Exception {
        String filePath = Testspace.getPath("output/" + spreadSheetname);
        XStorable xStorable = (XStorable) UnoRuntime.queryInterface(
                XStorable.class, scDocument);
        PropertyValue[] aStoreProperties = new PropertyValue[2];
        aStoreProperties[0] = new PropertyValue();
        aStoreProperties[1] = new PropertyValue();
        aStoreProperties[0].Name = "Override";
        aStoreProperties[0].Value = true;
        aStoreProperties[1].Name = "FilterName";
        aStoreProperties[1].Value = "MS Excel 97";
        xStorable.storeAsURL(FileUtil.getUrl(filePath), aStoreProperties);

        return UnoRuntime.queryInterface(XSpreadsheetDocument.class,
                unoApp.loadDocument(filePath));
    }
}
