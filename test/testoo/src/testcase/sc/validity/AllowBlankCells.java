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


public class AllowBlankCells {
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
     * test Allow Blank cell Checkbox in Validity.
     */
    @Test
    public void testAllowBlankCells() {
        startcenter.menuItem("File->New->Spreadsheet").select();
        sleep(1);

        String a ="A";
        for(int j=1;j<=6;j++){
            CalcUtil.selectRange("Sheet1.A"+j);
            typeKeys(a);
            typeKeys("<enter>");
        }
        CalcUtil.selectRange("Sheet1.A1:A6");
        typeKeys("<$copy>");
        CalcUtil.selectRange("Sheet1.B1");
        typeKeys("<$paste>");
        CalcUtil.selectRange("Sheet1.C1");
        typeKeys("<$paste>");

        CalcUtil.selectRange("Sheet1.D1");
        calc.menuItem("Data->Validity...").select();
        sleep(1);
        SC_ValidityCriteriaTabpage.select();
        SC_ValidityCriteriaAllowList.select("Cell range");
        SC_ValiditySourceInput.setText("$A$1:$C$6");
        SC_ValidityAllowBlankCells.check();
        typeKeys("<enter>");


        CalcUtil.selectRange("Sheet1.D1");
        sleep(1);
        typeKeys(a);
        typeKeys("<enter>");
        CalcUtil.selectRange("Sheet1.D1");
        SC_CellInput.activate();
        typeKeys("<backspace>");
        sleep(1);
        typeKeys("<enter>");
        assertEquals("",CalcUtil.getCellText("Sheet1.D1"));

        typeKeys(a);
        calc.menuItem("Data->Validity...").select();
        sleep(1);
        SC_ValidityCriteriaTabpage.select();
        SC_ValidityAllowBlankCells.uncheck();

        typeKeys("<enter>");

        CalcUtil.selectRange("Sheet1.D1");
        SC_CellInput.activate();
        typeKeys("<backspace>");
        typeKeys("<enter>");
        assertEquals("Invalid value.",ActiveMsgBox.getMessage());

    }

}

