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
package testcase.gui.sc.subtotals;

import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;
import static org.openoffice.test.common.Testspace.*;
import static testlib.gui.AppUtil.*;
import static testlib.gui.UIMap.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;

import testlib.gui.CalcUtil;
import testlib.gui.Log;

/**
 *
 *
 */
public class SubtotalsFunctions {

    @Rule
    public Log LOG = new Log();

    @Before
    public void setUp() throws Exception {
        app.start(true);
        String file = prepareData("sc/SubtotalsSampleFile.ods");
        app.dispatch(".uno:Open");
        submitOpenDlg(file);
        calc.waitForExistence(10, 2);
        CalcUtil.selectRange("A1:E7");
        app.dispatch(".uno:DataSubTotals");
    }

    @After
    public void tearDown() throws Exception {

    }

    /**
     * Just support en-US language
     * Verify Average function in SubTotals
     */
    @Test
    public void testAverage() {
        SCSubTotalsGroupByListBox.select(4);    // "Team"
//      SCCalcSubTotalForColumns.click(10, 25); // In different platform, can not focus on same checkbox
        SCCalcSubTotalForColumns.select(1);
        SCCalcSubTotalForColumns.check(1);  // "Code"
        sleep(1);
        SCCalcSubTotolsFuncionList.select(2);   // "Average"
        SCSubTotalsGroup1Dialog.ok();
        sleep(1);

        assertArrayEquals("Subtotal table", new String[][] { { "Level", "Code", "No.", "Team", "Name" }, { "BS", "20", "2", "A", "Chcomic" },
                { "CS", "30", "5", "A", "Ally" }, { "MS", "10", "1", "A", "Joker" }, { "", "20", "", "A Average", "" }, { "BS", "20", "4", "B", "Elle" },
                { "MS", "10", "3", "B", "Kevin" }, { "", "15", "", "B Average", "" }, { "BS", "20", "6", "C", "Sweet" }, { "", "20", "", "C Average", "" },
                { "", "18.33333333", "", "Grand Total", "" } }, CalcUtil.getCellTexts("A1:E11"));
    }

    /**
     * Just support en-US language
     * Verify Count Numbers only function in SubTotals
     */
    @Test
    public void testCountNumbersOnly() {
//      SCCalcSubTotalForColumns.click(10, 45);
        SCCalcSubTotalForColumns.select(2);
        SCCalcSubTotalForColumns.check(2);  // "No."
        SCCalcSubTotolsFuncionList.select(6);   // "Count (numbers only)"
        SCSubTotalsGroup1Dialog.ok();
        sleep(1);

        assertArrayEquals("Subtotal table", new String[][] { { "Level", "Code", "No.", "Team", "Name" }, { "BS", "20", "4", "B", "Elle" },
                { "BS", "20", "6", "C", "Sweet" }, { "BS", "20", "2", "A", "Chcomic" }, { "BS Count", "", "3", "", "" }, { "CS", "30", "5", "A", "Ally" },
                { "CS Count", "", "1", "", "" }, { "MS", "10", "1", "A", "Joker" }, { "MS", "10", "3", "B", "Kevin" }, { "MS Count", "", "2", "", "" },
                { "Grand Total", "", "6", "", "" } }, CalcUtil.getCellTexts("A1:E11"));
    }

    /**
     * Just support en-US language
     * Verify Max Numbers function in SubTotals
     */
    @Test
    public void testMax() {
//      SCCalcSubTotalForColumns.click(10, 45);
        SCCalcSubTotalForColumns.select(2);
        SCCalcSubTotalForColumns.check(2);  // "No."
        SCCalcSubTotolsFuncionList.select(3);   // "Max"
        SCSubTotalsGroup1Dialog.ok();
        sleep(1);

        assertArrayEquals("Subtotal table", new String[][] { { "Level", "Code", "No.", "Team", "Name" }, { "BS", "20", "4", "B", "Elle" },
                { "BS", "20", "6", "C", "Sweet" }, { "BS", "20", "2", "A", "Chcomic" }, { "BS Max", "", "6", "", "" }, { "CS", "30", "5", "A", "Ally" },
                { "CS Max", "", "5", "", "" }, { "MS", "10", "1", "A", "Joker" }, { "MS", "10", "3", "B", "Kevin" }, { "MS Max", "", "3", "", "" },
                { "Grand Total", "", "6", "", "" } }, CalcUtil.getCellTexts("A1:E11"));
    }

    /**
     * Just support en-US language
     * Verify Min Numbers function in SubTotals
     */
    @Test
    public void testMin() {
//      SCCalcSubTotalForColumns.click(10, 45);
        SCCalcSubTotalForColumns.select(2);
        SCCalcSubTotalForColumns.check(2);  // "No."
        SCCalcSubTotolsFuncionList.select(4);   // "Min"
        SCSubTotalsGroup1Dialog.ok();
        sleep(1);

        assertArrayEquals("Subtotal table", new String[][] { { "Level", "Code", "No.", "Team", "Name" }, { "BS", "20", "4", "B", "Elle" },
                { "BS", "20", "6", "C", "Sweet" }, { "BS", "20", "2", "A", "Chcomic" }, { "BS Min", "", "2", "", "" }, { "CS", "30", "5", "A", "Ally" },
                { "CS Min", "", "5", "", "" }, { "MS", "10", "1", "A", "Joker" }, { "MS", "10", "3", "B", "Kevin" }, { "MS Min", "", "1", "", "" },
                { "Grand Total", "", "1", "", "" } }, CalcUtil.getCellTexts("A1:E11"));
    }

    /**
     * Just support en-US language
     * Verify Product function in SubTotals
     */
    @Test
    public void testProduct() {
//      SCCalcSubTotalForColumns.click(10, 45);
        SCCalcSubTotalForColumns.select(2);
        SCCalcSubTotalForColumns.check(2);  // "No."
        SCCalcSubTotolsFuncionList.select(5);   // "Product"
        SCSubTotalsGroup1Dialog.ok();
        sleep(1);

        assertArrayEquals("Subtotal table", new String[][] { { "Level", "Code", "No.", "Team", "Name" }, { "BS", "20", "4", "B", "Elle" },
                { "BS", "20", "6", "C", "Sweet" }, { "BS", "20", "2", "A", "Chcomic" }, { "BS Product", "", "48", "", "" }, { "CS", "30", "5", "A", "Ally" },
                { "CS Product", "", "5", "", "" }, { "MS", "10", "1", "A", "Joker" }, { "MS", "10", "3", "B", "Kevin" }, { "MS Product", "", "3", "", "" },
                { "Grand Total", "", "720", "", "" } }, CalcUtil.getCellTexts("A1:E11"));
    }

    /**
     * Just support en-US language
     * Verify StDevP (Population) function in SubTotals
     */
    @Test
    public void testStDevPPopulation() {
//      SCCalcSubTotalForColumns.click(10, 45);
        SCCalcSubTotalForColumns.select(2);
        SCCalcSubTotalForColumns.check(2);  // "No."
        SCCalcSubTotolsFuncionList.select(8);   // "StDevP (Population)"
        SCSubTotalsGroup1Dialog.ok();
        sleep(1);

        assertArrayEquals("Subtotal table", new String[][] { { "Level", "Code", "No.", "Team", "Name" }, { "BS", "20", "4", "B", "Elle" },
                { "BS", "20", "6", "C", "Sweet" }, { "BS", "20", "2", "A", "Chcomic" }, { "BS StDev", "", "1.63299316", "", "" },
                { "CS", "30", "5", "A", "Ally" }, { "CS StDev", "", "0", "", "" }, { "MS", "10", "1", "A", "Joker" }, { "MS", "10", "3", "B", "Kevin" },
                { "MS StDev", "", "1", "", "" }, { "Grand Total", "", "1.70782513", "", "" } }, CalcUtil.getCellTexts("A1:E11"));
    }

    /**
     * Just support en-US language
     * Verify sum function in SubTotals
     */
    @Test
    public void testSum() {
//      SCCalcSubTotalForColumns.click(10, 45);
        SCCalcSubTotalForColumns.select(2);
        SCCalcSubTotalForColumns.check(2);  // "No."
        SCCalcSubTotolsFuncionList.select(0);   // "Sum"
        SCSubTotalsGroup1Dialog.ok();
        sleep(1);

        assertArrayEquals("Subtotal table", new String[][] { { "Level", "Code", "No.", "Team", "Name" }, { "BS", "20", "4", "B", "Elle" },
                { "BS", "20", "6", "C", "Sweet" }, { "BS", "20", "2", "A", "Chcomic" }, { "BS Sum", "", "12", "", "" }, { "CS", "30", "5", "A", "Ally" },
                { "CS Sum", "", "5", "", "" }, { "MS", "10", "1", "A", "Joker" }, { "MS", "10", "3", "B", "Kevin" }, { "MS Sum", "", "4", "", "" },
                { "Grand Total", "", "21", "", "" } }, CalcUtil.getCellTexts("A1:E11"));
    }

    /**
     * Just support en-US language
     * Verify Var Sample function in SubTotals
     */
    @Test
    public void testVarSample() {
//      SCCalcSubTotalForColumns.click(10, 45);
        SCCalcSubTotalForColumns.select(2);
        SCCalcSubTotalForColumns.check(2);  // "No."
        SCCalcSubTotolsFuncionList.select(9);   // "Var (Sample)"
        SCSubTotalsGroup1Dialog.ok();
        sleep(1);

        assertArrayEquals("Subtotal table", new String[][] { { "Level", "Code", "No.", "Team", "Name" }, { "BS", "20", "4", "B", "Elle" },
                { "BS", "20", "6", "C", "Sweet" }, { "BS", "20", "2", "A", "Chcomic" }, { "BS Var", "", "4", "", "" }, { "CS", "30", "5", "A", "Ally" },
                { "CS Var", "", "#DIV/0!", "", "" }, { "MS", "10", "1", "A", "Joker" }, { "MS", "10", "3", "B", "Kevin" }, { "MS Var", "", "2", "", "" },
                { "Grand Total", "", "3.5", "", "" } }, CalcUtil.getCellTexts("A1:E11"));
    }

    /**
     * Just support en-US language
     * Verify Don't sort in Sub totals
     */
    @Test
    public void testDoNotSortOption() {
        SCSubTotalsGroupByListBox.select(4);    // "Team"
//      SCCalcSubTotalForColumns.click(10, 25);
        SCCalcSubTotalForColumns.select(1);
        SCCalcSubTotalForColumns.check(1);  // "Code"
        SCSubTotalsOptionsTabPage.select();
        SCSubtotalsPreSortToGroupCheckBox.uncheck();
        SCSubTotalsOptionsTabPage.ok();
        sleep(1);

        assertArrayEquals("Subtotal table", new String[][] { { "Level", "Code", "No.", "Team", "Name" }, { "BS", "20", "4", "B", "Elle" },
                { "", "20", "", "B Sum", "" }, { "BS", "20", "6", "C", "Sweet" }, { "", "20", "", "C Sum", "" }, { "BS", "20", "2", "A", "Chcomic" },
                { "CS", "30", "5", "A", "Ally" }, { "MS", "10", "1", "A", "Joker" }, { "", "60", "", "A Sum", "" }, { "MS", "10", "3", "B", "Kevin" },
                { "", "10", "", "B Sum", "" }, { "", "110", "", "Grand Total", "" } }, CalcUtil.getCellTexts("A1:E12"));
    }

    /**
     * Just support en-US language
     * Verify SubTotals Options default UI
     */
    @Test
    public void testDefaultUI() {
        assertArrayEquals("Group List: ", new String[] { "- none -", "Level", "Code", "No.", "Team", "Name" }, SCSubTotalsGroupByListBox.getItemsText());

        SCSubTotalsOptionsTabPage.select();
        assertFalse(SCSubtotalsInsertPageBreakCheckBox.isChecked());
        assertFalse(SCSubtotalsCaseSensitiveCheckBox.isChecked());
        assertTrue(SCSubtotalsPreSortToGroupCheckBox.isChecked());
        assertTrue(SCSubtotalSortAscendingRadioButton.isChecked());
        assertFalse(SCSubtotalSortDescendingRadioButton.isChecked());
        assertFalse(SCSubtotalsIncludeFormatsCheckBox.isChecked());
        assertFalse(SCSubtotalsCustomSortOrderCheckBox.isChecked());
        assertFalse(SCSubtotalsCustomSortListBox.isEnabled());
        SCSubTotalsOptionsTabPage.ok();
    }

    /**
     * Just support en-US language
     * Verify Subtotals using all group
     */
    @Test
    public void testUsingAllGroup() {
        SCSubTotalsGroup1Dialog.select();
        SCSubTotalsGroupByListBox.select(1);    // "Level"
//      SCCalcSubTotalForColumns.click(10, 45);
        SCCalcSubTotalForColumns.select(2);
        SCCalcSubTotalForColumns.check(2);  // "No."
        SCSubTotalsGroup2Dialog.select();
        SCSubTotalsGroupByListBox.select(4);    // "Team"
//      SCCalcSubTotalForColumns.click(10, 25);
        SCCalcSubTotalForColumns.select(1);
        SCCalcSubTotalForColumns.check(1);  // "Code"
        SCCalcSubTotolsFuncionList.select(3);   // "Max"
        SCSubTotalsGroup3Dialog.select();
        SCSubTotalsGroupByListBox.select(5);    // "Name"
//      SCCalcSubTotalForColumns.click(10, 25);
        SCCalcSubTotalForColumns.select(1);
        SCCalcSubTotalForColumns.check(1);  // "Code"
        SCSubTotalsGroup1Dialog.select();
        SCSubTotalsGroup1Dialog.ok();
        sleep(1);

        assertArrayEquals("Subtotal table", new String[][] { { "Level", "Code", "No.", "Team", "Name" }, { "BS", "20", "2", "A", "Chcomic" },
                { "", "20", "", "", "Chcomic Sum" }, { "", "20", "", "A Max", "" }, { "BS", "20", "4", "B", "Elle" }, { "", "20", "", "", "Elle Sum" },
                { "", "20", "", "B Max", "" }, { "BS", "20", "6", "C", "Sweet" }, { "", "20", "", "", "Sweet Sum" }, { "", "20", "", "C Max", "" },
                { "BS Sum", "", "12", "", "" }, { "CS", "30", "5", "A", "Ally" }, { "", "30", "", "", "Ally Sum" }, { "", "30", "", "A Max", "" },
                { "CS Sum", "", "5", "", "" }, { "MS", "10", "1", "A", "Joker" }, { "", "10", "", "", "Joker Sum" }, { "", "10", "", "A Max", "" },
                { "MS", "10", "3", "B", "Kevin" }, { "", "10", "", "", "Kevin Sum" }, { "", "10", "", "B Max", "" }, { "MS Sum", "", "4", "", "" },
                { "Grand Total", "", "21", "", "" } }, CalcUtil.getCellTexts("A1:E23"));
    }


    /**
     * Just support en-US language
     * Verify Subtotals_Recalculate and refresh results when data rows deleted.
     */
    @Test
    public void testRecalculateAfterRowDeleted() {
//      SCCalcSubTotalForColumns.click(10,45);
        SCCalcSubTotalForColumns.select(2);
        SCCalcSubTotalForColumns.check(2);  // "No."
        SCSubTotalsGroup1Dialog.ok();
        sleep(1);

        assertArrayEquals("Subtotal table before deleting row", new String[][] { { "Level", "Code", "No.", "Team", "Name" }, { "BS", "20", "4", "B", "Elle" },
                { "BS", "20", "6", "C", "Sweet" }, { "BS", "20", "2", "A", "Chcomic" }, { "BS Sum", "", "12", "", "" }, { "CS", "30", "5", "A", "Ally" },
                { "CS Sum", "", "5", "", "" }, { "MS", "10", "1", "A", "Joker" }, { "MS", "10", "3", "B", "Kevin" }, { "MS Sum", "", "4", "", "" },
                { "Grand Total", "", "21", "", "" } }, CalcUtil.getCellTexts("A1:E11"));

        CalcUtil.selectRange("A3:E3");
        app.dispatch(".uno:DeleteCell");
        SCDeleteCellsDeleteRowsRadioButton.check();
        SCDeleteCellsDialog.ok();
        sleep(1);

        assertArrayEquals("Subtotal table after deleting row", new String[][] { { "Level", "Code", "No.", "Team", "Name" }, { "BS", "20", "4", "B", "Elle" },
                { "BS", "20", "2", "A", "Chcomic" }, { "BS Sum", "", "6", "", "" }, { "CS", "30", "5", "A", "Ally" }, { "CS Sum", "", "5", "", "" },
                { "MS", "10", "1", "A", "Joker" }, { "MS", "10", "3", "B", "Kevin" }, { "MS Sum", "", "4", "", "" }, { "Grand Total", "", "15", "", "" },
                { "", "", "", "", "" } }, CalcUtil.getCellTexts("A1:E11"));

    }

    /**
     * Verify Recalculates when source data changed with Average function in SubTotals
     */
    @Test
    public void testRecalculateWhenDataChanged() {
        SCSubTotalsGroupByListBox.select(4);    // "Team"
//      SCCalcSubTotalForColumns.click(10,25);
        SCCalcSubTotalForColumns.select(1);
        SCCalcSubTotalForColumns.check(1);  // "Code"
        SCCalcSubTotolsFuncionList.select(2);   // "Average"
        SCSubTotalsGroup1Dialog.ok();
        sleep(1);

        CalcUtil.selectRange("B4");
        typeKeys("40" + "<enter>");
        sleep(1);
        assertEquals("30",CalcUtil.getCellText("B5"));
        assertEquals("23.33333333",CalcUtil.getCellText("B11"));

        CalcUtil.selectRange("B7");
        typeKeys("50" + "<enter>");
        sleep(1);
        assertEquals("35",CalcUtil.getCellText("B8"));
        assertEquals("30",CalcUtil.getCellText("B11"));

        CalcUtil.selectRange("B9");
        typeKeys("30" + "<enter>");
        sleep(1);
        assertEquals("30",CalcUtil.getCellText("B10"));
        assertEquals("31.66666667",CalcUtil.getCellText("B11"));
    }
}
