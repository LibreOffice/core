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
package testcase.gui.sc.sort;

import static org.junit.Assert.*;
import static org.openoffice.test.common.Testspace.*;
import static org.openoffice.test.vcl.Tester.*;
import static testlib.gui.AppUtil.*;
import static testlib.gui.UIMap.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.Logger;

import testlib.gui.CalcUtil;

/**
 * Test Data->Sort dialog setting
 */
public class SortDialogSetting {

    @Rule
    public Logger log = Logger.getLogger(this);

    @Before
    public void setUp() throws Exception {
        app.start(true);

        // Create a new spreadsheet document
        app.dispatch("private:factory/scalc");
        calc.waitForExistence(10, 2);
    }

    @After
    public void tearDown() throws Exception {

    }

    /**
     * Test sort with options setting: case sensitive
     *
     * @throws Exception
     */
    @Test
    public void testSortOptionsCaseSensitive() throws Exception {

        // Input some data: A1~A6: 1ColumnName,D,C,B,A,a
        String[][] data = new String[][] { { "D" }, { "C" }, { "B" }, { "A" }, { "a" }, };
        String[][] expectedSortedResult = new String[][] { { "a" }, { "A" }, { "B" }, { "C" }, { "D" }, };
        CalcUtil.selectRange("A1");
        typeKeys("1ColumnName<down>D<down>C<down>B<down>A<down>a<down>");
        CalcUtil.selectRange("A6");
        app.dispatch(".uno:ChangeCaseToLower"); // In case SC capitalize first
                                                // letter automatically

        // "Data->Sort...", choose "Ascending", check "Case sensitive"
        app.dispatch(".uno:DataSort");
        SortOptionsPage.select();
        SortOptionsPage_CaseSensitive.check();
        SortOptionsPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result", expectedSortedResult, CalcUtil.getCellTexts("A2:A6"));

        // Uodo/redo
        app.dispatch(".uno:Undo");
        assertArrayEquals("Undo sorted result", data, CalcUtil.getCellTexts("A2:A6"));
        app.dispatch(".uno:Redo");
        assertArrayEquals("Redo sorted result", expectedSortedResult, CalcUtil.getCellTexts("A2:A6"));

        // Save and close document
        String saveTo = getPath("temp/" + "RowsSortWithOptionsCaseSensitive.ods");
        app.dispatch(".uno:SaveAs");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        app.dispatch(".uno:CloseDoc");
        openStartcenter();

        // Reopen and verify sorted result
        app.dispatch(".uno:Open");
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        assertArrayEquals("Saved sorted result", expectedSortedResult, CalcUtil.getCellTexts("A2:A6"));
    }

    /**
     * Test sort with options setting: copy sort result to
     *
     * @throws Exception
     */
    @Ignore("Bug 119035")
    public void testSortOptionsCopyResultTo() throws Exception {

        // Input some data
        // String[][] data = new String[][] {
        // {"3", "D"},
        // {"5", "FLK"},
        // {"4", "E"},
        // {"2", "BC"},
        // {"6", "GE"},
        // {"1", "AB"},
        // };
        String[][] expectedSortedResult = new String[][] { { "1", "AB" }, { "2", "BC" }, { "3", "D" }, { "4", "E" }, { "5", "FLK" }, { "6", "GE" }, };
        CalcUtil.selectRange("A1");
        typeKeys("3<down>5<down>4<down>2<down>6<down>1");
        CalcUtil.selectRange("B1");
        typeKeys("D<down>FLK<down>E<down>BC<down>GE<down>AB");

        // Data->Sort..., choose "Ascending", check "Copy sort results to:"
        app.dispatch(".uno:DataSort");
        SortOptionsPage.select();
        SortOptionsPage_CopyResultTo.check();
        SortOptionsPage_CopyResultToEdit.setText("$Sheet3.$A4");
        SortOptionsPage.ok();

        // Verify sorted result
        assertArrayEquals("Copy sorted result to", expectedSortedResult, CalcUtil.getCellTexts("$Sheet3.$A4:$B9"));

        // Uodo/redo
        app.dispatch(".uno:Undo");
        assertEquals("Undo sorted result", "", CalcUtil.getCellText("$Sheet3.$A4"));
        app.dispatch(".uno:Redo");
        assertArrayEquals("Redo sorted result", expectedSortedResult, CalcUtil.getCellTexts("$Sheet3.$A4:$B9"));

        // Move focus to sheet2 then select a cell range,
        // Insert->Names->Define...
        CalcUtil.selectRange("$Sheet2.$A1:$B3");
        app.dispatch(".uno:DefineName");
        DefineNamesDlg_NameEdit.setText("cellRange");
        DefineNamesDlg.ok();

        // Set focus to the original data, Data->Sort...
        CalcUtil.selectRange("$Sheet1.$B1");
        app.dispatch(".uno:DataSort");
        SortOptionsPage.select();
        SortOptionsPage_CopyResultTo.check();
        SortOptionsPage_CopyResultToCellRange.select("cellRange");
        SortOptionsPage.ok();

        // Verify sorted result
        assertArrayEquals("Copy sorted result to cell range", expectedSortedResult, CalcUtil.getCellTexts("$Sheet2.$A1:$B6"));

        // Uodo/redo
        app.dispatch(".uno:Undo");
        assertEquals("Undo sorted result", "", CalcUtil.getCellText("$Sheet2.$A1"));
        app.dispatch(".uno:Redo");
        assertArrayEquals("Redo sorted result", expectedSortedResult, CalcUtil.getCellTexts("$Sheet2.$A1:$B6"));

        // Save and close document
        String saveTo = getPath("temp/" + "RowsSortWithOptionsCopyResultTo.ods");
        app.dispatch(".uno:SaveAs");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        app.dispatch(".uno:CloseDoc");
        openStartcenter();

        // Reopen and verify sorted result
        app.dispatch(".uno:Open");
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        assertArrayEquals("Saved sorted result", expectedSortedResult, CalcUtil.getCellTexts("$Sheet3.$A4:$B9"));
        assertArrayEquals("Saved sorted result to cell range", expectedSortedResult, CalcUtil.getCellTexts("$Sheet2.$A1:$B6"));
    }

    /**
     * Test sort criteria: sort first by
     *
     * @throws Exception
     */
    @Test
    public void testSortCriteriaSortFirstBy() throws Exception {

        // Input some data
        String[][] data = new String[][] { { "3", "D" }, { "5", "FLK" }, { "4", "E" }, { "2", "BC" }, { "6", "GE" }, { "1", "AB" }, };
        String[][] expectedResultSortByColumnBAscending = new String[][] { { "1", "AB" }, { "2", "BC" }, { "3", "D" }, { "4", "E" }, { "5", "FLK" }, { "6", "GE" }, };
        String[][] expectedResultSortByColumnADescending = new String[][] { { "6", "GE" }, { "5", "FLK" }, { "4", "E" }, { "3", "D" }, { "2", "BC" }, { "1", "AB" }, };
        CalcUtil.selectRange("A1");
        typeKeys("3<down>5<down>4<down>2<down>6<down>1");
        CalcUtil.selectRange("B1");
        typeKeys("D<down>FLK<down>E<down>BC<down>GE<down>AB");

        // "Data->Sort...", choose "Ascending", sort first by Column B
        app.dispatch(".uno:DataSort");
        SortPage_Ascending1.check();
        SortPage_By1.select(2); // "Column B"
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result", expectedResultSortByColumnBAscending, CalcUtil.getCellTexts("A1:B6"));

        // Uodo/redo
        app.dispatch(".uno:Undo");
        assertArrayEquals("Undo sorted result", data, CalcUtil.getCellTexts("A1:B6"));
        app.dispatch(".uno:Redo");
        assertArrayEquals("Redo sorted result", expectedResultSortByColumnBAscending, CalcUtil.getCellTexts("A1:B6"));
        app.dispatch(".uno:Undo");

        // Save and close document
        String saveTo = getPath("temp/" + "SortCriteriaSortFirstBy.ods");
        app.dispatch(".uno:SaveAs");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        app.dispatch(".uno:CloseDoc");
        openStartcenter();

        // Reopen, "Data->Sort...", choose "Descending", sort first by Column A
        app.dispatch(".uno:Open");
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        app.dispatch(".uno:DataSort");
        SortPage_Descending1.check();
        SortPage_By1.select(1); // "Column A"
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Saved sorted result", expectedResultSortByColumnADescending, CalcUtil.getCellTexts("A1:B6"));
    }

    /**
     * Test sort criteria: sort second by
     *
     * @throws Exception
     */
    @Test
    public void testSortCriteriaSortSecondBy() throws Exception {

        // Input some data
        String[][] data = new String[][] { { "3", "D" }, { "5", "FLK" }, { "4", "E" }, { "1", "AB" }, { "6", "GE" }, { "2", "AB" }, };
        String[][] expectedResultSortFirstByB = new String[][] { { "1", "AB" }, { "2", "AB" }, { "3", "D" }, { "4", "E" }, { "5", "FLK" }, { "6", "GE" }, };
        String[][] expectedResultSortSecondByA = new String[][] { { "2", "AB" }, { "1", "AB" }, { "3", "D" }, { "4", "E" }, { "5", "FLK" }, { "6", "GE" }, };
        CalcUtil.selectRange("A1");
        typeKeys("3<down>5<down>4<down>1<down>6<down>2");
        CalcUtil.selectRange("B1");
        typeKeys("D<down>FLK<down>E<down>AB<down>GE<down>AB");

        // "Data->Sort...", choose "Ascending", sort first by Column B
        app.dispatch(".uno:DataSort");
        SortPage_Ascending1.check();
        SortPage_By1.select(2); // "Column B"
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result", expectedResultSortFirstByB, CalcUtil.getCellTexts("A1:B6"));

        // Uodo/redo
        app.dispatch(".uno:Undo");
        assertArrayEquals("Undo sorted result", data, CalcUtil.getCellTexts("A1:B6"));
        app.dispatch(".uno:Redo");
        assertArrayEquals("Redo sorted result", expectedResultSortFirstByB, CalcUtil.getCellTexts("A1:B6"));
        app.dispatch(".uno:Undo");

        // Save and close document
        String saveTo = getPath("temp/" + "SortCriteriaSortSecondBy.ods");
        app.dispatch(".uno:SaveAs");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        app.dispatch(".uno:CloseDoc");
        openStartcenter();

        // Reopen, "Data->Sort...", sort first by Column B "Ascending", sort
        // second by Column A "Descending"
        app.dispatch(".uno:Open");
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        app.dispatch(".uno:DataSort");
        SortPage_By1.select(2); // "Column B"
        SortPage_Ascending1.check();
        SortPage_By2.select(1); // "Column A"
        SortPage_Descending2.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Saved sorted result", expectedResultSortSecondByA, CalcUtil.getCellTexts("A1:B6"));
    }

    /**
     * Test sort criteria: sort third by
     *
     * @throws Exception
     */
    @Test
    public void testSortCriteriaSortThirdBy() throws Exception {

        // Input some data
        String[][] data = new String[][] { { "3", "AB", "2" }, { "8", "FLK", "5" }, { "6", "E", "4" }, { "1", "AB", "1" }, { "9", "GE", "6" }, { "2", "AB", "2" },
                { "7", "EFYU", "7" }, { "5", "DS", "8" }, { "4", "AB", "1" }, };
        String[][] expectedResultSortFirstByB = new String[][] { { "3", "AB", "2" }, { "1", "AB", "1" }, { "2", "AB", "2" }, { "4", "AB", "1" }, { "5", "DS", "8" },
                { "6", "E", "4" }, { "7", "EFYU", "7" }, { "8", "FLK", "5" }, { "9", "GE", "6" }, };
        String[][] expectedResultSortSecondByC = new String[][] { { "3", "AB", "2" }, { "2", "AB", "2" }, { "1", "AB", "1" }, { "4", "AB", "1" }, { "5", "DS", "8" },
                { "6", "E", "4" }, { "7", "EFYU", "7" }, { "8", "FLK", "5" }, { "9", "GE", "6" }, };
        String[][] expectedResultSortThirdByA = new String[][] { { "3", "AB", "2" }, { "2", "AB", "2" }, { "4", "AB", "1" }, { "1", "AB", "1" }, { "5", "DS", "8" },
                { "6", "E", "4" }, { "7", "EFYU", "7" }, { "8", "FLK", "5" }, { "9", "GE", "6" }, };
        CalcUtil.selectRange("A1");
        typeKeys("3<down>8<down>6<down>1<down>9<down>2<down>7<down>5<down>4");
        CalcUtil.selectRange("B1");
        typeKeys("AB<down>FLK<down>E<down>AB<down>GE<down>AB<down>EFYU<down>DS<down>AB");
        CalcUtil.selectRange("C1");
        typeKeys("2<down>5<down>4<down>1<down>6<down>2<down>7<down>8<down>1");

        // "Data->Sort...", choose "Ascending", sort first by Column B
        app.dispatch(".uno:DataSort");
        SortPage_By1.select(2); // "Column B"
        SortPage_Ascending1.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result", expectedResultSortFirstByB, CalcUtil.getCellTexts("A1:C9"));

        // Uodo/redo
        app.dispatch(".uno:Undo");
        assertArrayEquals("Undo sorted result", data, CalcUtil.getCellTexts("A1:C9"));
        app.dispatch(".uno:Redo");
        assertArrayEquals("Redo sorted result", expectedResultSortFirstByB, CalcUtil.getCellTexts("A1:C9"));
        app.dispatch(".uno:Undo");

        // Save and close document
        String saveTo = getPath("temp/" + "SortCriteriaSortThirdBy.ods");
        app.dispatch(".uno:SaveAs");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        app.dispatch(".uno:CloseDoc");
        openStartcenter();

        // Reopen, "Data->Sort...", sort first by Column B "Ascending", sort
        // second by Column C "Descending"
        app.dispatch(".uno:Open");
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        app.dispatch(".uno:DataSort");
        SortPage_By1.select(2); // "Column B"
        SortPage_Ascending1.check();
        SortPage_By2.select(3); // "Column C"
        SortPage_Descending2.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result", expectedResultSortSecondByC, CalcUtil.getCellTexts("A1:C9"));

        // "Data->Sort...", sort first by Column B "Ascending", sort second by
        // Column C "Descending", sort third by Column A "Descending"
        app.dispatch(".uno:DataSort");
        SortPage_By1.select(2); // "Column B"
        SortPage_Ascending1.check();
        SortPage_By2.select(3); // "Column C"
        SortPage_Descending2.check();
        SortPage_By3.select(1); // "Column A"
        SortPage_Descending3.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result", expectedResultSortThirdByA, CalcUtil.getCellTexts("A1:C9"));

        // Uodo/redo
        app.dispatch(".uno:Undo");
        assertArrayEquals("Undo sorted result", expectedResultSortSecondByC, CalcUtil.getCellTexts("A1:C9"));
        app.dispatch(".uno:Redo");
        assertArrayEquals("Redo sorted result", expectedResultSortThirdByA, CalcUtil.getCellTexts("A1:C9"));

        // Save and close document
        saveTo = getPath("temp/" + "SortCriteriaSortThirdBy1.ods");
        app.dispatch(".uno:SaveAs");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        app.dispatch(".uno:CloseDoc");
        openStartcenter();

        // Reopen and verify data sort is not lost
        app.dispatch(".uno:Open");
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        assertArrayEquals("Saved sorted result", expectedResultSortThirdByA, CalcUtil.getCellTexts("A1:C9"));
    }

    /**
     * Test sort options: custom sort order, predefined in preferences from copy
     * list
     *
     * @throws Exception
     */
    @Test
    public void testSortOptionsCustomSortOrderPredefineFromCopyList() throws Exception {

        // Dependencies start
        CalcUtil.selectRange("A1");
        typeKeys("red<down>yellow<down>blue<down>green<down>white<down>black");
        CalcUtil.selectRange("A1:A6");
        app.dispatch(".uno:ChangeCaseToLower"); // In case SC capitalize first
                                                // letter automatically

        // Select the cell range,
        // "Tools->Options...->OpenOffice.org Spreadsheets->Sort Lists"
        CalcUtil.selectRange("A1:A6");
        app.dispatch(".uno:OptionsTreeDialog");
        OptionsDlgList.collapseAll();
        // // Select "Sort Lists": start. Shrink the tree list and select
        // OptionsDlgList.select(0);
        // typeKeys("<left>");
        // for (int i=0; i<6; i++) {
        // typeKeys("<down><left>");
        // }
        OptionsDlgList.expand(3);
        // typeKeys("<right>");
        OptionsDlgList.select(7);
        // // Select "Sort Lists": end

        // Click "Copy" button, "OK", close the document
        OptionsDlg_SortListsTabCopy.click();
        OptionsDlg.ok();
        app.dispatch(".uno:CloseDoc");
        MsgBox_AdditionalRowsNotSaved.no();
        // Dependencies end

        // Create a new spreadsheet document
        app.dispatch("private:factory/scalc");

        // Input some data
        String[][] data = new String[][] { { "Color" }, { "black" }, { "yellow" }, { "blue" }, { "black" }, { "white" }, { "red" }, };
        String[][] expectedResultNoCustomSortOrder = new String[][] { { "Color" }, { "black" }, { "black" }, { "blue" }, { "red" }, { "white" }, { "yellow" }, };
        String[][] expectedResultCustomSortOrder = new String[][] { { "Color" }, { "red" }, { "yellow" }, { "blue" }, { "white" }, { "black" }, { "black" }, };
        CalcUtil.selectRange("A1");
        typeKeys("Color<down>black<down>yellow<down>blue<down>black<down>white<down>red");
        CalcUtil.selectRange("A2:A7");
        app.dispatch(".uno:ChangeCaseToLower"); // In case SC capitalize first
                                                // letter automatically

        // "Data->Sort...", "Options" tab, check "Range contains column labels",
        // no custom sort order, "Ascending", sort first by Color
        CalcUtil.selectRange("A1:A7");
        app.dispatch(".uno:DataSort");
        SortOptionsPage.select();
        SortOptionsPage_RangeContainsColumnLabels.check();
        SortOptionsPage_CustomSortOrder.uncheck();
        SortPage.select();
        SortPage_By1.select(1); // "Color"
        SortPage_Ascending1.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result without custom sort order", expectedResultNoCustomSortOrder, CalcUtil.getCellTexts("A1:A7"));

        // Uodo/redo
        app.dispatch(".uno:Undo");
        assertArrayEquals("Undo sorted result", data, CalcUtil.getCellTexts("A1:A7"));
        app.dispatch(".uno:Redo");
        assertArrayEquals("Redo sorted result", expectedResultNoCustomSortOrder, CalcUtil.getCellTexts("A1:A7"));
        app.dispatch(".uno:Undo");

        // Copy original data to sheet2
        CalcUtil.selectRange("A1:A7");
        app.dispatch(".uno:Copy");
        app.dispatch(".uno:SelectTables");
        SCSheetsList.select(1);
        SCSelectSheetsDlg.ok();
        app.dispatch(".uno:Paste");

        // "Data->Sort...", "Options" tab, check "Range contains column labels",
        // choose custom sort order, "Ascending", sort first by Color
        app.dispatch(".uno:DataSort");
        SortOptionsPage.select();
        SortOptionsPage_RangeContainsColumnLabels.check();
        SortOptionsPage_CustomSortOrder.check();
        SortOptionsPage_CustomSortOrderList.select("red,yellow,blue,green,white,black");
        SortPage.select();
        SortPage_By1.select(1); // "Color"
        SortPage_Ascending1.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result with custom sort order", expectedResultCustomSortOrder, CalcUtil.getCellTexts("A1:A7"));

        // Uodo/redo
        app.dispatch(".uno:Undo");
        assertArrayEquals("Undo sorted result", data, CalcUtil.getCellTexts("A1:A7"));
        app.dispatch(".uno:Redo");
        assertArrayEquals("Redo sorted result", expectedResultCustomSortOrder, CalcUtil.getCellTexts("A1:A7"));

        // Save and close document
        String saveTo = getPath("temp/" + "SortOptionsCustomSortOrderPredefineFromCopyList.ods");
        app.dispatch(".uno:SaveAs");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        app.dispatch(".uno:CloseDoc");
        openStartcenter();

        // Reopen and verify sorted result
        app.dispatch(".uno:Open");
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        app.dispatch(".uno:SelectTables");
        SCSheetsList.select(0); // Sheet 1
        SCSelectSheetsDlg.ok();
        assertArrayEquals("Original data", data, CalcUtil.getCellTexts("$A1:$A7"));
        app.dispatch(".uno:SelectTables");
        SCSheetsList.select(1); // Sheet 2
        SCSelectSheetsDlg.ok();
        assertArrayEquals("Saved sorted result", expectedResultCustomSortOrder, CalcUtil.getCellTexts("$A1:$A7"));
    }

    /**
     * Test sort options: custom sort order, predefined in preferences from new
     * list
     *
     * @throws Exception
     */
    @Test
    public void testSortOptionsCustomSortOrderPredefineFromNewList() throws Exception {

        // Dependencies start
        // "Tools->Options...->OpenOffice.org Spreadsheets->Sort Lists"
        app.dispatch(".uno:OptionsTreeDialog");
        // Select "Sort Lists": start. Shrink the tree list and select
        OptionsDlgList.select(0);
        typeKeys("<left>");
        for (int i = 0; i < 6; i++) {
            typeKeys("<down><left>");
        }
        OptionsDlgList.select(3);
        typeKeys("<right>");
        OptionsDlgList.select(7);
        // Select "Sort Lists": end

        // Click "New" button, input "white,red,yellow,blue,green,black", press
        // "Add" and "OK", close the document
        OptionsDlg_SortListsTabNew.click();
        typeKeys("white,red,yellow,blue,green,black");
        OptionsDlg_SortListsTabAdd.click();
        OptionsDlg.ok();
        app.dispatch(".uno:CloseDoc");
        // Dependencies end

        // Create a new spreadsheet document
        app.dispatch("private:factory/scalc");

        // Input some data
        String[][] data = new String[][] { { "Color" }, { "black" }, { "yellow" }, { "blue" }, { "black" }, { "white" }, { "red" }, };
        String[][] expectedResultNoCustomSortOrder = new String[][] { { "Color" }, { "black" }, { "black" }, { "blue" }, { "red" }, { "white" }, { "yellow" }, };
        String[][] expectedResultCustomSortOrder = new String[][] { { "Color" }, { "white" }, { "red" }, { "yellow" }, { "blue" }, { "black" }, { "black" }, };
        CalcUtil.selectRange("A1");
        typeKeys("Color<down>black<down>yellow<down>blue<down>black<down>white<down>red");
        CalcUtil.selectRange("A2:A7");
        app.dispatch(".uno:ChangeCaseToLower"); // In case SC capitalize first
                                                // letter automatically

        // "Data->Sort...", "Options" tab, check "Range contains column labels",
        // no custom sort order, "Ascending", sort first by Color
        CalcUtil.selectRange("A1:A7");
        app.dispatch(".uno:DataSort");
        SortOptionsPage.select();
        SortOptionsPage_RangeContainsColumnLabels.check();
        SortOptionsPage_CustomSortOrder.uncheck();
        SortPage.select();
        SortPage_By1.select(1); // "Color"
        SortPage_Ascending1.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result without custom sort order", expectedResultNoCustomSortOrder, CalcUtil.getCellTexts("A1:A7"));

        // Uodo/redo
        app.dispatch(".uno:Undo");
        assertArrayEquals("Undo sorted result", data, CalcUtil.getCellTexts("A1:A7"));
        app.dispatch(".uno:Redo");
        assertArrayEquals("Redo sorted result", expectedResultNoCustomSortOrder, CalcUtil.getCellTexts("A1:A7"));
        app.dispatch(".uno:Undo");

        // Copy original data to sheet2
        CalcUtil.selectRange("A1:A7");
        app.dispatch(".uno:Copy");
        app.dispatch(".uno:SelectTables");
        SCSheetsList.select(1); // Sheet 2
        SCSelectSheetsDlg.ok();
        app.dispatch(".uno:Paste");

        // "Data->Sort...", "Options" tab, check "Range contains column labels",
        // choose custom sort order, "Ascending", sort first by Color
        app.dispatch(".uno:DataSort");
        SortOptionsPage.select();
        SortOptionsPage_RangeContainsColumnLabels.check();
        SortOptionsPage_CustomSortOrder.check();
        SortOptionsPage_CustomSortOrderList.select("white,red,yellow,blue,green,black");
        SortPage.select();
        SortPage_By1.select(1); // "Color"
        SortPage_Ascending1.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result with custom sort order", expectedResultCustomSortOrder, CalcUtil.getCellTexts("A1:A7"));

        // Uodo/redo
        app.dispatch(".uno:Undo");
        assertArrayEquals("Undo sorted result", data, CalcUtil.getCellTexts("A1:A7"));
        app.dispatch(".uno:Redo");
        assertArrayEquals("Redo sorted result", expectedResultCustomSortOrder, CalcUtil.getCellTexts("A1:A7"));

        // Save and close document
        String saveTo = getPath("temp/" + "SortOptionsCustomSortOrderPredefineFromNewList.ods");
        app.dispatch(".uno:SaveAs");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        app.dispatch(".uno:CloseDoc");
        openStartcenter();

        // Reopen and verify sorted result
        app.dispatch(".uno:Open");
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        app.dispatch(".uno:SelectTables");
        SCSheetsList.select(0); // Sheet 1
        SCSelectSheetsDlg.ok();
        assertArrayEquals("Original data", data, CalcUtil.getCellTexts("$A1:$A7"));
        app.dispatch(".uno:SelectTables");
        SCSheetsList.select(1); // Sheet 2
        SCSelectSheetsDlg.ok();
        assertArrayEquals("Saved sorted result", expectedResultCustomSortOrder, CalcUtil.getCellTexts("$A1:$A7"));
    }

    /**
     * Test sort options: sort columns, direction "Left to right"
     *
     * @throws Exception
     */
    @Test
    public void testSortOptionsDirectionSortColumns() throws Exception {

        // Input some data
        String[][] data = new String[][] { { "Units", "7", "27", "4", "12", "3", "6" }, };
        String[][] expectedSortedResult = new String[][] { { "Units", "3", "4", "6", "7", "12", "27" }, };
        CalcUtil.selectRange("A1");
        typeKeys("Units<right>7<right>27<right>4<right>12<right>3<right>6");
        sleep(1); // If no sleep, some strings lost

        // "Data->Sort...", check "Range contains column labels",
        // "Left to right", sort first by"Units", "Ascending"
        app.dispatch(".uno:DataSort");
        SortOptionsPage.select();
        SortOptionsPage_RangeContainsColumnLabels.check();
        SortOptionsPage_LeftToRight.check();
        SortPage.select();
        SortPage_By1.select(1); // Choose "Units"
        SortPage_Ascending1.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result", expectedSortedResult, CalcUtil.getCellTexts("A1:G1"));

        // Uodo/redo
        app.dispatch(".uno:Undo");
        assertArrayEquals("Undo sorted result", data, CalcUtil.getCellTexts("A1:G1"));
        app.dispatch(".uno:Redo");
        assertArrayEquals("Redo sorted result", expectedSortedResult, CalcUtil.getCellTexts("A1:G1"));

        // Save and close document
        String saveTo = getPath("temp/" + "SortOptionsDirectionSortColumns.ods");
        app.dispatch(".uno:SaveAs");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        app.dispatch(".uno:CloseDoc");
        openStartcenter();

        // Reopen and verify sorted result
        app.dispatch(".uno:Open");
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        assertArrayEquals("Saved sorted result", expectedSortedResult, CalcUtil.getCellTexts("A1:G1"));
    }

    /**
     * Test sort options: "Include formats"
     *
     * @throws Exception
     */
    @Test
    public void testSortOptionsIncludeFormats() throws Exception {

        String[][] dataWithCurrencyFormats = new String[][] { { "Units" }, { "$32.00" }, { "57.00 \u20ac" }, { "\u20a4 74" }, { "R$ 50.00" }, { "\u062c.\u0645. 27" },
                { "7.00 \u0440\u0443\u0431" }, };
        String[][] expectedSortedResultIncludeFormat = new String[][] { { "Units" }, { "7.00 \u0440\u0443\u0431" }, { "\u062c.\u0645. 27" }, { "$32.00" }, { "R$ 50.00" },
                { "57.00 \u20ac" }, { "\u20a4 74" }, };
        String[][] expectedSortedResultExcludeFormat = new String[][] { { "Units" }, { "$7.00" }, { "27.00 \u20ac" }, { "\u20a4 32" }, { "R$ 50.00" }, { "\u062c.\u0645. 57" },
                { "74.00 \u0440\u0443\u0431" }, };

        // Open sample file to get source data
        String file = prepareData("sc/SortOptionsIncludeFormats.ods");
        app.dispatch(".uno:Open", 3);
        submitOpenDlg(file);
        calc.waitForExistence(10, 2);

        assertArrayEquals("source", dataWithCurrencyFormats, CalcUtil.getCellTexts("A1:A7"));

        // "Data->Sort...", check "Range contains column labels", check
        // "Include formats", sort first by "Units", "Ascending"
        app.dispatch(".uno:DataSort");
        SortOptionsPage.select();
        SortOptionsPage_RangeContainsColumnLabels.check();
        SortOptionsPage_IncludeFormats.check();
        SortPage.select();
        SortPage_By1.select(1); // "Units"
        SortPage_Ascending1.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result include formats", expectedSortedResultIncludeFormat, CalcUtil.getCellTexts("A1:A7"));

        // Uodo/redo
        app.dispatch(".uno:Undo");
        assertArrayEquals("Undo sorted result", dataWithCurrencyFormats, CalcUtil.getCellTexts("A1:A7"));
        app.dispatch(".uno:Redo");
        assertArrayEquals("Redo sorted result", expectedSortedResultIncludeFormat, CalcUtil.getCellTexts("A1:A7"));
        app.dispatch(".uno:Undo");

        // Copy the original data to sheet2
        CalcUtil.selectRange("A1:A7");
        app.dispatch(".uno:Copy");
        CalcUtil.selectRange("Sheet2.A1");
        app.dispatch(".uno:Paste");

        // "Data->Sort...", check "Range contains column labels", uncheck
        // "Include formats", sort first by "Units", "Ascending"
        app.dispatch(".uno:DataSort");
        SortOptionsPage.select();
        SortOptionsPage_RangeContainsColumnLabels.check();
        SortOptionsPage_IncludeFormats.uncheck();
        SortPage.select();
        SortPage_By1.select(1); // "Units"
        SortPage_Ascending1.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result exclude formats", expectedSortedResultExcludeFormat, CalcUtil.getCellTexts("A1:A7"));

        // Uodo/redo
        app.dispatch(".uno:Undo");
        assertArrayEquals("Undo sorted result", dataWithCurrencyFormats, CalcUtil.getCellTexts("A1:A7"));
        app.dispatch(".uno:Redo");
        assertArrayEquals("Redo sorted result", expectedSortedResultExcludeFormat, CalcUtil.getCellTexts("A1:A7"));

        // Save and close document
        String saveTo = getPath("temp/" + "SortOptionsIncludeFormats.ods");
        app.dispatch(".uno:SaveAs");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        app.dispatch(".uno:CloseDoc");
        openStartcenter();

        // Reopen and verify sorted result
        app.dispatch(".uno:Open");
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        assertArrayEquals("Original data", dataWithCurrencyFormats, CalcUtil.getCellTexts("$Sheet1.$A1:$A7"));
        assertArrayEquals("Saved sorted result exclude format", expectedSortedResultExcludeFormat, CalcUtil.getCellTexts("$Sheet2.$A1:$A7"));
    }

    /**
     * Test sort options: multiple sort, data overlap
     *
     * @throws Exception
     */
    @Test
    public void testSortOptionsMultipleSortDataOverlap() throws Exception {

        // Input some data
        String[][] data1 = new String[][] { { "D" }, { "C" }, { "B" }, { "A" }, { "E" }, };
        String[][] expectedSortedResult1 = new String[][] { { "A" }, { "B" }, { "C" }, { "D" }, { "E" }, };
        String[][] data2 = new String[][] { { "4" }, { "2" }, { "5" }, { "1" }, { "3" }, };
        String[][] expectedSortedResultDataOverlap = new String[][] { { "A" }, { "B" }, { "C" }, { "1" }, { "2" }, { "3" }, { "4" }, { "5" }, };
        CalcUtil.selectRange("A1");
        typeKeys("D<down>C<down>B<down>A<down>E");
        sleep(1); // If no sleep, some strings lost

        // "Data->Sort...", uncheck "Range contains column labels", sort first
        // by "Column A", "Ascending"
        app.dispatch(".uno:DataSort");
        SortOptionsPage.select();
        SortOptionsPage_RangeContainsColumnLabels.uncheck();
        SortPage.select();
        SortPage_By1.select(1); // "Column A"
        SortPage_Ascending1.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result", expectedSortedResult1, CalcUtil.getCellTexts("A1:A5"));

        // Uodo/redo
        app.dispatch(".uno:Undo");
        assertArrayEquals("Undo sorted result", data1, CalcUtil.getCellTexts("A1:A5"));
        app.dispatch(".uno:Redo");
        assertArrayEquals("Redo sorted result", expectedSortedResult1, CalcUtil.getCellTexts("A1:A5"));

        // Input data2 into same sheet, data1 and data2 are not overlapped
        CalcUtil.selectRange("G10");
        typeKeys("4<down>2<down>5<down>1<down>3");

        // Focus on data2, "Data->Sort...", "Copy result to" partially overlap
        // with data1, sort first by "Column G", "Ascending"
        CalcUtil.selectRange("G10");
        app.dispatch(".uno:DataSort");
        SortOptionsPage.select();
        SortOptionsPage_RangeContainsColumnLabels.uncheck();
        SortOptionsPage_CopyResultTo.check();
        SortOptionsPage_CopyResultToEdit.setText("A4");
        SortPage.select();
        SortPage_By1.select(1); // "Column G"
        SortPage_Ascending1.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result data overlap", expectedSortedResultDataOverlap, CalcUtil.getCellTexts("A1:A8"));

        // Save and close document
        String saveTo = getPath("temp/" + "SortOptionsMultipleSortDataOverlap.ods");
        app.dispatch(".uno:SaveAs");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        app.dispatch(".uno:CloseDoc");
        openStartcenter();

        // Reopen and verify sorted result
        app.dispatch(".uno:Open");
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        assertArrayEquals("Saved sorted result", expectedSortedResultDataOverlap, CalcUtil.getCellTexts("A1:A8"));
        assertArrayEquals("Original data2", data2, CalcUtil.getCellTexts("G10:G14"));
    }

    /**
     * Test sort options: multiple sort, no data overlap, sort parameter saved
     * correctly
     *
     * @throws Exception
     */
    @Test
    public void testSortOptionsMultipleSortSortParameterSaved() throws Exception {

        // Input some data
        String[][] data1 = new String[][] { { "D" }, { "C" }, { "B" }, { "A" }, { "E" }, };
        String[][] expectedSortedResult1 = new String[][] { { "A" }, { "B" }, { "C" }, { "D" }, { "E" }, };
        String[][] data2 = new String[][] { { "Numbers" }, { "4" }, { "2" }, { "5" }, { "1" }, { "3" }, };
        String[][] expectedSortedResult2 = new String[][] { { "Numbers" }, { "1" }, { "2" }, { "3" }, { "4" }, { "5" }, };
        CalcUtil.selectRange("A1");
        typeKeys("D<down>C<down>B<down>A<down>E");
        sleep(1); // If no sleep, some strings lost

        // "Data->Sort...", uncheck "Range contains column labels", check
        // "Case sensitive" and "Include formats", sort first by "Column A",
        // "Ascending"
        app.dispatch(".uno:DataSort");
        SortOptionsPage.select();
        SortOptionsPage_RangeContainsColumnLabels.uncheck();
        SortOptionsPage_CaseSensitive.check();
        SortOptionsPage_IncludeFormats.check();
        SortPage.select();
        SortPage_By1.select(1); // "Column A"
        SortPage_Ascending1.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result1", expectedSortedResult1, CalcUtil.getCellTexts("A1:A5"));

        // Uodo/redo
        app.dispatch(".uno:Undo");
        assertArrayEquals("Undo sorted result", data1, CalcUtil.getCellTexts("A1:A5"));
        app.dispatch(".uno:Redo");
        assertArrayEquals("Redo sorted result", expectedSortedResult1, CalcUtil.getCellTexts("A1:A5"));

        // Input data2 into same sheet, data1 and data2 are not overlapped
        CalcUtil.selectRange("G10");
        typeKeys("Numbers<down>4<down>2<down>5<down>1<down>3");

        // Focus on data2, "Data->Sort...", check
        // "Range contains column labels", uncheck "Case sensitive" and
        // "Include formats", sort first by "Numbers", "Ascending"
        CalcUtil.selectRange("G10");
        app.dispatch(".uno:DataSort");
        SortOptionsPage.select();
        SortOptionsPage_RangeContainsColumnLabels.check();
        SortOptionsPage_CaseSensitive.uncheck();
        SortOptionsPage_IncludeFormats.uncheck();
        SortPage.select();
        SortPage_By1.select(1); // "Numbers"
        SortPage_Ascending1.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result2", expectedSortedResult2, CalcUtil.getCellTexts("G10:G15"));

        // Uodo/redo
        app.dispatch(".uno:Undo");
        assertArrayEquals("Undo sorted result", data2, CalcUtil.getCellTexts("G10:G15"));
        app.dispatch(".uno:Redo");
        assertArrayEquals("Redo sorted result", expectedSortedResult2, CalcUtil.getCellTexts("G10:G15"));

        // Open sort dialog, check its setting
        app.dispatch(".uno:DataSort");
        SortOptionsPage.select();
        assertTrue("Range contains column labels should be checked", SortOptionsPage_RangeContainsColumnLabels.isChecked());
        assertFalse("Case sensitive should not be checked", SortOptionsPage_CaseSensitive.isChecked());
        assertFalse("Include formats should not be checked", SortOptionsPage_IncludeFormats.isChecked());
        SortOptionsPage.ok();

        // Save and close document
        String saveTo = getPath("temp/" + "SortOptionsMultipleSortParameterSaved.ods");
        app.dispatch(".uno:SaveAs");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        app.dispatch(".uno:CloseDoc");
        openStartcenter();

        // Reopen and verify sorted result and sort parameters
        app.dispatch(".uno:Open");
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        assertArrayEquals("Saved sorted result1", expectedSortedResult1, CalcUtil.getCellTexts("A1:A5"));
        assertArrayEquals("Saved sorted result2", expectedSortedResult2, CalcUtil.getCellTexts("G10:G15"));
        app.dispatch(".uno:DataSort");
        SortOptionsPage.select();
        assertTrue("Range contains column labels should be checked", SortOptionsPage_RangeContainsColumnLabels.isChecked());
        assertFalse("Case sensitive should not be checked", SortOptionsPage_CaseSensitive.isChecked());
        assertFalse("Include formats should not be checked", SortOptionsPage_IncludeFormats.isChecked());
        SortOptionsPage.ok();
    }

}
