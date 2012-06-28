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



package testcase.sc.celltext;
import static testlib.AppUtil.*;
import static testlib.UIMap.*;

import java.awt.Rectangle;
import java.io.File;

import org.junit.After;
import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;

import org.junit.Before;
import org.junit.Ignore;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.GraphicsUtil;

import testlib.CalcUtil;
import testlib.Log;

public class InsertRowsFromMainMenu {

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
     * Insert rows from main menu.
     *
     * @throws Exception
     */
    @Test
    public void insertRowsFromMainMenu(){
        //Create a new text document
        startcenter.menuItem("File->New->Spreadsheet").select();
        sleep(3);
        //input 1 into cell A1
        String expect = "Test123";
        CalcUtil.selectRange("A1");
        typeKeys(expect + "<enter>");
        CalcUtil.selectRange("A1");
        // Insert 1 columns before column A form main menu,so Cell A1 move A2, check it
        calc.menuItem("Insert->Rows...").select();
        assertEquals("1",SC_NumberOfColumnsRowsToInsert.getText());
        SC_InsertColumnsRowsdialog.ok();
        sleep(1);
        assertEquals(expect, CalcUtil.getCellText("A2"));
        //input Test123 into A5, select first 4 rows, in insert dialog, the defaul row should be 4, check it
        CalcUtil.selectRange("A5");
        typeKeys(expect + "<enter>");
        CalcUtil.selectRange("A1:A4");
        calc.menuItem("Insert->Rows...").select();
        assertEquals("4",SC_NumberOfColumnsRowsToInsert.getText());

        //After insert 4 rows after the selected rows, the A5 move to A9, check it
        SC_InsertColumnsRowsAfterSelectColumn.check();
        assertTrue(SC_InsertColumnsRowsAfterSelectColumn.isEnabled());
        SC_InsertColumnsRowsdialog.ok();
        sleep(1);
        assertEquals(expect,CalcUtil.getCellText("A9"));

        //Input some value into 1048576
        CalcUtil.selectRange("A1048576");
        typeKeys("1048576<enter>");
        assertEquals("1048576",CalcUtil.getCellText("A1048576"));
        // Select many rows input 65536 in insert Rows number, verify the warning message, check it
        CalcUtil.selectRange("A1:A50000");
        sleep(1);
        calc.menuItem("Insert->Rows...").select();
        SC_NumberOfColumnsRowsToInsert.setText("65536");
        SC_InsertColumnsRowsdialog.ok();
        assertEquals("Filled cells cannot be shifted" + "\n"
                + "beyond the sheet.",
                ActiveMsgBox.getMessage());
        typeKeys("<enter>");
    }
}
