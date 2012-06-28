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
public class SortOptionsMultipleSortParameterSaved{

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
     * Test sort options: multiple sort, no data overlap, sort parameter saved correctly
     * @throws Exception
     */
    @Test
    public void testSortOptionsMultipleSortSortParameterSaved() throws Exception{

        // Create a new spreadsheet document
        startcenter.menuItem("File->New->Spreadsheet").select();
        sleep(3);

        // Input some data
        String[][] data1 = new String[][] {
                {"D"},
                {"C"},
                {"B"},
                {"A"},
                {"E"},
        };
        String[][] expectedSortedResult1 = new String[][] {
                {"A"},
                {"B"},
                {"C"},
                {"D"},
                {"E"},
        };
        String[][] data2 = new String[][] {
                {"Numbers"},
                {"4"},
                {"2"},
                {"5"},
                {"1"},
                {"3"},
        };
        String[][] expectedSortedResult2 = new String[][] {
                {"Numbers"},
                {"1"},
                {"2"},
                {"3"},
                {"4"},
                {"5"},
        };
        CalcUtil.selectRange("A1");
        typeKeys("D<down>C<down>B<down>A<down>E");

        // "Data->Sort...", uncheck "Range contains column labels", check "Case sensitive" and "Include formats", sort first by "Column A", "Ascending"
        calc.menuItem("Data->Sort...").select();
        SortOptionsPage.select();
        SortOptionsPage_RangeContainsColumnLabels.uncheck();
        SortOptionsPage_CaseSensitive.check();
        SortOptionsPage_IncludeFormats.check();
        SortPage.select();
        SortPage_By1.select("Column A");
        SortPage_Ascending1.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result1", expectedSortedResult1, CalcUtil.getCellTexts("A1:A5"));

        // Uodo/redo
        calc.menuItem("Edit->Undo: Sort").select();
        assertArrayEquals("Undo sorted result", data1, CalcUtil.getCellTexts("A1:A5"));
        calc.menuItem("Edit->Redo: Sort").select();
        assertArrayEquals("Redo sorted result", expectedSortedResult1, CalcUtil.getCellTexts("A1:A5"));

        // Input data2 into same sheet, data1 and data2 are not overlapped
        CalcUtil.selectRange("G10");
        typeKeys("Numbers<down>4<down>2<down>5<down>1<down>3");

        // Focus on data2, "Data->Sort...", check "Range contains column labels", uncheck "Case sensitive" and "Include formats", sort first by "Numbers", "Ascending"
        CalcUtil.selectRange("G10");
        calc.menuItem("Data->Sort...").select();
        SortOptionsPage.select();
        SortOptionsPage_RangeContainsColumnLabels.check();
        SortOptionsPage_CaseSensitive.uncheck();
        SortOptionsPage_IncludeFormats.uncheck();
        SortPage.select();
        SortPage_By1.select("Numbers");
        SortPage_Ascending1.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result2", expectedSortedResult2, CalcUtil.getCellTexts("G10:G15"));

        // Uodo/redo
        calc.menuItem("Edit->Undo: Sort").select();
        assertArrayEquals("Undo sorted result", data2, CalcUtil.getCellTexts("G10:G15"));
        calc.menuItem("Edit->Redo: Sort").select();
        assertArrayEquals("Redo sorted result", expectedSortedResult2, CalcUtil.getCellTexts("G10:G15"));

        // Open sort dialog, check its setting
        calc.menuItem("Data->Sort...").select();
        SortOptionsPage.select();
        assertTrue("Range contains column labels should be checked", SortOptionsPage_RangeContainsColumnLabels.isChecked());
        assertFalse("Case sensitive should not be checked", SortOptionsPage_CaseSensitive.isChecked());
        assertFalse("Include formats should not be checked", SortOptionsPage_IncludeFormats.isChecked());
        SortOptionsPage.ok();

        // Save and close document
        String saveTo = fullPath("temp/" + "SortOptionsMultipleSortParameterSaved.ods");
        calc.menuItem("File->Save As...").select();
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        calc.menuItem("File->Close").select();
        openStartcenter();

        // Reopen and verify sorted result and sort parameters
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        assertArrayEquals("Saved sorted result1", expectedSortedResult1, CalcUtil.getCellTexts("A1:A5"));
        assertArrayEquals("Saved sorted result2", expectedSortedResult2, CalcUtil.getCellTexts("G10:G15"));
        calc.menuItem("Data->Sort...").select();
        SortOptionsPage.select();
        assertTrue("Range contains column labels should be checked", SortOptionsPage_RangeContainsColumnLabels.isChecked());
        assertFalse("Case sensitive should not be checked", SortOptionsPage_CaseSensitive.isChecked());
        assertFalse("Include formats should not be checked", SortOptionsPage_IncludeFormats.isChecked());
        SortOptionsPage.ok();
    }
}
