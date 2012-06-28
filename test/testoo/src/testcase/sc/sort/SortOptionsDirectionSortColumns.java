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
public class SortOptionsDirectionSortColumns {

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
     * Test sort options: sort columns, direction "Left to right"
     * @throws Exception
     */
    @Test
    public void testSortOptionsDirectionSortColumns() throws Exception{

        // Create a new spreadsheet document
        startcenter.menuItem("File->New->Spreadsheet").select();
        sleep(3);

        // Input some data
        String[][] data = new String[][] {
                {"Units", "7", "27", "4", "12", "3", "6"},
        };
        String[][] expectedSortedResult = new String[][] {
                {"Units", "3", "4", "6", "7", "12", "27"},
        };
        CalcUtil.selectRange("A1");
        typeKeys("Units<right>7<right>27<right>4<right>12<right>3<right>6");

        // "Data->Sort...", check "Range contains column labels", "Left to right", sort first by"Units", "Ascending"
        calc.menuItem("Data->Sort...").select();
        SortOptionsPage.select();
        SortOptionsPage_RangeContainsColumnLabels.check();
        SortOptionsPage_LeftToRight.check();
        SortPage.select();
        SortPage_By1.select("Units");
        SortPage_Ascending1.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result", expectedSortedResult, CalcUtil.getCellTexts("A1:G1"));

        // Uodo/redo
        calc.menuItem("Edit->Undo: Sort").select();
        assertArrayEquals("Undo sorted result", data, CalcUtil.getCellTexts("A1:G1"));
        calc.menuItem("Edit->Redo: Sort").select();
        assertArrayEquals("Redo sorted result", expectedSortedResult, CalcUtil.getCellTexts("A1:G1"));

        // Save and close document
        String saveTo = fullPath("temp/" + "SortOptionsDirectionSortColumns.ods");
        calc.menuItem("File->Save As...").select();
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        calc.menuItem("File->Close").select();
        openStartcenter();

        // Reopen and verify sorted result
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        assertArrayEquals("Saved sorted result", expectedSortedResult, CalcUtil.getCellTexts("A1:G1"));
    }
}
