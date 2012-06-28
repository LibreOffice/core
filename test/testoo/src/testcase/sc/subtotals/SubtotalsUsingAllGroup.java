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



package testcase.sc.subtotals;

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

public class SubtotalsUsingAllGroup {
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
     * Verify Subtotals using all group
     */
    @Test
    public void test() {
        String file = testFile("sc/SubtotalsSampleFile.ods");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        sleep(2);
        CalcUtil.selectRange("A1:E7");
        calc.menuItem("Data->Subtotals...").select();
        sleep(1);

        assertTrue(SCSubTotalsGroup1Dialog.exists());
        assertEquals("- none -",SCSubTotalsGroupByListBox.getItemText(0));
        assertEquals("Level",SCSubTotalsGroupByListBox.getItemText(1));
        assertEquals("Code",SCSubTotalsGroupByListBox.getItemText(2));
        assertEquals("No.",SCSubTotalsGroupByListBox.getItemText(3));
        assertEquals("Team",SCSubTotalsGroupByListBox.getItemText(4));
        assertEquals("Name",SCSubTotalsGroupByListBox.getItemText(5));

        SCSubTotalsGroup1Dialog.select();
        SCSubTotalsGroupByListBox.select("Level");
        SCCalcSubTotalForColumns.click(10,45);
        sleep(1);

        SCSubTotalsGroup2Dialog.select();
        SCSubTotalsGroupByListBox.select("Team");
        SCCalcSubTotalForColumns.click(10,25);
        SCCalcSubTotolsFuncionList.click(15,60);
        sleep(1);

        SCSubTotalsGroup3Dialog.select();
        SCSubTotalsGroupByListBox.select("Name");
        SCCalcSubTotalForColumns.click(10,25);
        sleep(1);

        SCSubTotalsGroup1Dialog.select();
        SCSubTotalsGroup1Dialog.ok();
        sleep(1);

        assertEquals("20",CalcUtil.getCellText("B3"));
        assertEquals("=SUBTOTAL(9;$B$2:$B$2)",CalcUtil.getCellInput("B3"));
        assertEquals("Chcomic Sum",CalcUtil.getCellText("E3"));

        assertEquals("20",CalcUtil.getCellText("B4"));
        assertEquals("=SUBTOTAL(4;$B$2:$B$3)",CalcUtil.getCellInput("B4"));
        assertEquals("A Max",CalcUtil.getCellText("D4"));

        assertEquals("20",CalcUtil.getCellText("B6"));
        assertEquals("=SUBTOTAL(9;$B$5:$B$5)",CalcUtil.getCellInput("B6"));
        assertEquals("Elle Sum",CalcUtil.getCellText("E6"));

        assertEquals("20",CalcUtil.getCellText("B7"));
        assertEquals("=SUBTOTAL(4;$B$5:$B$6)",CalcUtil.getCellInput("B7"));
        assertEquals("B Max",CalcUtil.getCellText("D7"));

        assertEquals("20",CalcUtil.getCellText("B9"));
        assertEquals("=SUBTOTAL(9;$B$8:$B$8)",CalcUtil.getCellInput("B9"));
        assertEquals("Sweet Sum",CalcUtil.getCellText("E9"));

        assertEquals("20",CalcUtil.getCellText("B10"));
        assertEquals("=SUBTOTAL(4;$B$8:$B$9)",CalcUtil.getCellInput("B10"));
        assertEquals("C Max",CalcUtil.getCellText("D10"));

        assertEquals("12",CalcUtil.getCellText("C11"));
        assertEquals("=SUBTOTAL(9;$C$2:$C$10)",CalcUtil.getCellInput("C11"));
        assertEquals("BS Sum",CalcUtil.getCellText("A11"));

        assertEquals("30",CalcUtil.getCellText("B13"));
        assertEquals("=SUBTOTAL(9;$B$12:$B$12)",CalcUtil.getCellInput("B13"));
        assertEquals("Ally Sum",CalcUtil.getCellText("E13"));

        assertEquals("30",CalcUtil.getCellText("B14"));
        assertEquals("=SUBTOTAL(4;$B$12:$B$13)",CalcUtil.getCellInput("B14"));
        assertEquals("A Max",CalcUtil.getCellText("D14"));

        assertEquals("5",CalcUtil.getCellText("C15"));
        assertEquals("=SUBTOTAL(9;$C$12:$C$14)",CalcUtil.getCellInput("C15"));
        assertEquals("CS Sum",CalcUtil.getCellText("A15"));

        assertEquals("10",CalcUtil.getCellText("B17"));
        assertEquals("=SUBTOTAL(9;$B$16:$B$16)",CalcUtil.getCellInput("B17"));
        assertEquals("Joker Sum",CalcUtil.getCellText("E17"));

        assertEquals("10",CalcUtil.getCellText("B18"));
        assertEquals("=SUBTOTAL(4;$B$16:$B$17)",CalcUtil.getCellInput("B18"));
        assertEquals("A Max",CalcUtil.getCellText("D18"));

        assertEquals("10",CalcUtil.getCellText("B20"));
        assertEquals("=SUBTOTAL(9;$B$19:$B$19)",CalcUtil.getCellInput("B20"));
        assertEquals("Kevin Sum",CalcUtil.getCellText("E20"));

        assertEquals("10",CalcUtil.getCellText("B21"));
        assertEquals("=SUBTOTAL(4;$B$19:$B$20)",CalcUtil.getCellInput("B21"));
        assertEquals("B Max",CalcUtil.getCellText("D21"));

        assertEquals("4",CalcUtil.getCellText("C22"));
        assertEquals("=SUBTOTAL(9;$C$16:$C$21)",CalcUtil.getCellInput("C22"));
        assertEquals("MS Sum",CalcUtil.getCellText("A22"));

        assertEquals("21",CalcUtil.getCellText("C23"));
        assertEquals("=SUBTOTAL(9;$C$2:$C$22)",CalcUtil.getCellInput("C23"));
        assertEquals("Grand Total",CalcUtil.getCellText("A23"));
    }

}
