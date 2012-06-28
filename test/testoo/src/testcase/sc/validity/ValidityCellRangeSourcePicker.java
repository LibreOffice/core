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


public class ValidityCellRangeSourcePicker {
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
     * test Cell range source picker in Validity. Input from Edit Box.
     */
    @Test
    public void testValidityCellRangeSourcePicker() {
        startcenter.menuItem("File->New->Spreadsheet").select();
        sleep(1);

        CalcUtil.selectRange("Sheet1.A1");
        calc.menuItem("Data->Validity...").select();
        sleep(1);


        SC_ValidityCriteriaTabpage.select();
        SC_ValidityCriteriaAllowList.select("Cell range");
        SC_ValiditySourcePicker.click();
        assertEquals(false,SC_ValidityCriteriaAllowList.exists());
        SC_ValiditySourceInput.setText("$Sheet1.$A$2:$C$5");
        SC_ValiditySourcePicker.click();
        assertEquals(true,SC_ValidityCriteriaAllowList.exists());
        typeKeys("<enter>");

        calc.focus();
        CalcUtil.selectRange("Sheet1.A2");
        SC_CellInput.activate();
        typeKeys("test");
        typeKeys("<enter>");

        CalcUtil.selectRange("Sheet1.A1");
        SC_CellInput.activate();
        typeKeys("test32");
        typeKeys("<enter>");
        assertEquals("Invalid value.",ActiveMsgBox.getMessage());
        ActiveMsgBox.focus();
        typeKeys("<enter>");

        CalcUtil.selectRange("Sheet1.A1");
        SC_CellInput.activate();
        typeKeys("test");
        typeKeys("<enter>");
        assertEquals("Test",CalcUtil.getCellText("Sheet1.A1"));

        }

}

