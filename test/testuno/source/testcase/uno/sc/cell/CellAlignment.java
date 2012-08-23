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

import static org.junit.Assert.assertEquals;

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
import com.sun.star.table.CellHoriJustify;
import com.sun.star.table.CellVertJustify;
import com.sun.star.table.XCell;
import com.sun.star.uno.Enum;

/**
 * Check the cell alignment setting can be applied and saved
 *
 */
@RunWith(value = Parameterized.class)
public class CellAlignment {

    private Enum expected;
    private String inputType;
    private Enum inputValue;
    private String fileType;

    private static final UnoApp unoApp = new UnoApp();

    XComponent scComponent = null;
    XSpreadsheetDocument scDocument = null;

    @Parameters
    public static Collection<Object[]> data() {
        return Arrays.asList(new Object[][] {
                {CellHoriJustify.STANDARD, "HoriJustify", CellHoriJustify.STANDARD, "ods"},
                {CellHoriJustify.LEFT, "HoriJustify", CellHoriJustify.LEFT, "ods"},
                {CellHoriJustify.CENTER, "HoriJustify", CellHoriJustify.CENTER, "ods"},
                {CellHoriJustify.RIGHT, "HoriJustify", CellHoriJustify.RIGHT, "ods"},
                {CellHoriJustify.BLOCK, "HoriJustify", CellHoriJustify.BLOCK, "ods"},
                {CellHoriJustify.REPEAT, "HoriJustify", CellHoriJustify.REPEAT, "ods"},
                {CellVertJustify.STANDARD, "VertJustify", CellVertJustify.STANDARD, "ods"},
                {CellVertJustify.TOP, "VertJustify", CellVertJustify.TOP, "ods"},
                {CellVertJustify.CENTER, "VertJustify", CellVertJustify.CENTER, "ods"},
                {CellVertJustify.BOTTOM, "VertJustify", CellVertJustify.BOTTOM, "ods"},

                {CellHoriJustify.STANDARD, "HoriJustify", CellHoriJustify.STANDARD, "xls"},
                {CellHoriJustify.LEFT, "HoriJustify", CellHoriJustify.LEFT, "xls"},
                {CellHoriJustify.CENTER, "HoriJustify", CellHoriJustify.CENTER, "xls"},
                {CellHoriJustify.RIGHT, "HoriJustify", CellHoriJustify.RIGHT, "xls"},
                {CellHoriJustify.BLOCK, "HoriJustify", CellHoriJustify.BLOCK, "xls"},
                {CellHoriJustify.REPEAT, "HoriJustify", CellHoriJustify.REPEAT, "xls"},
                {CellVertJustify.STANDARD, "VertJustify", CellVertJustify.STANDARD, "xls"},
                {CellVertJustify.TOP, "VertJustify", CellVertJustify.TOP, "xls"},
                {CellVertJustify.CENTER, "VertJustify", CellVertJustify.CENTER, "xls"}//,
//              {CellVertJustify.BOTTOM, "VertJustify", CellVertJustify.BOTTOM, "xls"}    Bug 120670
        });
    }

    public CellAlignment(Enum expected, String inputType, Enum inputValue, String fileType) {
        this.expected = expected;
        this.inputType = inputType;
        this.inputValue = inputValue;
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
     * Check the cell alignment setting can be applied and saved
     * 1. Create a spreadsheet file.
     * 2. Input number, text, formula into many cell.
     * 3. Set cell alignment.
     * 4. Save file as ODF/MSBinary format.
     * 5. Close and reopen file.  -> Check the alignment setting.
     * @throws Exception
     */
    @Test
    public void testCellAlignment() throws Exception {
        String fileName = "testCellAlignment";
        CellInfo cInfo = TestUtil.randCell(20, 50);

        int cellNum = 5;
        XCell[] cells = new XCell[cellNum];
        Enum[] results = new Enum[cellNum];

        XSpreadsheet sheet = SCUtil.getCurrentSheet(scDocument);

        //cellNum must be greater than 4
        if (cellNum < 5) {
            cellNum = 5;
        }
        for (int i = 0; i < cellNum; i++) {
            cells[i] = sheet.getCellByPosition(cInfo.getCol(), cInfo.getRow() + i);
        }

        cells[0].setValue(13.42);
        SCUtil. setTextToCell(cells[1], "alignment");
        cells[2].setFormula("=SUM(A100:B100)");
        cells[3].setValue(-0.2343123);

        for (int i = 0; i < cellNum; i++) {
            SCUtil.setCellProperties(cells[i], inputType, inputValue);
        }

        SCUtil.saveFileAs(scComponent, fileName, fileType);

        scDocument = SCUtil.reloadFile(unoApp, scDocument, fileName+"." + fileType);
        sheet = SCUtil.getCurrentSheet(scDocument);
        for (int i = 0; i < cellNum; i++) {
            cells[i] = sheet.getCellByPosition(cInfo.getCol(), cInfo.getRow() + i);
            results[i] = (Enum) SCUtil.getCellProperties(cells[i], inputType);
        }
        SCUtil.closeFile(scDocument);

        for (int i = 0; i < cellNum; i++ ) {

            assertEquals("Incorrect cell alignment(" + inputType + ") value got in ." + fileType + " file.", expected, results[i]);

        }

    }

}
