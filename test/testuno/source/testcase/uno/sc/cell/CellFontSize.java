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
public class CellFontSize {

    private double[] expected;
    private String inputType;
    private double[] inputData;
    private String fileType;

    private static final UnoApp unoApp = new UnoApp();

    XComponent scComponent = null;
    XSpreadsheetDocument scDocument = null;

    @Parameters
    public static Collection<Object[]> data() throws Exception {
        double[] list1 = TestUtil.randFontSizeList(30, 409); // Excel2003's value range [is 1,409]
        double[] list2 = TestUtil.randFontSizeList(10, 76);
        double[] list3 = TestUtil.randFontSizeList(5, 20);
        double[] list4 = TestUtil.randFontSizeList(20, 999); //OO's value range is [1, 1000)
        return Arrays.asList(new Object[][] {
            {list1, "CharHeight", list1, "ods"},
            {list2, "CharHeight", list2, "ods"},
            {list3, "CharHeight", list3, "ods"},
            {list4, "CharHeight", list4, "ods"},
            {list1, "CharHeight", list1, "xls"},
            {list2, "CharHeight", list2, "xls"},
            {list3, "CharHeight", list3, "xls"}
        });
    }

    public CellFontSize(double[] expected, String inputType, double[] inputData, String fileType) {
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
     * 3. Set cell font size
     * 4. Save file as ODF/MSBinary format.
     * 5. Close and reopen file.  -> Check the font size setting.
     * @throws Exception
     */
    @Test
    public void testCellFontSize() throws Exception {
        String fileName = "testCellFontSize";

        int cellNum = inputData.length;
        XCell[] cells = new XCell[cellNum];
        double[] results = new double[cellNum];
        CellInfo cInfo = TestUtil.randCell(256, 100);

        XSpreadsheet sheet = SCUtil.getCurrentSheet(scDocument);

        for (int i = 0; i < cellNum; i++) {
            cells[i] = sheet.getCellByPosition(cInfo.getCol(), cInfo.getRow() + i);
        }

        cells[0].setValue(inputData[0]);
        SCUtil. setTextToCell(cells[1], inputType);
        cells[2].setFormula("=TRUE()");
        cells[3].setValue(-0.000999999);

        for (int i = 0; i < cellNum; i++) {
            SCUtil.setCellProperties(cells[i], inputType, inputData[i]);
        }

        SCUtil.saveFileAs(scComponent, fileName, fileType);
        scDocument = SCUtil.reloadFile(unoApp, scDocument, fileName + "." + fileType);
        sheet = SCUtil.getCurrentSheet(scDocument);

        for (int i = 0; i < cellNum; i++) {
            cells[i] = sheet.getCellByPosition(cInfo.getCol(), cInfo.getRow() + i);
            results[i] = ((Float) SCUtil.getCellProperties(cells[i], inputType)).floatValue();
        }
        SCUtil.closeFile(scDocument);

        assertArrayEquals("Incorrect cell font size(" + inputType + ") value got in ." + fileType + " file.", expected, results, 0);

    }

}
