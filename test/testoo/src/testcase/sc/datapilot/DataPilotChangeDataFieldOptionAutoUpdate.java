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
package testcase.sc.datapilot;

import static testlib.AppUtil.*;
import static testlib.UIMap.*;

import java.awt.Rectangle;
import java.io.File;

import org.junit.After;
import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.GraphicsUtil;

import testlib.CalcUtil;
import testlib.Log;

/**
 *
 *
 */
public class DataPilotChangeDataFieldOptionAutoUpdate {
    /**
     * TestCapture helps us to do 1. Take a screenshot when failure occurs. 2.
     * Collect extra data when OpenOffice crashes.
     */
    @Rule
    public Log LOG = new Log();

    /**
     * initApp helps us to do 1. Patch the OpenOffice to enable automation if
     * necessary. 2. Start OpenOffice with automation enabled if necessary. 3.
     * Reset OpenOffice to startcenter.
     *
     * @throws java.lang.Exception
     */
    @Before
    public void setUp() throws Exception {
        initApp();
    }
    /**
     *
     * Verify that DP panel will be synchronized with table while add fields on panel.
     */
    @Test
    public void test() {
        //open the sample file Create DataPilotTable and verify the content
        String file = testFile("source_data01.ods");
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

}
