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
public class MoveTableNarrowSpaceToShowTheTable {
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
     * Verify that DP panel will be synchronized with table while move table.
     * After the field name get changed, when you refresh the data, the DataPilot table will change to empty table
     * if you create table small than the empty table, the table size will get larger
     * if there is another DataPilot table exist in the larger range, there will pop a warning.
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

}
