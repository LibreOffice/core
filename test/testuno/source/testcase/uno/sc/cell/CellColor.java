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


package testcase.uno.sc.cell;

import static org.junit.Assert.assertArrayEquals;

import java.util.Arrays;
import java.util.Collection;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;
import org.openoffice.test.uno.UnoApp;

import testlib.uno.SCUtil;
import testlib.uno.TestUtil;
import testlib.uno.CellInfo;

import com.sun.star.lang.XComponent;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.table.XCell;


/**
 *  Check the cell background color and font color setting can be applied and saved
 *
 */
@RunWith(value = Parameterized.class)
public class CellColor {

    private int[] expected;
    private String inputType;
    private int[] inputData;
    private String fileType;

    private static final UnoApp unoApp = new UnoApp();

    XComponent scComponent = null;
    XSpreadsheetDocument scDocument = null;

    @Parameters
    public static Collection<Object[]> data() throws Exception {
        int[] list1 = TestUtil.randColorList(30);
        int[] list2 = TestUtil.randColorList(56);
        int[] list3 = TestUtil.randColorList(5);
        return Arrays.asList(new Object[][] {
            {list1, "CellBackColor", list1, "ods"},
            {list2, "CellBackColor", list2, "ods"},
            {list3, "CellBackColor", list3, "ods"},
            {list1, "CellBackColor", list1, "xls"},
//          {list2, "CellBackColor", list2, "xls"},   Bug 120679
            {list3, "CellBackColor", list3, "xls"},

            {list1, "CharColor", list1, "ods"},
            {list2, "CharColor", list2, "ods"},
            {list3, "CharColor", list3, "ods"},
            {list1, "CharColor", list1, "xls"},
//          {list2, "CharColor", list2, "xls"},   Bug 120679
            {list3, "CharColor", list3, "xls"}
        });
    }

    public CellColor(int[] expected, String inputType, int[] inputData, String fileType) {
        this.expected = expected;
        this.inputType = inputType;
        this.inputData = inputData;
        this.fileType = fileType;
    }


    @Before
    public void setUp() throws Exception {
        scComponent = unoApp.newDocument("scalc");
        scDocument = SCUtil.getSCDocument(scComponent);
    }

    @After
    public void tearDown() throws Exception {
        unoApp.closeDocument(scComponent);

    }

    @BeforeClass
    public static void setUpConnection() throws Exception {
        unoApp.start();
    }

    @AfterClass
    public static void tearDownConnection() throws InterruptedException, Exception {
        unoApp.close();
        SCUtil.clearTempDir();
    }

    /**
     * Check the cell background color and font color
     * 1. Create a spreadsheet file.
     * 2. Input number, text, formula into many cell.
     * 3a. Set cell background color.
     * 3b. Set cell font color.
     * 4. Save file as ODF/MSBinary format.
     * 5. Close and reopen file.  -> Check the color setting.
     * @throws Exception
     */
    @Test
    public void testCellColor() throws Exception {
        String fileName = "testCellColor";

        int cellNum = inputData.length;
        XCell[] cells = new XCell[cellNum];
        int[] results = new int[cellNum];
        CellInfo cInfo = TestUtil.randCell(256, 100);

        XSpreadsheet sheet = SCUtil.getCurrentSheet(scDocument);

        for (int i = 0; i < cellNum; i++) {
            cells[i] = sheet.getCellByPosition(cInfo.getCol(), cInfo.getRow() + i);
        }

        cells[0].setValue(inputData[0]);
        SCUtil. setTextToCell(cells[1], inputType);
        cells[2].setFormula("=100*23/5");
        cells[3].setValue(-0.0003424);

        for (int i = 0; i < cellNum; i++) {
            SCUtil.setCellProperties(cells[i], inputType, inputData[i]);
        }

        SCUtil.saveFileAs(scComponent, fileName, fileType);
        scDocument = SCUtil.reloadFile(unoApp, scDocument, fileName + "." + fileType);
        sheet = SCUtil.getCurrentSheet(scDocument);

        for (int i = 0; i < cellNum; i++) {
            cells[i] = sheet.getCellByPosition(cInfo.getCol(), cInfo.getRow() + i);
            results[i] = ((Integer) SCUtil.getCellProperties(cells[i], inputType)).intValue();
        }
        SCUtil.closeFile(scDocument);

        assertArrayEquals("Incorrect cell background color(" + inputType + ") value got in ." + fileType + " file.", expected, results);

    }

}
