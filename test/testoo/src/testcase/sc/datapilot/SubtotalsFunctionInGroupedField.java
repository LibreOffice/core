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
public class SubtotalsFunctionInGroupedField {
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

}
