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



/**
 *
 */
package testcase.sc.sort;

import static testlib.AppUtil.*;
import static testlib.UIMap.*;

import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.sleep;
import static org.openoffice.test.vcl.Tester.typeKeys;

import org.junit.Before;
import org.junit.Ignore;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;

import testlib.CalcUtil;
import testlib.Log;

/**
 *
 */
public class SortOptionsIncludeFormats {

    /**
     * TestCapture helps us to do
     * 1. Take a screenshot when failure occurs.
     * 2. Collect extra data when OpenOffice crashes.
     */
    @Rule
    public Log LOG = new Log();

    /**
     * initApp helps us to do
     * 1. Patch the OpenOffice to enable automation if necessary.
     * 2. Start OpenOffice with automation enabled if necessary.
     * 3. Reset OpenOffice to startcenter.
     *
     * @throws java.lang.Exception
     */
    @Before
    public void setUp() throws Exception {
        initApp();
    }

    /**
     * Test sort options: "Include formats"
     * @throws Exception
     */
    @Test
    public void testSortOptionsIncludeFormats() throws Exception{

        // Create a new spreadsheet document
        startcenter.menuItem("File->New->Spreadsheet").select();
        sleep(3);

        // Input some data
//      String[][] data = new String[][] {
//              {"Units"},
//              {"32"},
//              {"57"},
//              {"74"},
//              {"50"},
//              {"27"},
//              {"7"},
//      };
        String[][] dataWithCurrencyFormats = new String[][] {
                {"Units"},
                {"$32.00"},
                {"57.00 \u20ac"},
                {"\u20a4 74"},
                {"R$ 50.00"},
                {"\u062c.\u0645. 27.00"},
                {"7.00 \u0440\u0443\u0431."},
        };
        String[][] expectedSortedResultIncludeFormat = new String[][] {
                {"Units"},
                {"7.00 \u0440\u0443\u0431."},
                {"\u062c.\u0645. 27.00"},
                {"$32.00"},
                {"R$ 50.00"},
                {"57.00 \u20ac"},
                {"\u20a4 74"},
        };
        String[][] expectedSortedResultExcludeFormat = new String[][] {
                {"Units"},
                {"$7.00"},
                {"27.00 \u20ac"},
                {"\u20a4 32"},
                {"R$ 50.00"},
                {"\u062c.\u0645. 57.00"},
                {"74.00 \u0440\u0443\u0431."},
        };
        CalcUtil.selectRange("A1");
        typeKeys("Units<down>32<down>57<down>74<down>50<down>27<down>7");

        // Set Currency formats
        CalcUtil.selectRange("A2");
        typeKeys("<ctrl 1>");
//      calc.menuItem("Fortmat->Cells...").select(); error: can not find item "Format"
        FormatCellsDlg_NumbersPage.select();
        FormatCellsDlg_NumbersPageCategory.select("Currency");
        FormatCellsDlg_NumbersPageCurrencyFormat.select("$ English (USA)");
        FormatCellsDlg_NumbersPage.ok();
        CalcUtil.selectRange("A3");
        typeKeys("<ctrl 1>");
        FormatCellsDlg_NumbersPage.select();
        FormatCellsDlg_NumbersPageCategory.select("Currency");
        FormatCellsDlg_NumbersPageCurrencyFormat.select("\u20ac Spanish (Spain)");
        FormatCellsDlg_NumbersPage.ok();
        CalcUtil.selectRange("A4");
        typeKeys("<ctrl 1>");
        FormatCellsDlg_NumbersPage.select();
        FormatCellsDlg_NumbersPageCategory.select("Currency");
        FormatCellsDlg_NumbersPageCurrencyFormat.select("\u20a4 Latin");
        FormatCellsDlg_NumbersPage.ok();
        CalcUtil.selectRange("A5");
        typeKeys("<ctrl 1>");
        FormatCellsDlg_NumbersPage.select();
        FormatCellsDlg_NumbersPageCategory.select("Currency");
        FormatCellsDlg_NumbersPageCurrencyFormat.select("R$ Portuguese (Brazil)");
        FormatCellsDlg_NumbersPage.ok();
        CalcUtil.selectRange("A6");
        typeKeys("<ctrl 1>");
        FormatCellsDlg_NumbersPage.select();
        FormatCellsDlg_NumbersPageCategory.select("Currency");
        FormatCellsDlg_NumbersPageCurrencyFormat.select("\u062c.\u0645. Arabic (Egypt)");
        FormatCellsDlg_NumbersPage.ok();
        CalcUtil.selectRange("A7");
        typeKeys("<ctrl 1>");
        FormatCellsDlg_NumbersPage.select();
        FormatCellsDlg_NumbersPageCategory.select("Currency");
        FormatCellsDlg_NumbersPageCurrencyFormat.select("\u0440\u0443\u0431. Russian");
        FormatCellsDlg_NumbersPage.ok();

        // "Data->Sort...", check "Range contains column labels", check "Include formats", sort first by "Units", "Ascending"
        calc.menuItem("Data->Sort...").select();
        SortOptionsPage.select();
        SortOptionsPage_RangeContainsColumnLabels.check();
        SortOptionsPage_IncludeFormats.check();
        SortPage.select();
        SortPage_By1.select("Units");
        SortPage_Ascending1.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result include formats", expectedSortedResultIncludeFormat, CalcUtil.getCellTexts("A1:A7"));

        // Uodo/redo
        calc.menuItem("Edit->Undo: Sort").select();
        assertArrayEquals("Undo sorted result", dataWithCurrencyFormats, CalcUtil.getCellTexts("A1:A7"));
        calc.menuItem("Edit->Redo: Sort").select();
        assertArrayEquals("Redo sorted result", expectedSortedResultIncludeFormat, CalcUtil.getCellTexts("A1:A7"));
        calc.menuItem("Edit->Undo: Sort").select();

        // Copy the original data to sheet2
        CalcUtil.selectRange("A1:A7");
        calc.menuItem("Edit->Copy").select();
        CalcUtil.selectRange("Sheet2.A1");
        calc.menuItem("Edit->Paste").select();
        sleep(1);

        // "Data->Sort...", check "Range contains column labels", uncheck "Include formats", sort first by "Units", "Ascending"
        calc.menuItem("Data->Sort...").select();
        SortOptionsPage.select();
        SortOptionsPage_RangeContainsColumnLabels.check();
        SortOptionsPage_IncludeFormats.uncheck();
        SortPage.select();
        SortPage_By1.select("Units");
        SortPage_Ascending1.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result exclude formats", expectedSortedResultExcludeFormat, CalcUtil.getCellTexts("A1:A7"));

        // Uodo/redo
        calc.menuItem("Edit->Undo: Sort").select();
        assertArrayEquals("Undo sorted result", dataWithCurrencyFormats, CalcUtil.getCellTexts("A1:A7"));
        calc.menuItem("Edit->Redo: Sort").select();
        assertArrayEquals("Redo sorted result", expectedSortedResultExcludeFormat, CalcUtil.getCellTexts("A1:A7"));

        // Save and close document
        String saveTo = fullPath("temp/" + "SortOptionsIncludeFormats.ods");
        calc.menuItem("File->Save As...").select();
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        calc.menuItem("File->Close").select();
        openStartcenter();

        // Reopen and verify sorted result
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        assertArrayEquals("Original data", dataWithCurrencyFormats, CalcUtil.getCellTexts("$Sheet1.$A1:$A7"));
        assertArrayEquals("Saved sorted result exclude format", expectedSortedResultExcludeFormat, CalcUtil.getCellTexts("$Sheet2.$A1:$A7"));
    }
}
