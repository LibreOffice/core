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
package testcase.gui.sc.datapilot;

import static testlib.gui.AppUtil.*;
import static testlib.gui.UIMap.*;
import static org.openoffice.test.common.Testspace.*;
import org.junit.After;
import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Rule;
import org.openoffice.test.common.SystemUtil;

import testlib.gui.CalcUtil;
import testlib.gui.Log;

/**
 * Test Data->Pivot Table
 * Test cases in this class are unavailable. Because these test cases are based on symphony.
 * DataPilot of symphony is totally different from PivotTable of AOO.
 * Keep this class to track the test point.
 * TODO: Add test cases about Pivot Table.
 */
public class DataPivotTable {

    @Rule
    public Log LOG = new Log();

    @Before
    public void setUp() throws Exception {
        app.start(true);
    }

    @After
    public void tearDown() throws Exception {

    }

    /**
     *
     * Verify copy and paste Data Pilot Table
     */
    @Ignore("DataPilot in symphony is totally not the same in AOO")
    public void testCopyPasteDataPilotTable() {
        //Open the sample file
        String file = prepareData("source_data01.ods");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        sleep(2);

        //Create DataPilotTable and verify the content
        CalcUtil.selectRange("A1:E27");
        calc.menuItem("Data->DataPilot->Start...").select();
        CreateDataPilotTableDialog.ok();
        assertEquals("New DataPilot Table",CalcUtil.getCellText("B2"));
        assertEquals("Use the DataPilot panel to assign fields to areas in the DataPilot table.",CalcUtil.getCellText("B4"));
        assertEquals("The DataPilot panel automatically displays when the DataPilot table has focus.",CalcUtil.getCellText("B5"));
        assertEquals("Page Area",CalcUtil.getCellText("B7"));
        assertEquals("Row Area",CalcUtil.getCellText("B10"));
        assertEquals("Column Area",CalcUtil.getCellText("D9"));
        assertEquals("Data Area",CalcUtil.getCellText("D11"));

        if(DataPilotAutomaticallyUpdateCheckBox.isChecked()==false){
            DataPilotAutomaticallyUpdateCheckBox.check();
        }

        //Add field into every area
        DataPilotFieldSelect.click(1, 1);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Page").select();
        assertEquals("Locale",CalcUtil.getCellText("A1"));
        sleep(1);

        DataPilotFieldSelect.click(1,30);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Column").select();
        assertEquals("Name",CalcUtil.getCellText("A3"));
        sleep(1);

        DataPilotFieldSelect.click(1,50);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Row").select();
        assertEquals("Date",CalcUtil.getCellText("A4"));
        sleep(1);

        DataPilotFieldSelect.click(1,70);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Data by->Sum").select();
        assertEquals("Sum - Order Number",CalcUtil.getCellText("A3"));
        assertEquals("266773",CalcUtil.getCellText("J23"));
        sleep(1);

        //Copy and paste the data pilot table and verify the content
        CalcUtil.selectRange("A1:J24");
        typeKeys("<$copy>");

        CalcUtil.selectRange("A26");
        typeKeys("<$paste>");
        assertEquals("Locale",CalcUtil.getCellText("A26"));
        assertEquals("Sum - Order Number",CalcUtil.getCellText("A28"));
        assertEquals("Name",CalcUtil.getCellText("B28"));
        assertEquals("Date",CalcUtil.getCellText("A29"));
        assertEquals("Total Result",CalcUtil.getCellText("A48"));
        assertEquals("Total Result",CalcUtil.getCellText("J29"));
        assertEquals("266773",CalcUtil.getCellText("J48"));
        sleep(1);

        CalcUtil.selectRange("A1:J24");
        typeKeys("<$copy>");
        CalcUtil.selectRange("$A.$A29");
        typeKeys("<$paste>");
        assertEquals("Locale",CalcUtil.getCellText("A29"));
        assertEquals("Sum - Order Number",CalcUtil.getCellText("A31"));
        assertEquals("Name",CalcUtil.getCellText("B31"));
        assertEquals("Date",CalcUtil.getCellText("A32"));
        assertEquals("Total Result",CalcUtil.getCellText("A51"));
        assertEquals("Total Result",CalcUtil.getCellText("J32"));
        assertEquals("266773",CalcUtil.getCellText("J51"));
    }

    /**
     *
     * Verify that DP panel will be synchronized with table while add fields on panel.
     */
    @Ignore("DataPilot in symphony is totally not the same in AOO")
    public void testDataPilotAddFieldsAutoUpdate() {
        //open the sample file Create DataPilotTable and verify the content
        String file = prepareData("source_data01.ods");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        sleep(2);
        CalcUtil.selectRange("A1:E27");
        calc.menuItem("Data->DataPilot->Start...").select();
        CreateDataPilotTableDialog.ok();
        assertEquals("New DataPilot Table",CalcUtil.getCellText("B2"));
        assertEquals("Use the DataPilot panel to assign fields to areas in the DataPilot table.",CalcUtil.getCellText("B4"));
        assertEquals("The DataPilot panel automatically displays when the DataPilot table has focus.",CalcUtil.getCellText("B5"));
        assertEquals("Page Area",CalcUtil.getCellText("B7"));
        assertEquals("Row Area",CalcUtil.getCellText("B10"));
        assertEquals("Column Area",CalcUtil.getCellText("D9"));
        assertEquals("Data Area",CalcUtil.getCellText("D11"));

        if(DataPilotAutomaticallyUpdateCheckBox.isChecked()==false){
            DataPilotAutomaticallyUpdateCheckBox.check();

        }
        sleep(1);

        //Add field into every area
        DataPilotFieldSelect.click(1, 30);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Column").select();
        assertEquals("Name",CalcUtil.getCellText("A1"));
        sleep(1);

        DataPilotFieldSelect.click(1,50);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Row").select();
        assertEquals("Date",CalcUtil.getCellText("A2"));
        sleep(1);

        DataPilotFieldSelect.drag(1, 1, 184, 80);
        assertEquals("Locale",CalcUtil.getCellText("A1"));
        sleep(1);

        typeKeys("<$undo>");
        assertEquals("",CalcUtil.getCellText("A1"));
        sleep(1);

        typeKeys("<$redo>");
        assertEquals("Locale",CalcUtil.getCellText("A1"));
        sleep(1);

        DataPilotFieldSelect.click(1,70);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Data by->Sum").select();
        assertEquals("Sum - Order Number",CalcUtil.getCellText("A3"));
        assertEquals("266773",CalcUtil.getCellText("J23"));
        sleep(1);

        typeKeys("<$undo>");
        assertEquals("",CalcUtil.getCellText("A3"));
        sleep(1);

        typeKeys("<$redo>");
        assertEquals("Sum - Order Number",CalcUtil.getCellText("A3"));
        assertEquals("266773",CalcUtil.getCellText("J23"));
        sleep(1);
    }

    /**
     *
     * Verify that DP panel will be synchronized with table while add fields on panel.
     */
    @Ignore("DataPilot in symphony is totally not the same in AOO")
    public void testDataPilotAddFieldsManualUpdate() {
        //open the sample file Create DataPilotTable and verify the content
        String file = prepareData("source_data01.ods");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        sleep(2);
        CalcUtil.selectRange("A1:E27");
        calc.menuItem("Data->DataPilot->Start...").select();
        CreateDataPilotTableDialog.ok();
        assertEquals("New DataPilot Table",CalcUtil.getCellText("B2"));
        assertEquals("Use the DataPilot panel to assign fields to areas in the DataPilot table.",CalcUtil.getCellText("B4"));
        assertEquals("The DataPilot panel automatically displays when the DataPilot table has focus.",CalcUtil.getCellText("B5"));
        assertEquals("Page Area",CalcUtil.getCellText("B7"));
        assertEquals("Row Area",CalcUtil.getCellText("B10"));
        assertEquals("Column Area",CalcUtil.getCellText("D9"));
        assertEquals("Data Area",CalcUtil.getCellText("D11"));

        if(DataPilotAutomaticallyUpdateCheckBox.isChecked()==false){
            DataPilotAutomaticallyUpdateCheckBox.check();

        }

        DataPilotAutomaticallyUpdateCheckBox.uncheck();

        sleep(1);
        DataPilotFieldSelect.click(1, 30);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Column").select();
        assertEquals("New DataPilot Table",CalcUtil.getCellText("B2"));
        assertEquals("Use the DataPilot panel to assign fields to areas in the DataPilot table.",CalcUtil.getCellText("B4"));
        assertEquals("The DataPilot panel automatically displays when the DataPilot table has focus.",CalcUtil.getCellText("B5"));
        assertEquals("Page Area",CalcUtil.getCellText("B7"));
        assertEquals("Row Area",CalcUtil.getCellText("B10"));
        assertEquals("Column Area",CalcUtil.getCellText("D9"));
        assertEquals("Data Area",CalcUtil.getCellText("D11"));
        sleep(1);

        DataPilotFieldSelect.click(1,50);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Row").select();
        assertEquals("New DataPilot Table",CalcUtil.getCellText("B2"));
        assertEquals("Use the DataPilot panel to assign fields to areas in the DataPilot table.",CalcUtil.getCellText("B4"));
        assertEquals("The DataPilot panel automatically displays when the DataPilot table has focus.",CalcUtil.getCellText("B5"));
        assertEquals("Page Area",CalcUtil.getCellText("B7"));
        assertEquals("Row Area",CalcUtil.getCellText("B10"));
        assertEquals("Column Area",CalcUtil.getCellText("D9"));
        assertEquals("Data Area",CalcUtil.getCellText("D11"));
        sleep(1);

        CalcUtil.selectRange("B20");
        ActiveMsgBox.yes();
        sleep(1);
        assertEquals("Name",CalcUtil.getCellText("B1"));
        assertEquals("Date",CalcUtil.getCellText("A2"));

        DataPilotFieldSelect.drag(1, 1, 184, 80);
        assertEquals("Name",CalcUtil.getCellText("B1"));
        assertEquals("Date",CalcUtil.getCellText("A2"));
        sleep(1);

        DataPilotFieldSelect.click(1,70);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Data by->Sum").select();
        assertEquals("Name",CalcUtil.getCellText("B1"));
        assertEquals("Date",CalcUtil.getCellText("A2"));
        sleep(1);

        DataPilotTableUpdateButton.click();
        assertEquals("Locale",CalcUtil.getCellText("A1"));
        assertEquals("Sum - Order Number",CalcUtil.getCellText("A3"));
        assertEquals("266773",CalcUtil.getCellText("J23"));
        sleep(1);


        typeKeys("<$undo>");
        assertEquals("Name",CalcUtil.getCellText("B1"));
        assertEquals("Date",CalcUtil.getCellText("A2"));
        sleep(1);


        typeKeys("<$redo>");
        assertEquals("Locale",CalcUtil.getCellText("A1"));
        assertEquals("Sum - Order Number",CalcUtil.getCellText("A3"));
        assertEquals("266773",CalcUtil.getCellText("J23"));
        sleep(1);

        DataPilotAutomaticallyUpdateCheckBox.check();
        assertTrue(DataPilotAutomaticallyUpdateCheckBox.isChecked());
    }

    /**
     *
     * Verify that DP panel will be synchronized with table while add fields on panel.
     */
    @Ignore("DataPilot in symphony is totally not the same in AOO")
    public void testDataPilotChangeDataFieldOptionAutoUpdate() {
        //open the sample file Create DataPilotTable and verify the content
        String file = prepareData("source_data01.ods");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        sleep(2);
        CalcUtil.selectRange("A1:E27");
        calc.menuItem("Data->DataPilot->Start...").select();
        CreateDataPilotTableDialog.ok();
        assertEquals("New DataPilot Table",CalcUtil.getCellText("B2"));
        assertEquals("Use the DataPilot panel to assign fields to areas in the DataPilot table.",CalcUtil.getCellText("B4"));
        assertEquals("The DataPilot panel automatically displays when the DataPilot table has focus.",CalcUtil.getCellText("B5"));
        assertEquals("Page Area",CalcUtil.getCellText("B7"));
        assertEquals("Row Area",CalcUtil.getCellText("B10"));
        assertEquals("Column Area",CalcUtil.getCellText("D9"));
        assertEquals("Data Area",CalcUtil.getCellText("D11"));
        if(DataPilotAutomaticallyUpdateCheckBox.isChecked()==false){
            DataPilotAutomaticallyUpdateCheckBox.check();

        }
        sleep(1);

        DataPilotFieldSelect.click(1, 1);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Column").select();
        assertEquals("Locale",CalcUtil.getCellText("A1"));
        sleep(1);

        DataPilotFieldSelect.click(1,30);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Column").select();
        assertEquals("Name",CalcUtil.getCellText("B1"));
        sleep(1);

        DataPilotFieldSelect.click(1,50);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Row").select();
        assertEquals("Date",CalcUtil.getCellText("A3"));
        sleep(1);

        DataPilotFieldSelect.click(1,70);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Row").select();
        assertEquals("Order Number",CalcUtil.getCellText("B3"));
        sleep(1);

        DataPilotFieldSelect.click(1,90);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Data by->Sum").select();
        assertEquals("Sum - Amount",CalcUtil.getCellText("A1"));
        assertEquals("32779.17",CalcUtil.getCellInput("K30"));
        sleep(1);

        //Change Locale option settings, and verify it
        String target = "New Locale";
        DataPilotColumnArea.click(1,1);
        DataPilotColumnArea.openContextMenu();
        menuItem("Field Option").select();
        DataPilotFieldOptionFieldTabPageDialog.select();
        DataPilotFieldDisplayNameEditBox.setText(target);
        sleep(1);

        DataPilotFieldOptionFiledSubtotalsPage.select();
        sleep(1);
        DataPilotFieldOptionSubTotalsManuallyRadioButton.check();
        sleep(1);
        SC_DataPilotOptionSubtotalFunctionList.click(9,24);
        sleep(1);

        DataPilotFieldOptionSortTabPageDialog.select();


        DataPilotFieldOptionSortDescendingRadioButton.check();
        DataPilotFieldOptionFieldTabPageDialog.select();
        DataPilotFieldOptionFieldTabPageDialog.ok();
        sleep(1);

        assertEquals("New Locale",CalcUtil.getCellText("C1"));
        assertEquals("USA",CalcUtil.getCellText("C2"));
        assertEquals("USA Count - Amount",CalcUtil.getCellInput("H2"));
        assertEquals("17",CalcUtil.getCellInput("H30"));
        assertEquals("China Count - Amount",CalcUtil.getCellInput("L2"));
        assertEquals("9",CalcUtil.getCellInput("L30"));
        assertEquals("32779.17",CalcUtil.getCellInput("M30"));

        DataPilotColumnArea.click(1,30);
        DataPilotColumnArea.openContextMenu();
        menuItem("Field Option").select();
        DataPilotFieldOptionFieldTabPageDialog.select();
        DataPilotFieldShowItemWithoutDataCheckBox.check();
        DataPilotFieldOptionFieldTabPageDialog.ok();
        assertEquals("Emmy Ma",CalcUtil.getCellText("F3"));
        assertEquals("32779.17",CalcUtil.getCellInput("U30"));

        CalcUtil.selectRange("C1");
        SpreadSheetCells.openContextMenu();
        menuItem("Field Option...").select();
        DataPilotFieldOptionFieldTabPageDialog.select();
        DataPilotFieldDisplayNameEditBox.setText("Locale");
        DataPilotFieldOptionFiledSubtotalsPage.select();
        DataPilotFieldOptionSubTotalsNeverRadioButton.check();
        sleep(1);
        DataPilotFieldOptionSortTabPageDialog.select();
        DataPilotFieldOptionSortAscendingRadioButton.check();
        DataPilotFieldOptionFieldTabPageDialog.select();
        DataPilotFieldOptionFieldTabPageDialog.ok();

        assertEquals("Locale",CalcUtil.getCellText("C1"));
        assertEquals("China",CalcUtil.getCellText("C2"));
        assertEquals("USA",CalcUtil.getCellText("K2"));
        assertEquals("32779.17",CalcUtil.getCellInput("S30"));


        CalcUtil.selectRange("D1");
        SpreadSheetCells.openContextMenu();
        menuItem("Field Option...").select();
        DataPilotFieldOptionFieldTabPageDialog.select();
        DataPilotFieldShowItemWithoutDataCheckBox.uncheck();
        DataPilotFieldOptionFieldTabPageDialog.ok();
        assertEquals("32779.17",CalcUtil.getCellInput("K30"));
    }

    /**
     *
     * Verify that Verify the Drag/Move function of Grouped field
     */
    @Ignore("DataPilot in symphony is totally not the same in AOO")
    public void testDragMoveFunctionOfGroupedField() {
        String file = prepareData("source_data01.ods");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        sleep(2);
        CalcUtil.selectRange("A1:E27");
        calc.menuItem("Data->DataPilot->Start...").select();
        CreateDataPilotTableDialog.ok();
        assertEquals("New DataPilot Table",CalcUtil.getCellText("B2"));
        assertEquals("Use the DataPilot panel to assign fields to areas in the DataPilot table.",CalcUtil.getCellText("B4"));
        assertEquals("The DataPilot panel automatically displays when the DataPilot table has focus.",CalcUtil.getCellText("B5"));
        assertEquals("Page Area",CalcUtil.getCellText("B7"));
        assertEquals("Row Area",CalcUtil.getCellText("B10"));
        assertEquals("Column Area",CalcUtil.getCellText("D9"));
        assertEquals("Data Area",CalcUtil.getCellText("D11"));
        if(DataPilotAutomaticallyUpdateCheckBox.isChecked()==false){
            DataPilotAutomaticallyUpdateCheckBox.check();

        }

        DataPilotFieldSelect.click(1, 1);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Page").select();
        assertEquals("Locale",CalcUtil.getCellText("A1"));
        sleep(1);

        DataPilotFieldSelect.click(1,30);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Column").select();
        assertEquals("Name",CalcUtil.getCellText("A3"));
        sleep(1);

        DataPilotFieldSelect.click(1,50);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Row").select();
        assertEquals("Date",CalcUtil.getCellText("A4"));
        sleep(1);

        DataPilotFieldSelect.click(1,70);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Data by->Sum").select();
        assertEquals("Sum - Order Number",CalcUtil.getCellText("A3"));
        assertEquals("266773",CalcUtil.getCellText("J23"));
        sleep(1);

        CalcUtil.selectRange("B4:C4");
        SpreadSheetCells.openContextMenu();
        menuItem("Group->Group Selected Items").select();
        assertEquals("Name2",CalcUtil.getCellText("B3"));
        assertEquals("Name",CalcUtil.getCellText("C3"));
        assertEquals("Group1",CalcUtil.getCellText("D4"));
        assertEquals("266773",CalcUtil.getCellText("J24"));
        sleep(1);

        DataPilotColumnArea.click(1,1);
        DataPilotColumnArea.openContextMenu();
        menuItem("Move to Row").select();
        assertEquals("Name2",CalcUtil.getCellText("B4"));
        assertEquals("Group1",CalcUtil.getCellText("B6"));
        assertEquals("266773",CalcUtil.getCellText("K31"));
        sleep(1);

        typeKeys("<$undo>");
        assertEquals("Name2",CalcUtil.getCellText("B3"));
        assertEquals("Name",CalcUtil.getCellText("C3"));
        assertEquals("Group1",CalcUtil.getCellText("D4"));
        assertEquals("266773",CalcUtil.getCellText("J24"));
        sleep(1);

        typeKeys("<$redo>");
        assertEquals("Name2",CalcUtil.getCellText("B4"));
        assertEquals("Group1",CalcUtil.getCellText("B6"));
        assertEquals("266773",CalcUtil.getCellText("K31"));
    }

    /**
     *
     * Verify that DP panel while switch focus among tables with same/different source range.
     */
    @Ignore("DataPilot in symphony is totally not the same in AOO")
    public void testHideShowPanel() {
        String file = prepareData("source_data01.ods");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        sleep(2);
        CalcUtil.selectRange("A1:E27");
        calc.menuItem("Data->DataPilot->Start...").select();
        CreateDataPilotTableDialog.ok();
        assertEquals("New DataPilot Table",CalcUtil.getCellText("B2"));
        assertEquals("Use the DataPilot panel to assign fields to areas in the DataPilot table.",CalcUtil.getCellText("B4"));
        assertEquals("The DataPilot panel automatically displays when the DataPilot table has focus.",CalcUtil.getCellText("B5"));
        assertEquals("Page Area",CalcUtil.getCellText("B7"));
        assertEquals("Row Area",CalcUtil.getCellText("B10"));
        assertEquals("Column Area",CalcUtil.getCellText("D9"));
        assertEquals("Data Area",CalcUtil.getCellText("D11"));

        if(DataPilotAutomaticallyUpdateCheckBox.isChecked()==false){
            DataPilotAutomaticallyUpdateCheckBox.check();
        }

        DataPilotFieldSelect.click(1, 1);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Page").select();
        assertEquals("Locale",CalcUtil.getCellText("A1"));
        sleep(1);

        DataPilotFieldSelect.click(1,30);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Column").select();
        assertEquals("Name",CalcUtil.getCellText("A3"));
        sleep(1);

        DataPilotFieldSelect.click(1,50);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Row").select();
        assertEquals("Date",CalcUtil.getCellText("A4"));
        sleep(1);

        DataPilotFieldSelect.click(1,70);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Data by->Sum").select();
        assertEquals("Sum - Order Number",CalcUtil.getCellText("A3"));
        assertEquals("266773",CalcUtil.getCellText("J23"));
        sleep(1);

        CalcUtil.selectRange("A1:J24");
        typeKeys("<$copy>");

        CalcUtil.selectRange("A26");
        typeKeys("<$paste>");
        assertEquals("Locale",CalcUtil.getCellText("A26"));
        assertEquals("Sum - Order Number",CalcUtil.getCellText("A28"));
        assertEquals("Name",CalcUtil.getCellText("B28"));
        assertEquals("Date",CalcUtil.getCellText("A29"));
        assertEquals("Total Result",CalcUtil.getCellText("A48"));
        assertEquals("Total Result",CalcUtil.getCellText("J29"));
        assertEquals("266773",CalcUtil.getCellText("J48"));
        sleep(1);

        CalcUtil.selectRange("A1");
        SpreadSheetCells.openContextMenu();
        menuItem("Show DataPilot Panel").select();
        assertFalse(DataPilotPanel.exists());
        sleep(1);

        CalcUtil.selectRange("A1");
        SpreadSheetCells.openContextMenu();
        menuItem("Show DataPilot Panel").select();
        assertTrue(DataPilotPanel.exists());
        sleep(1);

        DataPilotPanel.close();
        assertFalse(DataPilotPanel.exists());
        sleep(1);

        CalcUtil.selectRange("A26");
        DataPilotButton.click();
        sleep(1);
        assertTrue(DataPilotPanel.exists());
        assertEquals("Locale",CalcUtil.getCellText("A26"));
        assertEquals("Sum - Order Number",CalcUtil.getCellText("A28"));
        assertEquals("Name",CalcUtil.getCellText("B28"));
        assertEquals("Date",CalcUtil.getCellText("A29"));
        assertEquals("Total Result",CalcUtil.getCellText("A48"));
        assertEquals("Total Result",CalcUtil.getCellText("J29"));
        assertEquals("266773",CalcUtil.getCellText("J48"));
    }

    /**
     *
     * Verify that DP panel will be synchronized with table while add fields on panel.
     */
    @Ignore("DataPilot in symphony is totally not the same in AOO")
    public void testMoveFieldInSameAreaManualUpdate() {
        String file = prepareData("source_data01.ods");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        sleep(2);
        CalcUtil.selectRange("A1:E27");
        calc.menuItem("Data->DataPilot->Start...").select();
        CreateDataPilotTableDialog.ok();
        if(DataPilotAutomaticallyUpdateCheckBox.isChecked()==false){
            DataPilotAutomaticallyUpdateCheckBox.check();

        }
        assertEquals("New DataPilot Table",CalcUtil.getCellText("B2"));
        assertEquals("Use the DataPilot panel to assign fields to areas in the DataPilot table.",CalcUtil.getCellText("B4"));
        assertEquals("The DataPilot panel automatically displays when the DataPilot table has focus.",CalcUtil.getCellText("B5"));
        assertEquals("Page Area",CalcUtil.getCellText("B7"));
        assertEquals("Row Area",CalcUtil.getCellText("B10"));
        assertEquals("Column Area",CalcUtil.getCellText("D9"));
        assertEquals("Data Area",CalcUtil.getCellText("D11"));
        sleep(1);

        DataPilotFieldSelect.click(1, 1);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Column").select();
        assertEquals("Locale",CalcUtil.getCellText("A1"));
        sleep(1);

        DataPilotFieldSelect.click(1,30);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Column").select();
        assertEquals("Name",CalcUtil.getCellText("B1"));
        sleep(1);

        DataPilotFieldSelect.click(1,50);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Row").select();
        assertEquals("Date",CalcUtil.getCellText("A3"));
        sleep(1);

        DataPilotFieldSelect.click(1,70);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Row").select();
        assertEquals("Order Number",CalcUtil.getCellText("B3"));
        sleep(1);

        DataPilotFieldSelect.click(1,90);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Data by->Sum").select();
        assertEquals("Sum - Amount",CalcUtil.getCellText("A1"));
        assertEquals("32779.17",CalcUtil.getCellInput("K30"));
        sleep(1);

        DataPilotAutomaticallyUpdateCheckBox.uncheck();
        DataPilotColumnArea.drag(1, 1, 1, 60);
        assertEquals("Name",CalcUtil.getCellText("D1"));
        assertEquals("Locale",CalcUtil.getCellText("C1"));
        sleep(1);

        DataPilotPaneRowArea.drag(1,1, 1, 60);
        assertEquals("Order Number",CalcUtil.getCellText("B3"));
        assertEquals("Date",CalcUtil.getCellText("A3"));
        sleep(1);

        CalcUtil.selectRange("A33");
        ActiveMsgBox.yes();
        assertEquals("Name",CalcUtil.getCellText("C1"));
        assertEquals("Locale",CalcUtil.getCellText("D1"));
        assertEquals("Order Number",CalcUtil.getCellText("A3"));
        assertEquals("Date",CalcUtil.getCellText("B3"));
        sleep(1);

        DataPilotColumnArea.click(1,1);
        DataPilotColumnArea.openContextMenu();
        menuItem("Move Down").select();
        assertEquals("Name",CalcUtil.getCellText("C1"));
        assertEquals("Locale",CalcUtil.getCellText("D1"));
        sleep(1);

        DataPilotPaneRowArea.click(1,30);
        DataPilotPaneRowArea.openContextMenu();
        menuItem("Move Up").select();
        assertEquals("Order Number",CalcUtil.getCellText("A3"));
        assertEquals("Date",CalcUtil.getCellText("B3"));
        sleep(1);

        CalcUtil.selectRange("A33");
        ActiveMsgBox.yes();
        assertEquals("Name",CalcUtil.getCellText("D1"));
        assertEquals("Locale",CalcUtil.getCellText("C1"));
        assertEquals("Order Number",CalcUtil.getCellText("B3"));
        assertEquals("Date",CalcUtil.getCellText("A3"));
        assertEquals("Sum - Amount",CalcUtil.getCellText("A1"));
        assertEquals("32779.17",CalcUtil.getCellInput("K30"));
        DataPilotAutomaticallyUpdateCheckBox.check();
        sleep(1);
    }

    /**
     *
     * Verify that DP panel will be synchronized with table while move fields on panel.
     */
    @Ignore("DataPilot in symphony is totally not the same in AOO")
    public void testMoveFieldToOtherAreaAutoUpdate() {
        String file = prepareData("source_data01.ods");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        sleep(2);
        CalcUtil.selectRange("A1:E27");
        calc.menuItem("Data->DataPilot->Start...").select();
        CreateDataPilotTableDialog.ok();
        if(DataPilotAutomaticallyUpdateCheckBox.isChecked()==false){
            DataPilotAutomaticallyUpdateCheckBox.check();

        }
        assertEquals("New DataPilot Table",CalcUtil.getCellText("B2"));
        assertEquals("Use the DataPilot panel to assign fields to areas in the DataPilot table.",CalcUtil.getCellText("B4"));
        assertEquals("The DataPilot panel automatically displays when the DataPilot table has focus.",CalcUtil.getCellText("B5"));
        assertEquals("Page Area",CalcUtil.getCellText("B7"));
        assertEquals("Row Area",CalcUtil.getCellText("B10"));
        assertEquals("Column Area",CalcUtil.getCellText("D9"));
        assertEquals("Data Area",CalcUtil.getCellText("D11"));
        sleep(1);

        DataPilotFieldSelect.click(1, 1);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Column").select();
        assertEquals("Locale",CalcUtil.getCellText("A1"));
        sleep(1);

        DataPilotFieldSelect.click(1,30);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Column").select();
        assertEquals("Name",CalcUtil.getCellText("B1"));
        sleep(1);

        DataPilotFieldSelect.click(1,50);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Row").select();
        assertEquals("Date",CalcUtil.getCellText("A3"));
        sleep(1);

        DataPilotFieldSelect.click(1,70);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Row").select();
        assertEquals("Order Number",CalcUtil.getCellText("B3"));
        sleep(1);

        DataPilotFieldSelect.click(1,90);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Data by->Sum").select();
        assertEquals("Sum - Amount",CalcUtil.getCellText("A1"));
        assertEquals("32779.17",CalcUtil.getCellInput("K30"));
        sleep(1);

        DataPilotPaneRowArea.drag(1,1, 1, -50);
        assertEquals("Date",CalcUtil.getCellText("D1"));
        assertEquals("32779.17",CalcUtil.getCellInput("AB31"));
        sleep(1);

        DataPilotColumnArea.click(1,50);
        DataPilotColumnArea.openContextMenu();
        menuItem("Move to Row").select();
        assertEquals("Date",CalcUtil.getCellText("B3"));
        assertEquals("32779.17",CalcUtil.getCellInput("K30"));
        sleep(1);

        typeKeys("<$undo>");
        assertEquals("Date",CalcUtil.getCellText("D1"));
        assertEquals("32779.17",CalcUtil.getCellInput("AB31"));
        sleep(1);

        typeKeys("<$redo>");
        assertEquals("Date",CalcUtil.getCellText("B3"));
        assertEquals("32779.17",CalcUtil.getCellInput("K30"));
    }

    /**
     *
     * Verify that DP panel will be synchronized with table while move fields on panel.
     */
    @Ignore("DataPilot in symphony is totally not the same in AOO")
    public void testMoveFieldToOtherAreaDiscardChange() {
        String file = prepareData("source_data01.ods");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        sleep(2);
        CalcUtil.selectRange("A1:E27");
        calc.menuItem("Data->DataPilot->Start...").select();
        CreateDataPilotTableDialog.ok();
        if(DataPilotAutomaticallyUpdateCheckBox.isChecked()==false){
            DataPilotAutomaticallyUpdateCheckBox.check();

        }
        assertEquals("New DataPilot Table",CalcUtil.getCellText("B2"));
        assertEquals("Use the DataPilot panel to assign fields to areas in the DataPilot table.",CalcUtil.getCellText("B4"));
        assertEquals("The DataPilot panel automatically displays when the DataPilot table has focus.",CalcUtil.getCellText("B5"));
        assertEquals("Page Area",CalcUtil.getCellText("B7"));
        assertEquals("Row Area",CalcUtil.getCellText("B10"));
        assertEquals("Column Area",CalcUtil.getCellText("D9"));
        assertEquals("Data Area",CalcUtil.getCellText("D11"));
        sleep(1);

        DataPilotFieldSelect.click(1, 1);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Column").select();
        assertEquals("Locale",CalcUtil.getCellText("A1"));
        sleep(1);

        DataPilotFieldSelect.click(1,30);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Column").select();
        assertEquals("Name",CalcUtil.getCellText("B1"));
        sleep(1);

        DataPilotFieldSelect.click(1,50);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Row").select();
        assertEquals("Date",CalcUtil.getCellText("A3"));
        sleep(1);

        DataPilotFieldSelect.click(1,70);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Row").select();
        assertEquals("Order Number",CalcUtil.getCellText("B3"));
        sleep(1);

        DataPilotFieldSelect.click(1,90);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Data by->Sum").select();
        assertEquals("Sum - Amount",CalcUtil.getCellText("A1"));
        assertEquals("32779.17",CalcUtil.getCellInput("K30"));
        sleep(1);

        DataPilotAutomaticallyUpdateCheckBox.uncheck();

        DataPilotPaneRowArea.drag(1,1, 1, -50);
        assertEquals("Date",CalcUtil.getCellText("A3"));
        assertEquals("32779.17",CalcUtil.getCellInput("K30"));
        sleep(1);

        CalcUtil.selectRange("B33");
        ActiveMsgBox.yes();
        assertEquals("Date",CalcUtil.getCellText("D1"));
        assertEquals("32779.17",CalcUtil.getCellInput("AB31"));
        sleep(1);

        DataPilotColumnArea.click(1,50);
        DataPilotColumnArea.openContextMenu();
        menuItem("Move to Row").select();
        assertEquals("Date",CalcUtil.getCellText("D1"));
        assertEquals("32779.17",CalcUtil.getCellInput("AB31"));

        CalcUtil.selectRange("B33");
        ActiveMsgBox.yes();
        assertEquals("Date",CalcUtil.getCellText("B3"));
        assertEquals("32779.17",CalcUtil.getCellInput("K30"));
        sleep(1);

        DataPilotAutomaticallyUpdateCheckBox.check();
    }

    /**
     *
     * Verify that DP panel will be synchronized with table while move table.
     * After the field name get changed, when you refresh the data, the DataPilot table will change to empty table
     * if you create table small than the empty table, the table size will get larger
     * if there is another DataPilot table exist in the larger range, there will pop a warning.
     */
    @Ignore("DataPilot in symphony is totally not the same in AOO")
    public void testMoveTableNarrowSpaceToShowTheTable() {
        String file = prepareData("source_data01.ods");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        sleep(2);
        CalcUtil.selectRange("A1:E27");
        calc.menuItem("Data->DataPilot->Start...").select();
        CreateDataPilotTableDialog.ok();
        assertEquals("New DataPilot Table",CalcUtil.getCellText("B2"));
        assertEquals("Use the DataPilot panel to assign fields to areas in the DataPilot table.",CalcUtil.getCellText("B4"));
        assertEquals("The DataPilot panel automatically displays when the DataPilot table has focus.",CalcUtil.getCellText("B5"));
        assertEquals("Page Area",CalcUtil.getCellText("B7"));
        assertEquals("Row Area",CalcUtil.getCellText("B10"));
        assertEquals("Column Area",CalcUtil.getCellText("D9"));
        assertEquals("Data Area",CalcUtil.getCellText("D11"));

        if(DataPilotAutomaticallyUpdateCheckBox.isChecked()==false){
            DataPilotAutomaticallyUpdateCheckBox.check();
        }

        DataPilotFieldSelect.click(1, 1);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Page").select();
        assertEquals("Locale",CalcUtil.getCellText("A1"));
        sleep(1);

        CalcUtil.selectRange("A1:B4");
        typeKeys("<$copy>");
        CalcUtil.selectRange("C1");
        typeKeys("<$paste>");
        sleep(1);

        CalcUtil.selectRange("$A.A1");
        typeKeys("New Locale" + "<enter>");

        CalcUtil.selectRange("$DataPilot_A_1.A1");
        SpreadSheetCells.openContextMenu();
        menuItem("Refresh Data").select();
        ActiveMsgBox.ok();

        CalcUtil.selectRange("A1:B4");
        SpreadSheetCells.drag(80, 45, 80, 350);

        assertEquals("New DataPilot Table",CalcUtil.getCellText("B20"));
        assertEquals("Use the DataPilot panel to assign fields to areas in the DataPilot table.",CalcUtil.getCellText("B22"));
        assertEquals("The DataPilot panel automatically displays when the DataPilot table has focus.",CalcUtil.getCellText("B23"));
        assertEquals("Page Area",CalcUtil.getCellText("B25"));
        assertEquals("Row Area",CalcUtil.getCellText("B28"));
        assertEquals("Column Area",CalcUtil.getCellText("D27"));
        assertEquals("Data Area",CalcUtil.getCellText("D29"));
        DataPilotPanel.close();
        sleep(1);

        CalcUtil.selectRange("C1:D4");
        SpreadSheetCells.drag(210, 50, 10, 50);
        assertEquals("New DataPilot Table",CalcUtil.getCellText("B2"));
        assertEquals("Use the DataPilot panel to assign fields to areas in the DataPilot table.",CalcUtil.getCellText("B4"));
        assertEquals("The DataPilot panel automatically displays when the DataPilot table has focus.",CalcUtil.getCellText("B5"));
        assertEquals("Page Area",CalcUtil.getCellText("B7"));
        assertEquals("Row Area",CalcUtil.getCellText("B10"));
        assertEquals("Column Area",CalcUtil.getCellText("D9"));
        assertEquals("Data Area",CalcUtil.getCellText("D11"));
    }

    /**
     *
     * Verify the data pilot result table refresh after source data changed
     */
    @Ignore("DataPilot in symphony is totally not the same in AOO")
    public void testOneMRowDataPilotRefresh() {
        String file = prepareData("source_data01.ods");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        sleep(2);
        calc.maximize();
        CalcUtil.selectRange("A1:E27");
        typeKeys("<$copy>");
        CalcUtil.selectRange("A1048540");
        typeKeys("<$paste>");
        calc.menuItem("Data->DataPilot->Start...").select();
        DataPilotTableToExistPlaceRadioButton.check();
        assertTrue(DataPilotTableToExistPlaceRadioButton.isChecked());
        DataPilotTableToExistPlaceEditBox.setText("$A.$F$1048540");
        CreateDataPilotTableDialog.ok();
        sleep(1);
        if(DataPilotAutomaticallyUpdateCheckBox.isChecked()==false){
            DataPilotAutomaticallyUpdateCheckBox.check();

        }
        DataPilotFieldSelect.click(1, 1);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Page").select();
        assertEquals("Locale",CalcUtil.getCellText("F1048540"));
        sleep(1);

        DataPilotFieldSelect.click(1,30);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Column").select();
        assertEquals("Name",CalcUtil.getCellText("F1048542"));
        sleep(1);

        DataPilotFieldSelect.click(1,50);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Row").select();
        assertEquals("Date",CalcUtil.getCellText("F1048543"));
        sleep(1);

        DataPilotFieldSelect.click(1,70);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Data by->Sum").select();
        assertEquals("Sum - Order Number",CalcUtil.getCellText("F1048542"));
        assertEquals("266773",CalcUtil.getCellText("O1048562"));
        sleep(1);

        CalcUtil.selectRange("D1048541");
        typeKeys("10000<enter>");
        sleep(1);

        CalcUtil.selectRange("O1048562");
        calc.menuItem("Data->DataPilot->Refresh").select();
        assertEquals("266525",CalcUtil.getCellText("O1048562"));
    }

    /**
     *
     * Verify that DP panel will be synchronized with table while remove fields on panel.
     */
    @Ignore("DataPilot in symphony is totally not the same in AOO")
    public void testRemoveFieldsDiscardChange() {
        String file = prepareData("source_data01.ods");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        sleep(2);
        CalcUtil.selectRange("A1:E27");
        calc.menuItem("Data->DataPilot->Start...").select();
        CreateDataPilotTableDialog.ok();
        assertEquals("New DataPilot Table",CalcUtil.getCellText("B2"));
        assertEquals("Use the DataPilot panel to assign fields to areas in the DataPilot table.",CalcUtil.getCellText("B4"));
        assertEquals("The DataPilot panel automatically displays when the DataPilot table has focus.",CalcUtil.getCellText("B5"));
        assertEquals("Page Area",CalcUtil.getCellText("B7"));
        assertEquals("Row Area",CalcUtil.getCellText("B10"));
        assertEquals("Column Area",CalcUtil.getCellText("D9"));
        assertEquals("Data Area",CalcUtil.getCellText("D11"));

        if(DataPilotAutomaticallyUpdateCheckBox.isChecked()==false){
            DataPilotAutomaticallyUpdateCheckBox.check();
        }

        DataPilotFieldSelect.click(1, 1);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Page").select();
        assertEquals("Locale",CalcUtil.getCellText("A1"));
        sleep(1);

        DataPilotFieldSelect.click(1,30);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Column").select();
        assertEquals("Name",CalcUtil.getCellText("A3"));
        sleep(1);

        DataPilotFieldSelect.click(1,50);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Row").select();
        assertEquals("Date",CalcUtil.getCellText("A4"));
        sleep(1);

        DataPilotFieldSelect.click(1,70);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Data by->Sum").select();
        assertEquals("Sum - Order Number",CalcUtil.getCellText("A3"));
        assertEquals("266773",CalcUtil.getCellText("J23"));
        sleep(1);

        DataPilotAutomaticallyUpdateCheckBox.uncheck();

        DataPilotColumnArea.drag(1, 1, -30, 1);
        assertEquals("Name",CalcUtil.getCellText("B3"));
        assertEquals("266773",CalcUtil.getCellText("J23"));
        sleep(1);

        DataPilotPaneRowArea.click(1,1);
        DataPilotPaneRowArea.openContextMenu();
        menuItem("Remove").select();
        assertEquals("Date",CalcUtil.getCellText("A4"));
        assertEquals("266773",CalcUtil.getCellText("J23"));
        sleep(1);

        DataPilotPanePageArea.drag(1, 1, -30, 1);
        assertEquals("Locale",CalcUtil.getCellText("A1"));
        assertEquals("266773",CalcUtil.getCellText("J23"));
        sleep(1);

        DataPiloPaneDataArea.openContextMenu();
        menuItem("Remove").select();
        assertEquals("Sum - Order Number",CalcUtil.getCellText("A3"));
        assertEquals("266773",CalcUtil.getCellText("J23"));

        CalcUtil.selectRange("B25");
        ActiveMsgBox.yes();
        assertEquals("New DataPilot Table",CalcUtil.getCellText("B2"));
        assertEquals("Use the DataPilot panel to assign fields to areas in the DataPilot table.",CalcUtil.getCellText("B4"));
        assertEquals("The DataPilot panel automatically displays when the DataPilot table has focus.",CalcUtil.getCellText("B5"));
        assertEquals("Page Area",CalcUtil.getCellText("B7"));
        assertEquals("Row Area",CalcUtil.getCellText("B10"));
        assertEquals("Column Area",CalcUtil.getCellText("D9"));
        assertEquals("Data Area",CalcUtil.getCellText("D11"));

        DataPilotAutomaticallyUpdateCheckBox.check();
    }

    /**
     *
     * Verify the Remove function of Grouped field
     */
    @Ignore("DataPilot in symphony is totally not the same in AOO")
    public void testRemoveFunctionOfGroupedField() {
        String file = prepareData("source_data01.ods");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        sleep(2);
        CalcUtil.selectRange("A1:E27");
        calc.menuItem("Data->DataPilot->Start...").select();
        CreateDataPilotTableDialog.ok();
        assertEquals("New DataPilot Table",CalcUtil.getCellText("B2"));
        assertEquals("Use the DataPilot panel to assign fields to areas in the DataPilot table.",CalcUtil.getCellText("B4"));
        assertEquals("The DataPilot panel automatically displays when the DataPilot table has focus.",CalcUtil.getCellText("B5"));
        assertEquals("Page Area",CalcUtil.getCellText("B7"));
        assertEquals("Row Area",CalcUtil.getCellText("B10"));
        assertEquals("Column Area",CalcUtil.getCellText("D9"));
        assertEquals("Data Area",CalcUtil.getCellText("D11"));

        if(DataPilotAutomaticallyUpdateCheckBox.isChecked()==false){
            DataPilotAutomaticallyUpdateCheckBox.check();
        }

        DataPilotFieldSelect.click(1, 1);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Page").select();
        assertEquals("Locale",CalcUtil.getCellText("A1"));
        sleep(1);

        DataPilotFieldSelect.click(1,30);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Column").select();
        assertEquals("Name",CalcUtil.getCellText("A3"));
        sleep(1);

        DataPilotFieldSelect.click(1,50);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Row").select();
        assertEquals("Date",CalcUtil.getCellText("A4"));
        sleep(1);

        DataPilotFieldSelect.click(1,70);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Data by->Sum").select();
        assertEquals("Sum - Order Number",CalcUtil.getCellText("A3"));
        assertEquals("266773",CalcUtil.getCellText("J23"));
        sleep(1);

        CalcUtil.selectRange("B4:C4");
        SpreadSheetCells.openContextMenu();
        menuItem("Group->Group Selected Items").select();
        assertEquals("Name2",CalcUtil.getCellText("B3"));
        assertEquals("Name",CalcUtil.getCellText("C3"));
        assertEquals("Group1",CalcUtil.getCellText("D4"));
        assertEquals("266773",CalcUtil.getCellText("J24"));
        sleep(1);

        CalcUtil.selectRange("D4");
        SpreadSheetCells.openContextMenu();
        menuItem("Group->Ungroup").select();
        assertEquals("Name",CalcUtil.getCellText("B3"));
        assertEquals("",CalcUtil.getCellText("C3"));
        assertEquals("Bill Zhang",CalcUtil.getCellText("D4"));
        assertEquals("266773",CalcUtil.getCellText("J23"));
    }

    /**
     *
     * Verify that DP panel will be synchronized with table while rename fields on panel.
     */
    @Ignore("DataPilot in symphony is totally not the same in AOO")
    public void testRenameFieldAutoUpdate() {
        String file = prepareData("source_data01.ods");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        sleep(2);
        CalcUtil.selectRange("A1:E27");
        calc.menuItem("Data->DataPilot->Start...").select();
        CreateDataPilotTableDialog.ok();
        assertEquals("New DataPilot Table",CalcUtil.getCellText("B2"));
        assertEquals("Use the DataPilot panel to assign fields to areas in the DataPilot table.",CalcUtil.getCellText("B4"));
        assertEquals("The DataPilot panel automatically displays when the DataPilot table has focus.",CalcUtil.getCellText("B5"));
        assertEquals("Page Area",CalcUtil.getCellText("B7"));
        assertEquals("Row Area",CalcUtil.getCellText("B10"));
        assertEquals("Column Area",CalcUtil.getCellText("D9"));
        assertEquals("Data Area",CalcUtil.getCellText("D11"));

        if(DataPilotAutomaticallyUpdateCheckBox.isChecked()==false){
            DataPilotAutomaticallyUpdateCheckBox.check();
        }

        DataPilotFieldSelect.click(1, 1);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Page").select();
        assertEquals("Locale",CalcUtil.getCellText("A1"));
        sleep(1);

        DataPilotFieldSelect.click(1,30);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Column").select();
        assertEquals("Name",CalcUtil.getCellText("A3"));
        sleep(1);

        DataPilotFieldSelect.click(1,50);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Row").select();
        assertEquals("Date",CalcUtil.getCellText("A4"));
        sleep(1);

        DataPilotFieldSelect.click(1,70);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Data by->Sum").select();
        assertEquals("Sum - Order Number",CalcUtil.getCellText("A3"));
        assertEquals("266773",CalcUtil.getCellText("J23"));
        sleep(1);

        DataPilotPanePageArea.click(1,1);
        DataPilotPanePageArea.openContextMenu();
        menuItem("Field Option").select();
        DataPilotFieldDisplayNameEditBox.setText("New Locale");
        DataPilotFieldOptionFieldTabPageDialog.ok();
        assertEquals("New Locale",CalcUtil.getCellText("A1"));
        sleep(1);

        typeKeys("<$undo>");
        assertEquals("Locale",CalcUtil.getCellText("A1"));
        sleep(1);

        typeKeys("<$redo>");
        assertEquals("New Locale",CalcUtil.getCellText("A1"));
        sleep(1);

        CalcUtil.selectRange("B3");
        SpreadSheetCells.openContextMenu();
        menuItem("Field Option...").select();
        DataPilotFieldDisplayNameEditBox.setText("New Name");
        DataPilotFieldOptionFieldTabPageDialog.ok();
        assertEquals("New Name",CalcUtil.getCellText("B3"));
        sleep(1);

        typeKeys("<$undo>");
        assertEquals("Name",CalcUtil.getCellText("B3"));
        sleep(1);

        typeKeys("<$redo>");
        assertEquals("New Name",CalcUtil.getCellText("B3"));
        sleep(1);

        CalcUtil.selectRange("A4");
        SpreadSheetCells.openContextMenu();
        menuItem("Field Option...").select();
        DataPilotFieldDisplayNameEditBox.setText("New Date");
        DataPilotFieldOptionFieldTabPageDialog.ok();
        assertEquals("New Date",CalcUtil.getCellText("A4"));
        sleep(1);

        typeKeys("<$undo>");
        assertEquals("Date",CalcUtil.getCellText("A4"));
        sleep(1);

        typeKeys("<$redo>");
        assertEquals("New Date",CalcUtil.getCellText("A4"));
        sleep(1);

        DataPiloPaneDataArea.openContextMenu();
        menuItem("Field Option").select();
        DataPilotFieldDisplayNameEditBox.setText("New Sum - Order Number");
        DataPilotFieldOptionFieldTabPageDialog.ok();
        assertEquals("New Sum - Order Number",CalcUtil.getCellText("A3"));
        sleep(1);

        typeKeys("<$undo>");
        assertEquals("Sum - Order Number",CalcUtil.getCellText("A3"));
        sleep(1);

        typeKeys("<$redo>");
        assertEquals("New Sum - Order Number",CalcUtil.getCellText("A3"));
        sleep(1);
    }

    /**
     *
     * Verify that DP panel will be synchronized with table while add fields on panel.
     */
    @Ignore("DataPilot in symphony is totally not the same in AOO")
    public void testSortFunctionInGroupedField() {
        String file = prepareData("source_data01.ods");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        sleep(2);
        CalcUtil.selectRange("A1:E27");
        calc.menuItem("Data->DataPilot->Start...").select();
        CreateDataPilotTableDialog.ok();
        assertEquals("New DataPilot Table",CalcUtil.getCellText("B2"));
        assertEquals("Use the DataPilot panel to assign fields to areas in the DataPilot table.",CalcUtil.getCellText("B4"));
        assertEquals("The DataPilot panel automatically displays when the DataPilot table has focus.",CalcUtil.getCellText("B5"));
        assertEquals("Page Area",CalcUtil.getCellText("B7"));
        assertEquals("Row Area",CalcUtil.getCellText("B10"));
        assertEquals("Column Area",CalcUtil.getCellText("D9"));
        assertEquals("Data Area",CalcUtil.getCellText("D11"));

        if(DataPilotAutomaticallyUpdateCheckBox.isChecked()==false){
            DataPilotAutomaticallyUpdateCheckBox.check();
        }

        DataPilotFieldSelect.click(1, 1);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Page").select();
        assertEquals("Locale",CalcUtil.getCellText("A1"));
        sleep(1);

        DataPilotFieldSelect.click(1,30);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Column").select();
        assertEquals("Name",CalcUtil.getCellText("A3"));
        sleep(1);

        DataPilotFieldSelect.click(1,50);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Row").select();
        assertEquals("Date",CalcUtil.getCellText("A4"));
        sleep(1);

        DataPilotFieldSelect.click(1,70);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Data by->Sum").select();
        assertEquals("Sum - Order Number",CalcUtil.getCellText("A3"));
        assertEquals("266773",CalcUtil.getCellText("J23"));
        sleep(1);

        CalcUtil.selectRange("B4:C4");
        SpreadSheetCells.openContextMenu();
        menuItem("Group->Group Selected Items").select();
        assertEquals("Name2",CalcUtil.getCellText("B3"));
        assertEquals("Name",CalcUtil.getCellInput("C3"));
        assertEquals("Group1",CalcUtil.getCellText("D4"));
        assertEquals("266773",CalcUtil.getCellText("J24"));
        sleep(1);

        CalcUtil.selectRange("B3");
        SpreadSheetCells.openContextMenu();
        menuItem("Field Option...").select();
        DataPilotFieldOptionSortTabPageDialog.select();

        DataPilotFieldOptionSortDescendingRadioButton.check();
        DataPilotFieldOptionFieldTabPageDialog.select();
        DataPilotFieldOptionFieldTabPageDialog.ok();
        sleep(1);

        assertEquals("Kevin Wang",CalcUtil.getCellText("B4"));
        assertEquals("Group1",CalcUtil.getCellText("F4"));
        assertEquals("Bill Zhang",CalcUtil.getCellText("I4"));
        assertEquals("266773",CalcUtil.getCellText("J24"));

        typeKeys("<$undo>");
        assertEquals("Bill Zhang",CalcUtil.getCellText("B4"));
        assertEquals("Group1",CalcUtil.getCellText("D4"));
        assertEquals("Kevin Wang",CalcUtil.getCellText("I4"));
        assertEquals("266773",CalcUtil.getCellText("J24"));
        sleep(1);

        typeKeys("<$redo>");
        assertEquals("Kevin Wang",CalcUtil.getCellText("B4"));
        assertEquals("Group1",CalcUtil.getCellText("F4"));
        assertEquals("Bill Zhang",CalcUtil.getCellText("I4"));
        assertEquals("266773",CalcUtil.getCellText("J24"));
    }

    /**
     *
     * Verify that DP panel will be synchronized with table while add fields on panel.
     */
    @Ignore("DataPilot in symphony is totally not the same in AOO")
    public void testSubtotalsFunctionInGroupedField() {
        String file = prepareData("source_data01.ods");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        sleep(2);
        CalcUtil.selectRange("A1:E27");
        calc.menuItem("Data->DataPilot->Start...").select();
        CreateDataPilotTableDialog.ok();
        assertEquals("New DataPilot Table",CalcUtil.getCellText("B2"));
        assertEquals("Use the DataPilot panel to assign fields to areas in the DataPilot table.",CalcUtil.getCellText("B4"));
        assertEquals("The DataPilot panel automatically displays when the DataPilot table has focus.",CalcUtil.getCellText("B5"));
        assertEquals("Page Area",CalcUtil.getCellText("B7"));
        assertEquals("Row Area",CalcUtil.getCellText("B10"));
        assertEquals("Column Area",CalcUtil.getCellText("D9"));
        assertEquals("Data Area",CalcUtil.getCellText("D11"));

        if(DataPilotAutomaticallyUpdateCheckBox.isChecked()==false){
            DataPilotAutomaticallyUpdateCheckBox.check();
        }

        DataPilotFieldSelect.click(1, 1);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Page").select();
        assertEquals("Locale",CalcUtil.getCellText("A1"));
        sleep(1);

        DataPilotFieldSelect.click(1,30);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Column").select();
        assertEquals("Name",CalcUtil.getCellText("A3"));
        sleep(1);

        DataPilotFieldSelect.click(1,50);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Row").select();
        assertEquals("Date",CalcUtil.getCellText("A4"));
        sleep(1);

        DataPilotFieldSelect.click(1,70);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Data by->Sum").select();
        assertEquals("Sum - Order Number",CalcUtil.getCellText("A3"));
        assertEquals("266773",CalcUtil.getCellText("J23"));
        sleep(1);

        CalcUtil.selectRange("B4:C4");
        SpreadSheetCells.openContextMenu();
        menuItem("Group->Group Selected Items").select();
        assertEquals("Name2",CalcUtil.getCellText("B3"));
        assertEquals("Name",CalcUtil.getCellText("C3"));
        assertEquals("Group1",CalcUtil.getCellText("D4"));
        assertEquals("266773",CalcUtil.getCellText("J24"));
        sleep(1);

        CalcUtil.selectRange("B3");
        SpreadSheetCells.openContextMenu();
        menuItem("Field Option...").select();
        DataPilotFieldOptionFiledSubtotalsPage.select();
        sleep(1);

        DataPilotFieldOptionSubTotalsManuallyRadioButton.check();

        SC_DataPilotOptionSubtotalFunctionList.click(9,24);
        sleep(1);
        DataPilotFieldOptionFieldTabPageDialog.select();
        DataPilotFieldOptionFieldTabPageDialog.ok();

        assertEquals("Bill Zhang Count - Order Number",CalcUtil.getCellText("C4"));
        assertEquals("5",CalcUtil.getCellText("C24"));

        assertEquals("Emmy Ma Count - Order Number",CalcUtil.getCellText("E4"));
        assertEquals("2",CalcUtil.getCellText("E24"));

        assertEquals("Group1 Count - Order Number",CalcUtil.getCellText("H4"));
        assertEquals("9",CalcUtil.getCellText("H24"));

        assertEquals("Harry Wu Count - Order Number",CalcUtil.getCellText("J4"));
        assertEquals("1",CalcUtil.getCellText("J24"));

        assertEquals("Jerry Lu Count - Order Number",CalcUtil.getCellText("L4"));
        assertEquals("2",CalcUtil.getCellText("L24"));

        assertEquals("Joe Liu Count - Order Number",CalcUtil.getCellText("N4"));
        assertEquals("4",CalcUtil.getCellText("N24"));

        assertEquals("Kevin Wang Count - Order Number",CalcUtil.getCellText("P4"));
        assertEquals("3",CalcUtil.getCellText("P24"));
        assertEquals("266773",CalcUtil.getCellText("Q24"));
        sleep(1);

        typeKeys("<$undo>");
        assertEquals("Emmy Ma",CalcUtil.getCellText("C4"));
        assertEquals("20518",CalcUtil.getCellText("C24"));

        assertEquals("",CalcUtil.getCellText("E4"));
        assertEquals("20528",CalcUtil.getCellText("E24"));

        assertEquals("Joe Liu",CalcUtil.getCellText("H4"));
        assertEquals("41056",CalcUtil.getCellText("H24"));

        assertEquals("Total Result",CalcUtil.getCellText("J4"));
        assertEquals("266773",CalcUtil.getCellText("J24"));

        assertEquals("",CalcUtil.getCellText("L4"));
        assertEquals("",CalcUtil.getCellText("L24"));

        assertEquals("",CalcUtil.getCellText("N4"));
        assertEquals("",CalcUtil.getCellText("N24"));

        assertEquals("",CalcUtil.getCellText("P4"));
        assertEquals("",CalcUtil.getCellText("P24"));
        assertEquals("",CalcUtil.getCellText("Q24"));
        sleep(1);

        typeKeys("<$redo>");
        assertEquals("Bill Zhang Count - Order Number",CalcUtil.getCellText("C4"));
        assertEquals("5",CalcUtil.getCellText("C24"));

        assertEquals("Emmy Ma Count - Order Number",CalcUtil.getCellText("E4"));
        assertEquals("2",CalcUtil.getCellText("E24"));

        assertEquals("Group1 Count - Order Number",CalcUtil.getCellText("H4"));
        assertEquals("9",CalcUtil.getCellText("H24"));

        assertEquals("Harry Wu Count - Order Number",CalcUtil.getCellText("J4"));
        assertEquals("1",CalcUtil.getCellText("J24"));

        assertEquals("Jerry Lu Count - Order Number",CalcUtil.getCellText("L4"));
        assertEquals("2",CalcUtil.getCellText("L24"));

        assertEquals("Joe Liu Count - Order Number",CalcUtil.getCellText("N4"));
        assertEquals("4",CalcUtil.getCellText("N24"));

        assertEquals("Kevin Wang Count - Order Number",CalcUtil.getCellText("P4"));
        assertEquals("3",CalcUtil.getCellText("P24"));
        assertEquals("266773",CalcUtil.getCellText("Q24"));
    }

    /**
     *
     * Verify that DP panel will be synchronized with table while add fields on panel.
     */
    @Ignore("DataPilot in symphony is totally not the same in AOO")
    public void testTopNFunctionInGroupedField() {
        String file = prepareData("source_data01.ods");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        sleep(2);
        CalcUtil.selectRange("A1:E27");
        calc.menuItem("Data->DataPilot->Start...").select();
        CreateDataPilotTableDialog.ok();
        assertEquals("New DataPilot Table",CalcUtil.getCellText("B2"));
        assertEquals("Use the DataPilot panel to assign fields to areas in the DataPilot table.",CalcUtil.getCellText("B4"));
        assertEquals("The DataPilot panel automatically displays when the DataPilot table has focus.",CalcUtil.getCellText("B5"));
        assertEquals("Page Area",CalcUtil.getCellText("B7"));
        assertEquals("Row Area",CalcUtil.getCellText("B10"));
        assertEquals("Column Area",CalcUtil.getCellText("D9"));
        assertEquals("Data Area",CalcUtil.getCellText("D11"));

        if(DataPilotAutomaticallyUpdateCheckBox.isChecked()==false){
            DataPilotAutomaticallyUpdateCheckBox.check();
        }

        DataPilotFieldSelect.click(1, 1);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Page").select();
        assertEquals("Locale",CalcUtil.getCellText("A1"));
        sleep(1);

        DataPilotFieldSelect.click(1,30);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Column").select();
        assertEquals("Name",CalcUtil.getCellText("A3"));
        sleep(1);

        DataPilotFieldSelect.click(1,50);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Row").select();
        assertEquals("Date",CalcUtil.getCellText("A4"));
        sleep(1);

        DataPilotFieldSelect.click(1,70);
        DataPilotFieldSelect.openContextMenu();
        menuItem("Add to Data by->Sum").select();
        assertEquals("Sum - Order Number",CalcUtil.getCellInput("A3"));
        assertEquals("266773",CalcUtil.getCellText("J23"));
        sleep(1);

        CalcUtil.selectRange("B4:C4");
        SpreadSheetCells.openContextMenu();
        menuItem("Group->Group Selected Items").select();
        assertEquals("Name2",CalcUtil.getCellText("B3"));
        assertEquals("Name",CalcUtil.getCellText("C3"));
        assertEquals("Group1",CalcUtil.getCellText("D4"));
        assertEquals("266773",CalcUtil.getCellText("J24"));
        sleep(1);

        CalcUtil.selectRange("A1");
        sleep(1);

        // the Display resolution is 1280 * 1050 on windows and Linux and 1920 * 1080 on Mac
        if(SystemUtil.isWindows()){
            SpreadSheetCells.click(238, 43);
        }
        else if(SystemUtil.isLinux()){
            SpreadSheetCells.click(267, 43);
        }
        else if(SystemUtil.isMac()){
            SpreadSheetCells.click(238, 43);
        }

        typeKeys("<tab>");
        typeKeys("<tab>");
        typeKeys("<tab>");
        typeKeys("<tab>");
        typeKeys("<tab>");
        typeKeys("<tab>");
        typeKeys("<enter>");
        OnlyDisplayTopNItemCheckBox.check();
        NumberOfItemShowInTopNEditBox.setText("4");
        FieldTopNSettingDialog.ok();
        sleep(1);

        assertEquals("Bill Zhang",CalcUtil.getCellText("B4"));
        assertEquals("51299",CalcUtil.getCellText("B24"));

        assertEquals("Group1",CalcUtil.getCellText("C4"));
        assertEquals("71806",CalcUtil.getCellText("C24"));
        assertEquals("",CalcUtil.getCellText("D4"));
        assertEquals("Amy Zhao",CalcUtil.getCellText("C5"));
        assertEquals("Anne Lee",CalcUtil.getCellText("D5"));
        assertEquals("20528",CalcUtil.getCellText("D24"));

        assertEquals("Joe Liu",CalcUtil.getCellText("E4"));
        assertEquals("41056",CalcUtil.getCellText("E24"));

        assertEquals("Kevin Wang",CalcUtil.getCellText("F4"));
        assertEquals("30771",CalcUtil.getCellText("F24"));

        assertEquals("Total Result",CalcUtil.getCellText("G4"));
        assertEquals("215460",CalcUtil.getCellText("G24"));
        sleep(1);

        typeKeys("<$undo>");
        assertEquals("Name2",CalcUtil.getCellText("B3"));
        assertEquals("Name",CalcUtil.getCellText("C3"));
        assertEquals("Group1",CalcUtil.getCellText("D4"));
        assertEquals("266773",CalcUtil.getCellText("J24"));

        assertEquals("Bill Zhang",CalcUtil.getCellText("B4"));
        assertEquals("51299",CalcUtil.getCellText("B24"));

        assertEquals("Emmy Ma",CalcUtil.getCellText("C4"));
        assertEquals("20518",CalcUtil.getCellText("C24"));
        assertEquals("Group1",CalcUtil.getCellText("D4"));
        assertEquals("Emmy Ma",CalcUtil.getCellText("C5"));
        assertEquals("Amy Zhao",CalcUtil.getCellText("D5"));
        assertEquals("71806",CalcUtil.getCellText("D24"));

        assertEquals("",CalcUtil.getCellText("E4"));
        assertEquals("20528",CalcUtil.getCellText("E24"));

        assertEquals("Harry Wu",CalcUtil.getCellText("F4"));
        assertEquals("10265",CalcUtil.getCellText("F24"));

        assertEquals("Jerry Lu",CalcUtil.getCellText("G4"));
        assertEquals("20530",CalcUtil.getCellText("G24"));

        assertEquals("Total Result",CalcUtil.getCellText("J4"));
        assertEquals("266773",CalcUtil.getCellText("J24"));
        sleep(1);

        typeKeys("<$redo>");
        assertEquals("Bill Zhang",CalcUtil.getCellText("B4"));
        assertEquals("51299",CalcUtil.getCellText("B24"));

        assertEquals("Group1",CalcUtil.getCellText("C4"));
        assertEquals("71806",CalcUtil.getCellText("C24"));
        assertEquals("",CalcUtil.getCellText("D4"));
        assertEquals("Amy Zhao",CalcUtil.getCellText("C5"));
        assertEquals("Anne Lee",CalcUtil.getCellText("D5"));
        assertEquals("20528",CalcUtil.getCellText("D24"));

        assertEquals("Joe Liu",CalcUtil.getCellText("E4"));
        assertEquals("41056",CalcUtil.getCellText("E24"));

        assertEquals("Kevin Wang",CalcUtil.getCellText("F4"));
        assertEquals("30771",CalcUtil.getCellText("F24"));

        assertEquals("Total Result",CalcUtil.getCellText("G4"));
        assertEquals("215460",CalcUtil.getCellText("G24"));
    }
}
