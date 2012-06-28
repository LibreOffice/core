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



package testcase.sc.validity;

import static testlib.AppUtil.*;
import static testlib.UIMap.*;

import java.io.File;

import org.junit.After;
import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.vcl.IDList;
import org.openoffice.test.vcl.widgets.VclMessageBox;


import testlib.CalcUtil;
import testlib.Log;


public class AllowTimeBetween {
    private static IDList idList = new IDList(new File("./ids"));
    public static final VclMessageBox ActiveMsgBox = new VclMessageBox(idList.getId("UID_ACTIVE"), "Message on message box.");

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
     * test Allow time between  in Validity.
     */
    @Test
    public void testAllowTimeBetween() {
        startcenter.menuItem("File->New->Spreadsheet").select();
        sleep(1);

        CalcUtil.selectRange("Sheet1.A1:C5");
        calc.menuItem("Data->Validity...").select();
        sleep(1);

        SC_ValidityCriteriaTabpage.select();
        SC_ValidityCriteriaAllowList.select("Time");
        SC_ValidityDecimalCompareOperator.select("between");
        SC_ValiditySourceInput.setText("27:00");
        SC_ValidityMaxValueInput.setText("21:00");
        SC_ValidityErrorAlertTabPage.select();
        SC_ValidityShowErrorMessage.check();
        SC_ValidityErrorMessageTitle.setText("Stop to enter");
        SC_ValidityErrorMessage.setText("Invalid value");
        typeKeys("<tab>");
        typeKeys("<enter>");
        sleep(1);

        CalcUtil.selectRange("Sheet1.A1");
        SC_CellInput.activate();
        typeKeys("21:00");
        typeKeys("<enter>");
        assertEquals("09:00:00 PM",CalcUtil.getCellText("Sheet1.A1"));

        CalcUtil.selectRange("Sheet1.A2");
        SC_CellInput.activate();
        typeKeys("27:00");
        typeKeys("<enter>");
        assertEquals("27:00:00",CalcUtil.getCellText("Sheet1.A2"));

        CalcUtil.selectRange("Sheet1.A3");
        SC_CellInput.activate();
        typeKeys("1.125");
        typeKeys("<enter>");
        assertEquals("1.125",CalcUtil.getCellText("Sheet1.A3"));

        CalcUtil.selectRange("Sheet1.A4");
        SC_CellInput.activate();
        typeKeys("0.875");
        typeKeys("<enter>");
        assertEquals("0.875",CalcUtil.getCellText("Sheet1.A4"));

        CalcUtil.selectRange("Sheet1.B1");
        SC_CellInput.activate();
        typeKeys("03:00:01");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.B1"));

        CalcUtil.selectRange("Sheet1.B2");
        SC_CellInput.activate();
        typeKeys("20:59:59");
        typeKeys("<enter>");
        sleep(1);
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.B2"));

        CalcUtil.selectRange("Sheet1.B3");
        SC_CellInput.activate();
        typeKeys("1.126");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.B3"));

        CalcUtil.selectRange("Sheet1.B4");
        SC_CellInput.activate();
        typeKeys("0.874");
        typeKeys("<enter>");
        sleep(1);
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.B4"));

        CalcUtil.selectRange("Sheet1.C1");
        SC_CellInput.activate();
        typeKeys("test");
        typeKeys("<enter>");
        sleep(1);
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.C1"));

        CalcUtil.selectRange("Sheet1.C2");
        SC_CellInput.activate();
        typeKeys("24:00");
        typeKeys("<enter>");
        assertEquals("24:00:00",CalcUtil.getCellText("Sheet1.C2"));

        CalcUtil.selectRange("Sheet1.C3");
        SC_CellInput.activate();
        typeKeys("12:00");
        typeKeys("<enter>");
        sleep(1);
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.C3"));
        }

}

