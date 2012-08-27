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
import testlib.uno.SCUtil;
import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNamed;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XStorable;
import com.sun.star.io.IOException;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.SheetLinkMode;
import com.sun.star.sheet.XCalculatable;
import com.sun.star.sheet.XExternalDocLink;
import com.sun.star.sheet.XExternalDocLinks;
import com.sun.star.sheet.XSheetLinkable;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheetView;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCell;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XRefreshable;

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
        scDocument = SCUtil.getSCDocument(scComponent);
        XSpreadsheets spreadsheets = scDocument.getSheets();
        spreadsheets.insertNewByName(sheetname, (short) 1);

        // active the sheet second sheet aa
        XSpreadsheet newSpreadSheet = SCUtil.getSCSheetByIndex(scDocument,
                (short) 1);
        SCUtil.setCurrentSheet(scDocument, newSpreadSheet);

        // get the new speadsheet name
        assertEquals("actual should equals aa", sheetname,
                SCUtil.getSCSheetNameByIndex(scDocument, (short) 1));

        // Change the Spreadsheet name
        String changedname = "SpeadsheetAfterChange";
        SCUtil.setSCSheetNameByIndex(scDocument, (short) 1, changedname);

        // Save and reload document
        SCUtil.saveFileAs(scComponent, "TestSpreadsheet", "ods");
        XSpreadsheetDocument scDocumentTemp = SCUtil.reloadFile(unoApp,
                scDocument, "TestSpreadsheet.ods");

        scDocument = scDocumentTemp;
        String sheetnameaftermove = SCUtil.getSCSheetNameByIndex(scDocument,
                (short) 1);

        // Verify the changed Spreadsheet name
        assertEquals("actual should equals SpeadsheetAfterChange", changedname,
                sheetnameaftermove);

        scDocument.getSheets().removeByName(changedname);

        assertFalse("actual should equals false",
                spreadsheets.hasByName(changedname));
        SCUtil.save(scDocumentTemp);
    }

    @Test
    public void copypastesheet() throws Exception {
        // Insert some value into cells
        scDocument = SCUtil.getSCDocument(scComponent);
        String souceSheetName = "sourcesheet";
        SCUtil.setSCSheetNameByIndex(scDocument, (short) 0, souceSheetName);
        String[][] stringValues = { { "", "Jan", "Feb", "Mar", "Apr", "Mai" },
                { "Profit", "12.3", "43.2", "5.1", "76", "56.8" },
                { "Rival in business", "12.2", "12.6", "17.7", "20.4", "100" }, };
        XSpreadsheet sourceSpreadSheet = SCUtil.getSCSheetByName(scDocument,
                souceSheetName);
        // input strings into sheet1
        SCUtil.setTextToCellRange(sourceSpreadSheet, 0, 0, 5, 2, stringValues);
        // copy the sheet from sourcesheet to copysheet
        String newcopysheet = "copysheet";
        XSpreadsheets spreadsheets = scDocument.getSheets();
        spreadsheets.copyByName(souceSheetName, newcopysheet, (short) 2);

        // Save and reload document
        SCUtil.saveFileAs(scComponent, "TestCopysheet", "xls");
        XSpreadsheetDocument scDocumentTemp = SCUtil.reloadFile(unoApp,
                scDocument, "TestCopysheet.xls");
        scDocument = scDocumentTemp;

        XSpreadsheet copysheet = SCUtil
                .getSCSheetByIndex(scDocument, (short) 2);
        String[][] CopystringValues = SCUtil.getTextFromCellRange(copysheet, 0,
                0, 5, 2);
        assertArrayEquals("Expect string value should be stringValues",
                stringValues, CopystringValues);

    }

    @Test
    public void movesheet() throws Exception {

        // new sc document
        scDocument = SCUtil.getSCDocument(scComponent);
        XSpreadsheets spreadsheets = scDocument.getSheets();

        // change the first sheet name and input same value into the sheet cell
        String sheetname = "sourcesheet";
        SCUtil.setSCSheetNameByIndex(scDocument, (short) 0, sheetname);
        String[][] stringValues = { { "", "Jan", "Feb", "Mar", "Apr", "Mai" },
                { "Profit", "12.3", "43.2", "5.1", "76", "56.8" },
                { "Rival in business", "12.2", "12.6", "17.7", "20.4", "100" }, };
        XSpreadsheet movesheet = SCUtil
                .getSCSheetByIndex(scDocument, (short) 0);
        SCUtil.setTextToCellRange(movesheet, 0, 0, 5, 2, stringValues);

        // Before move, get the 2nd sheet name
        String secondSheetNameBeforeMove = SCUtil.getSCSheetNameByIndex(
                scDocument, (short) 1);

        // move the first sheet
        spreadsheets.moveByName(sheetname, (short) 2);

        // Save and reload document
        SCUtil.saveFileAs(scComponent, "Testmovesheet", "xls");
        XSpreadsheetDocument scDocumentTemp = SCUtil.reloadFile(unoApp,
                scDocument, "Testmovesheet.xls");
        scDocument = scDocumentTemp;

        // After move, get the first sheet name, and verify it same as 2nd sheet
        // name before move
        String firstsheetnameAfterMove = SCUtil.getSCSheetNameByIndex(
                scDocument, (short) 0);
        assertEquals("Expect result should be Sheet2",
                secondSheetNameBeforeMove, firstsheetnameAfterMove);

        // Get the target sheet name after move
        String sheetnameAfterMove = SCUtil.getSCSheetNameByIndex(scDocument,
                (short) 1);
        assertEquals("Expect result should be sourcesheet", sheetname,
                sheetnameAfterMove);

        // Check the cell value after move
        XSpreadsheet sheetaftermove = SCUtil.getSCSheetByIndex(scDocument,
                (short) 1);
        String[][] stringValuesaftermove = SCUtil.getTextFromCellRange(
                sheetaftermove, 0, 0, 5, 2);

        assertArrayEquals("Expect result should be stringValues", stringValues,
                stringValuesaftermove);

    }

    @Test
    public void hideShowSheet() throws Exception {
        // Insert a sheet named hide sheet after first sheet
        String sheetname = "hide sheet";
        scDocument = SCUtil.getSCDocument(scComponent);
        XSpreadsheets spreadsheets = scDocument.getSheets();
        spreadsheets.insertNewByName(sheetname, (short) 1);

        // active the sheet second sheet "hide sheet"
        XSpreadsheet secondSpreadSheet = SCUtil.getSCSheetByIndex(scDocument,
                (short) 1);
        SCUtil.setCurrentSheet(scDocument, secondSpreadSheet);
        // get second sheet name and verify it should be "hide sheet"
        assertEquals("expect active sheet name will be hide sheet", sheetname,
                SCUtil.getSCSheetNameByIndex(scDocument, (short) 1));

        // hide the sheet you insert
        XPropertySet sheetPropertySet = (XPropertySet) UnoRuntime
                .queryInterface(XPropertySet.class, secondSpreadSheet);
        boolean isvisiable = false;
        sheetPropertySet.setPropertyValue("IsVisible", isvisiable);

        // Save and reload document
        SCUtil.saveFileAs(scComponent, "Testhideshowsheet", "xls");
        XSpreadsheetDocument scDocumentTemp = SCUtil.reloadFile(unoApp,
                scDocument, "Testhideshowsheet.xls");
        scDocument = scDocumentTemp;

        // get the active sheet name after hide sheet, it should be Sheet2
        String sheet2Name = SCUtil.getSCSheetNameByIndex(scDocument, (short) 2);
        String activesheetname = SCUtil.getSCActiveSheetName(scDocument);
        assertEquals("Expect sheet name should be Sheet2", sheet2Name,
                activesheetname);

        // show sheet "hide sheet"
        sheetPropertySet = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class,
                SCUtil.getSCSheetByIndex(scDocument, (short) 1));
        isvisiable = true;
        sheetPropertySet.setPropertyValue("IsVisible", isvisiable);

        // active sheet "hide sheet"
        secondSpreadSheet = SCUtil.getSCSheetByIndex(scDocument, (short) 1);
        SCUtil.setCurrentSheet(scDocument, secondSpreadSheet);

        // Get current active sheet name, verify it same as "hide sheet"
        String currentactivesheetname = SCUtil.getSCActiveSheetName(scDocument);
        assertEquals("Expect active sheet name is hidesheet", sheetname,
                currentactivesheetname);
        SCUtil.save(scDocument);
    }

    @Test
    public void sheetColor() throws Exception {
        // get first sheet propertyset
        scDocument = SCUtil.getSCDocument(scComponent);
        XSpreadsheets spreadsheets = scDocument.getSheets();
        XSpreadsheet firstSpreadSheet = SCUtil.getSCSheetByIndex(scDocument,
                (short) 0);
        XPropertySet sheet1PropertySet = (XPropertySet) UnoRuntime
                .queryInterface(XPropertySet.class, firstSpreadSheet);

        // Set tabcolor to 111
        sheet1PropertySet.setPropertyValue("TabColor", 111);

        // copy the color sheet to new sheet
        spreadsheets.copyByName(
                SCUtil.getSCSheetNameByIndex(scDocument, (short) 0),
                "newsheet", (short) 3);

        // Save and reopen the document
        SCUtil.saveFileAs(scComponent, "Testcolorsheet", "ods");
        XSpreadsheetDocument scDocumentTemp = SCUtil.reloadFile(unoApp,
                scDocument, "Testcolorsheet.ods");
        scDocument = scDocumentTemp;

        // Get first sheet color
        sheet1PropertySet = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class,
                SCUtil.getSCSheetByIndex(scDocument, (short) 0));
        int firstSheetcolorid = (Integer) sheet1PropertySet
                .getPropertyValue("TabColor");

        // Get the copyed sheet color
        XPropertySet newsheetPropertySet = (XPropertySet) UnoRuntime
                .queryInterface(XPropertySet.class,
                        SCUtil.getSCSheetByIndex(scDocument, (short) 3));
        int copySheetcolorid = (Integer) newsheetPropertySet
                .getPropertyValue("TabColor");

        // Verify first sheet color changed successfully
        assertEquals("Expect color should be 111", 111, firstSheetcolorid);

        // Verify first sheet color same as copy sheet color
        assertEquals("Expect color should be 111", firstSheetcolorid,
                copySheetcolorid);
    }

    @Test
    public void insertSheetFromfile() throws Exception {
        // New a document source.xls, add value to 3 sheet
        scDocument = SCUtil.getSCDocument(scComponent);
        XSpreadsheets spreadsheets = scDocument.getSheets();
        XSpreadsheet firstSheet = SCUtil.getSCSheetByIndex(scDocument,
                (short) 0);
        XSpreadsheet secondSheet = SCUtil.getSCSheetByIndex(scDocument,
                (short) 1);
        XSpreadsheet thirdSheet = SCUtil.getSCSheetByIndex(scDocument,
                (short) 2);
        SCUtil.setFormulaToCell(firstSheet, 1, 2, "=2*2");
        SCUtil.setFormulaToCell(secondSheet, 1, 2, "=2*2");
        SCUtil.setFormulaToCell(thirdSheet, 1, 2, "=2*2");

        // Save and close this document
        SCUtil.saveFileAs(scComponent, "source", "xls");
        SCUtil.closeFile(scDocument);

        // get source document URL
        String SourcestoreUrl = Testspace.getUrl("output/" + "source" + "."
                + "xls");

        // New a document
        scComponent = unoApp.newDocument("scalc");
        scDocument = SCUtil.getSCDocument(scComponent);
        spreadsheets = scDocument.getSheets();
        // Insert firstexternalsheet sheet, link with Sheet1 in source document
        // and the link mode is NORMAL
        spreadsheets.insertNewByName("firstexternalsheet", (short) 3);
        XSpreadsheet firstexternalsheet = SCUtil.getSCSheetByIndex(scDocument,
                (short) 3);
        XSheetLinkable xfirstSheetLinkable = (XSheetLinkable) UnoRuntime
                .queryInterface(XSheetLinkable.class, firstexternalsheet);
        xfirstSheetLinkable.link(SourcestoreUrl, "", "MS Excel 97", "",
                SheetLinkMode.NORMAL);

        // Insert secondexternalsheet sheet, link with Sheet2 in source document
        // and the link mode is VALUE
        spreadsheets.insertNewByName("secondexternalsheet", (short) 4);
        XSpreadsheet secondexternalsheet = SCUtil.getSCSheetByIndex(scDocument,
                (short) 4);
        XSheetLinkable xsecondSheetLinkable = (XSheetLinkable) UnoRuntime
                .queryInterface(XSheetLinkable.class, secondexternalsheet);
        xsecondSheetLinkable.link(SourcestoreUrl, "Sheet2", "MS Excel 97", "",
                SheetLinkMode.VALUE);

        // Insert secondexternalsheet sheet, link with Sheet2 in source document
        // and the link mode is NONE
        spreadsheets.insertNewByName("thirdexternalsheet", (short) 5);
        XSpreadsheet thirdexternalsheet = SCUtil.getSCSheetByIndex(scDocument,
                (short) 5);
        XSheetLinkable xthirdSheetLinkable = (XSheetLinkable) UnoRuntime
                .queryInterface(XSheetLinkable.class, thirdexternalsheet);
        xthirdSheetLinkable.link(SourcestoreUrl, "Sheet3", "MS Excel 97", "",
                SheetLinkMode.NONE);

        // Verify firstexternalsheet
        assertEquals("Expect formula should be =2*2", "=2*2",
                SCUtil.getFormulaFromCell(firstexternalsheet, 1, 2));
        assertEquals("Expect formula result should be 4", "4",
                SCUtil.getTextFromCell(firstexternalsheet, 1, 2));

        // Verify secondexternalsheet
        assertEquals("Expect formula should be 4", "4",
                SCUtil.getFormulaFromCell(secondexternalsheet, 1, 2));
        assertEquals("Expect formula result should be 4", "4",
                SCUtil.getTextFromCell(secondexternalsheet, 1, 2));

        // Verify thirdexternalsheet
        assertEquals("Expect formula should be blank", "",
                SCUtil.getFormulaFromCell(thirdexternalsheet, 1, 2));
        assertEquals("Expect formula result should be blank", "",
                SCUtil.getTextFromCell(thirdexternalsheet, 1, 2));

        // save document and verify the linked sheet again
        SCUtil.saveFileAs(scComponent, "linked", "ods");
        XSpreadsheetDocument tempscDocument = SCUtil.reloadFile(unoApp,
                scDocument, "linked.ods");
        scDocument = tempscDocument;
        firstexternalsheet = SCUtil.getSCSheetByIndex(scDocument, (short) 3);
        secondexternalsheet = SCUtil.getSCSheetByIndex(scDocument, (short) 4);
        thirdexternalsheet = SCUtil.getSCSheetByIndex(scDocument, (short) 5);

        // Verify firstexternalsheet
        assertEquals("Expect formula should be =2*2", "=2*2",
                SCUtil.getFormulaFromCell(firstexternalsheet, 1, 2));
        assertEquals("Expect formula result should be 4", "4",
                SCUtil.getTextFromCell(firstexternalsheet, 1, 2));

        // Verify secondexternalsheet
        assertEquals("Expect formula should be 4", "4",
                SCUtil.getFormulaFromCell(secondexternalsheet, 1, 2));
        assertEquals("Expect formula result should be 4", "4",
                SCUtil.getTextFromCell(secondexternalsheet, 1, 2));

        // Verify thirdexternalsheet
        assertEquals("Expect formula should be blank", "",
                SCUtil.getFormulaFromCell(thirdexternalsheet, 1, 2));
        assertEquals("Expect formula result should be blank", "",
                SCUtil.getTextFromCell(thirdexternalsheet, 1, 2));

        //save and close document
        SCUtil.save(scDocument);
        SCUtil.closeFile(scDocument);

        //Open souce document and change the value in souce document
        XSpreadsheetDocument sourcescDocument = SCUtil.reloadFile(unoApp,
                scDocument, "source.xls");
        firstSheet = SCUtil.getSCSheetByIndex(sourcescDocument, (short) 0);
        secondSheet = SCUtil.getSCSheetByIndex(sourcescDocument, (short) 1);
        thirdSheet = SCUtil.getSCSheetByIndex(sourcescDocument, (short) 2);
        SCUtil.setFormulaToCell(firstSheet, 1, 2, "=3*3");
        SCUtil.setFormulaToCell(secondSheet, 1, 2, "=3*3");
        SCUtil.setFormulaToCell(thirdSheet, 1, 2, "=3*3");
        SCUtil.save(sourcescDocument);
        SCUtil.closeFile(sourcescDocument);

        //Open link document
        tempscDocument = SCUtil.reloadFile(unoApp, scDocument, "linked.ods");
        scDocument = tempscDocument;
        spreadsheets = scDocument.getSheets();

        firstexternalsheet = SCUtil.getSCSheetByIndex(scDocument, (short) 3);
        secondexternalsheet = SCUtil.getSCSheetByIndex(scDocument, (short) 4);
        thirdexternalsheet = SCUtil.getSCSheetByIndex(scDocument, (short) 5);

        //get Object SheetLinks for document
        XPropertySet sheetpropertyset = (XPropertySet) UnoRuntime
                .queryInterface(XPropertySet.class, scDocument);
        Object sheetLinks = sheetpropertyset.getPropertyValue("SheetLinks");

        XIndexAccess xsheetlinks = (XIndexAccess) UnoRuntime.queryInterface(
                XIndexAccess.class, sheetLinks);

        //Refresh all links
        for (int i = 0; i < xsheetlinks.getCount(); i++) {
            Object sheetlink = xsheetlinks.getByIndex(i);
            XRefreshable xsheetRefreshable = (XRefreshable) UnoRuntime
                    .queryInterface(XRefreshable.class, sheetlink);
            xsheetRefreshable.refresh();
        }

        // Verify firstexternalsheet
        assertEquals("Expect formula should be =3*3", "=3*3",
                SCUtil.getFormulaFromCell(firstexternalsheet, 1, 2));
        assertEquals("Expect formula result should be 9", "9",
                SCUtil.getTextFromCell(firstexternalsheet, 1, 2));

        // Verify secondexternalsheet
        assertEquals("Expect formula should be 9", "9",
                SCUtil.getFormulaFromCell(secondexternalsheet, 1, 2));
        assertEquals("Expect formula result should be 9", "9",
                SCUtil.getTextFromCell(secondexternalsheet, 1, 2));

        // Verify thirdexternalsheet
        assertEquals("Expect formula should be blank", "",
                SCUtil.getFormulaFromCell(thirdexternalsheet, 1, 2));
        assertEquals("Expect formula result should be blank", "",
                SCUtil.getTextFromCell(thirdexternalsheet, 1, 2));

        //Save the document before close
        SCUtil.save(scDocument);

    }

}
