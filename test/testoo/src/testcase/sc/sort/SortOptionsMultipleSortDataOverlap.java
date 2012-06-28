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
public class SortOptionsMultipleSortDataOverlap {

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
     * Test sort options: multiple sort, data overlap
     * @throws Exception
     */
    @Test
    public void testSortOptionsMultipleSortDataOverlap() throws Exception{

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
                {"4"},
                {"2"},
                {"5"},
                {"1"},
                {"3"},
        };
        String[][] expectedSortedResultDataOverlap = new String[][] {
                {"A"},
                {"B"},
                {"C"},
                {"1"},
                {"2"},
                {"3"},
                {"4"},
                {"5"},
        };
        CalcUtil.selectRange("A1");
        typeKeys("D<down>C<down>B<down>A<down>E");

        // "Data->Sort...", uncheck "Range contains column labels", sort first by "Column A", "Ascending"
        calc.menuItem("Data->Sort...").select();
        SortOptionsPage.select();
        SortOptionsPage_RangeContainsColumnLabels.uncheck();
        SortPage.select();
        SortPage_By1.select("Column A");
        SortPage_Ascending1.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result", expectedSortedResult1, CalcUtil.getCellTexts("A1:A5"));

        // Uodo/redo
        calc.menuItem("Edit->Undo: Sort").select();
        assertArrayEquals("Undo sorted result", data1, CalcUtil.getCellTexts("A1:A5"));
        calc.menuItem("Edit->Redo: Sort").select();
        assertArrayEquals("Redo sorted result", expectedSortedResult1, CalcUtil.getCellTexts("A1:A5"));

        // Input data2 into same sheet, data1 and data2 are not overlapped
        CalcUtil.selectRange("G10");
        typeKeys("4<down>2<down>5<down>1<down>3");

        // Focus on data2, "Data->Sort...", "Copy result to" partially overlap with data1, sort first by "Column G", "Ascending"
        CalcUtil.selectRange("G10");
        calc.menuItem("Data->Sort...").select();
        SortOptionsPage.select();
        SortOptionsPage_RangeContainsColumnLabels.uncheck();
        SortOptionsPage_CopyResultTo.check();
        SortOptionsPage_CopyResultToEdit.setText("A4");
        SortPage.select();
        SortPage_By1.select("Column G");
        SortPage_Ascending1.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result data overlap", expectedSortedResultDataOverlap, CalcUtil.getCellTexts("A1:A8"));

        // Save and close document
        String saveTo = fullPath("temp/" + "SortOptionsMultipleSortDataOverlap.ods");
        calc.menuItem("File->Save As...").select();
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        calc.menuItem("File->Close").select();
        openStartcenter();

        // Reopen and verify sorted result
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        assertArrayEquals("Saved sorted result", expectedSortedResultDataOverlap, CalcUtil.getCellTexts("A1:A8"));
        assertArrayEquals("Original data2", data2, CalcUtil.getCellTexts("G10:G14"));
    }
}
