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



package testcase.gui.sc.validity;

import static testlib.gui.AppUtil.*;
import static testlib.gui.UIMap.*;

import java.io.File;

import org.junit.After;
import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Ignore;
import org.junit.Test;
import org.openoffice.test.vcl.IDList;
import org.openoffice.test.vcl.widgets.VclMessageBox;

import testlib.gui.CalcUtil;
import testlib.gui.Log;


public class ValidityDialogSetting {
    private static IDList idList = new IDList(new File("./ids"));
    public static final VclMessageBox ActiveMsgBox = new VclMessageBox(idList.getId("UID_ACTIVE"), "Message on message box.");

    @Rule
    public Log LOG = new Log();

    @Before
    public void setUp() throws Exception {
        app.start();

        // New a spreadsheet, select cell range, open Validity dialog
        startcenter.menuItem("File->New->Spreadsheet").select();
        CalcUtil.selectRange("Sheet1.A1:C5");
        calc.menuItem("Data->Validity...").select();
    }

    @After
    public void tearDown() throws Exception {
        app.close();
    }

    /**
     * test Allow not between Date type in Validity.
     */
    @Test
    public void testAllowDateNotBetween() {
        SC_ValidityCriteriaTabpage.select();
        SC_ValidityCriteriaAllowList.select("Date");
        SC_ValidityDecimalCompareOperator.select("not between");
        SC_ValiditySourceInput.setText("01/01/08");
        SC_ValidityMaxValueInput.setText("03/01/08");
        SC_ValidityErrorAlertTabPage.select();
        SC_ValidityShowErrorMessage.check();
        SC_ValidityErrorMessageTitle.setText("Stop to enter");
        SC_ValidityErrorMessage.setText("Invalid value");
        SC_ValidityErrorAlertTabPage.ok();

        CalcUtil.selectRange("Sheet1.A1");
        SC_InputBar_Input.activate();
        typeKeys("12/31/07");
        typeKeys("<enter>");
        assertEquals("12/31/07",CalcUtil.getCellText("Sheet1.A1"));

        CalcUtil.selectRange("Sheet1.A2");
        SC_InputBar_Input.activate();
        typeKeys("03/02/08");
        typeKeys("<enter>");
        assertEquals("03/02/08",CalcUtil.getCellText("Sheet1.A2"));

        CalcUtil.selectRange("Sheet1.A3");
        SC_InputBar_Input.activate();
        typeKeys("01/01/08");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.A3"));

        CalcUtil.selectRange("Sheet1.A4");
        SC_InputBar_Input.activate();
        typeKeys("03/01/08");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.A4"));

        CalcUtil.selectRange("Sheet1.A5");
        SC_InputBar_Input.activate();
        typeKeys("01/02/08");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.A5"));

        CalcUtil.selectRange("Sheet1.B1");
        SC_InputBar_Input.activate();
        typeKeys("02/29/08");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.B1"));

        CalcUtil.selectRange("Sheet1.B2");
        SC_InputBar_Input.activate();
        typeKeys("test");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.B2"));

        CalcUtil.selectRange("Sheet1.B3");
        SC_InputBar_Input.activate();
        typeKeys("39448");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.B3"));
    }

    /**
     * test Allow Decimal equal in Validity.
     */
    @Test
    public void testAllowDecimalEqual() {

        SC_ValidityCriteriaTabpage.select();
        SC_ValidityCriteriaAllowList.select("Decimal");
        SC_ValidityDecimalCompareOperator.select("equal");
        SC_ValiditySourceInput.setText("0.33333333");
        SC_ValidityErrorAlertTabPage.select();
        SC_ValidityShowErrorMessage.check();
        SC_ValidityErrorMessageTitle.setText("Stop to enter");
        SC_ValidityErrorMessage.setText("Invalid value");
        SC_ValidityErrorAlertTabPage.ok();

        CalcUtil.selectRange("Sheet1.A1");
        SC_InputBar_Input.activate();
        typeKeys("0.33333333");
        typeKeys("<enter>");
        assertEquals("0.33333333",CalcUtil.getCellText("Sheet1.A1"));

        CalcUtil.selectRange("Sheet1.A2");
        SC_InputBar_Input.activate();
        typeKeys("=1/3");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.A2"));

        CalcUtil.selectRange("Sheet1.A3");
        SC_InputBar_Input.activate();
        typeKeys("0.3");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.A3"));

        CalcUtil.selectRange("Sheet1.A4");
        SC_InputBar_Input.activate();
        typeKeys("0.333333333");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.A4"));

        CalcUtil.selectRange("Sheet1.B2");
        SC_InputBar_Input.activate();
        typeKeys("test");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.B2"));
    }

    /**
     * test Allow Text length,  greater than or equal to  in Validity.
     */
    @Test
    public void testAllowGreaterTextLength() {

        SC_ValidityCriteriaTabpage.select();
        SC_ValidityCriteriaAllowList.select("Text length");
        SC_ValidityDecimalCompareOperator.select("greater than or equal to");
        SC_ValiditySourceInput.setText("10");
        SC_ValidityErrorAlertTabPage.select();
        SC_ValidityShowErrorMessage.check();
        SC_ValidityErrorMessageTitle.setText("Stop to enter");
        SC_ValidityErrorMessage.setText("Invalid value");
        SC_ValidityErrorAlertTabPage.ok();

        CalcUtil.selectRange("Sheet1.A1");
        SC_InputBar_Input.activate();
        typeKeys("testtesttesttest");
        typeKeys("<enter>");
        assertEquals("testtesttesttest",CalcUtil.getCellText("Sheet1.A1"));

        CalcUtil.selectRange("Sheet1.A2");
        SC_InputBar_Input.activate();
        typeKeys("test test ");
        typeKeys("<enter>");
        assertEquals("test test ",CalcUtil.getCellText("Sheet1.A2"));

        CalcUtil.selectRange("Sheet1.A4");
        SC_InputBar_Input.activate();
        typeKeys(" ");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.A4"));

        CalcUtil.selectRange("Sheet1.A3");
        SC_InputBar_Input.activate();
        typeKeys("Testatest");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.A3"));
    }

    /**
     * test Allow Text length,  less than  in Validity.
     */
    @Ignore("Bug 93128")
    public void testAllowLessThanTextLength() {

        calc.menuItem("Data->Validity...").select();

        SC_ValidityCriteriaTabpage.select();
        SC_ValidityCriteriaAllowList.select("Text length");
        SC_ValidityDecimalCompareOperator.select("less than");
        SC_ValiditySourceInput.setText("10");

        SC_ValidityInputHelpTabPage.select();
        SC_ValidityInputHelpCheckbox.check();
        SC_ValidityInputHelpTitle.setText("Help Info Title");
        SC_ValidityHelpMessage.setText("help info");

        SC_ValidityErrorAlertTabPage.select();
        SC_ValidityShowErrorMessage.check();
        SC_ValidityErrorAlertActionList.select("Information");
        SC_ValidityErrorMessageTitle.setText("Notes to enter");
        SC_ValidityErrorMessage.setText("Invalid value");
        SC_ValidityErrorAlertTabPage.ok();

        CalcUtil.selectRange("Sheet1.A1");
        SC_InputBar_Input.activate();
        typeKeys("testtesttesttest<enter>");
        ActiveMsgBox.ok();
        assertEquals("testtesttesttest",CalcUtil.getCellText("Sheet1.A1"));

        CalcUtil.selectRange("Sheet1.A2");
        SC_InputBar_Input.activate();
        typeKeys("sfsafsddddddd<enter>");
        ActiveMsgBox.cancel();
        assertEquals("",CalcUtil.getCellText("Sheet1.A2"));

        CalcUtil.selectRange("Sheet1.A2");
        SC_InputBar_Input.activate();
        typeKeys("10<enter>");
        assertEquals("10",CalcUtil.getCellText("Sheet1.A2"));

        CalcUtil.selectRange("Sheet1.A3");
        SC_InputBar_Input.activate();
        typeKeys("ok<enter>");
        assertEquals("ok",CalcUtil.getCellText("Sheet1.A3"));
    }

    /**
     * test Allow list.
     */
    @Test
    public void testAllowListSpecialChar() {

        SC_ValidityCriteriaTabpage.select();
        SC_ValidityCriteriaAllowList.select("List");
        SC_ValidityEntries.focus();
        typeKeys("a");
        typeKeys("<enter>");
        typeKeys("b");
        SC_ValidityErrorAlertTabPage.select();
        SC_ValidityShowErrorMessage.check();
        SC_ValidityErrorMessageTitle.setText("Stop to enter");
        SC_ValidityErrorMessage.setText("Invalid value");
        SC_ValidityCriteriaTabpage.select();
        SC_ValidityCriteriaTabpage.ok();

        calc.rightClick(1, 1);
        typeKeys("<shift s>");
        typeKeys("<down><enter>");  // Choose a
        assertEquals("a",CalcUtil.getCellText("Sheet1.A1"));

        CalcUtil.selectRange("Sheet1.B2");
        SC_InputBar_Input.activate();
        typeKeys("test");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.B2"));
    }

    /**
     * test Allow time between  in Validity.
     */
    @Test
    public void testAllowTimeBetween() {

        SC_ValidityCriteriaTabpage.select();
        SC_ValidityCriteriaAllowList.select("Time");
        SC_ValidityDecimalCompareOperator.select("between");
        SC_ValiditySourceInput.setText("27:00");
        SC_ValidityMaxValueInput.setText("21:00");
        SC_ValidityErrorAlertTabPage.select();
        SC_ValidityShowErrorMessage.check();
        SC_ValidityErrorMessageTitle.setText("Stop to enter");
        SC_ValidityErrorMessage.setText("Invalid value");
        SC_ValidityErrorAlertTabPage.ok();

        CalcUtil.selectRange("Sheet1.A1");
        SC_InputBar_Input.activate();
        typeKeys("21:00");
        typeKeys("<enter>");
        assertEquals("09:00:00 PM",CalcUtil.getCellText("Sheet1.A1"));

        CalcUtil.selectRange("Sheet1.A2");
        SC_InputBar_Input.activate();
        typeKeys("27:00");
        typeKeys("<enter>");
        assertEquals("27:00:00",CalcUtil.getCellText("Sheet1.A2"));

        CalcUtil.selectRange("Sheet1.A3");
        SC_InputBar_Input.activate();
        typeKeys("1.125");
        typeKeys("<enter>");
        assertEquals("1.125",CalcUtil.getCellText("Sheet1.A3"));

        CalcUtil.selectRange("Sheet1.A4");
        SC_InputBar_Input.activate();
        typeKeys("0.875");
        typeKeys("<enter>");
        assertEquals("0.875",CalcUtil.getCellText("Sheet1.A4"));

        CalcUtil.selectRange("Sheet1.B1");
        SC_InputBar_Input.activate();
        typeKeys("03:00:01");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.B1"));

        CalcUtil.selectRange("Sheet1.B2");
        SC_InputBar_Input.activate();
        typeKeys("20:59:59");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.B2"));

        CalcUtil.selectRange("Sheet1.B3");
        SC_InputBar_Input.activate();
        typeKeys("1.126");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.B3"));

        CalcUtil.selectRange("Sheet1.B4");
        SC_InputBar_Input.activate();
        typeKeys("0.874");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.B4"));

        CalcUtil.selectRange("Sheet1.C1");
        SC_InputBar_Input.activate();
        typeKeys("test");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.C1"));

        CalcUtil.selectRange("Sheet1.C2");
        SC_InputBar_Input.activate();
        typeKeys("24:00");
        typeKeys("<enter>");
        assertEquals("24:00:00",CalcUtil.getCellText("Sheet1.C2"));

        CalcUtil.selectRange("Sheet1.C3");
        SC_InputBar_Input.activate();
        typeKeys("12:00");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.C3"));
    }

    /**
     * test Allow time Greater than and equal to  in Validity.
     */
    @Test
    public void testAllowTimeGreaterThan() {

        SC_ValidityCriteriaTabpage.select();
        SC_ValidityCriteriaAllowList.select("Time");
        SC_ValidityDecimalCompareOperator.select("greater than or equal to");
        SC_ValiditySourceInput.setText("8:00");

        SC_ValidityErrorAlertTabPage.select();
        SC_ValidityShowErrorMessage.check();
        SC_ValidityErrorAlertActionList.select("Warning");

        SC_ValidityErrorMessageTitle.setText("warning to enter");
        SC_ValidityErrorMessage.setText("Invalid value");
        SC_ValidityErrorAlertTabPage.ok();

        CalcUtil.selectRange("Sheet1.A1");
        SC_InputBar_Input.activate();
        typeKeys("7:30");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("07:30:00 AM",CalcUtil.getCellText("Sheet1.A1"));

        CalcUtil.selectRange("Sheet1.A2");
        SC_InputBar_Input.activate();
        typeKeys("6:00");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.cancel();
        assertEquals("",CalcUtil.getCellText("Sheet1.A2"));

        CalcUtil.selectRange("Sheet1.A3");
        SC_InputBar_Input.activate();
        typeKeys("8:00");
        typeKeys("<enter>");
        assertEquals("08:00:00 AM",CalcUtil.getCellText("Sheet1.A3"));
    }

    /**
     * test Allow whole number, less than or equal to in Validity.
     */
    @Test
    public void testAllowWholeNumLessThan() {

        SC_ValidityCriteriaTabpage.select();
        SC_ValidityCriteriaAllowList.select("Whole Numbers");
        SC_ValidityDecimalCompareOperator.select("less than or equal");
        SC_ValiditySourceInput.setText("100");
        SC_ValidityErrorAlertTabPage.select();
        SC_ValidityShowErrorMessage.check();
        SC_ValidityErrorMessageTitle.setText("Stop to enter");
        SC_ValidityErrorMessage.setText("Invalid value");
        SC_ValidityErrorAlertTabPage.ok();

        CalcUtil.selectRange("Sheet1.A1");
        SC_InputBar_Input.activate();
        typeKeys("99");
        typeKeys("<enter>");
        assertEquals("99",CalcUtil.getCellText("Sheet1.A1"));

        CalcUtil.selectRange("Sheet1.A2");
        SC_InputBar_Input.activate();
        typeKeys("100");
        typeKeys("<enter>");
        assertEquals("100",CalcUtil.getCellText("Sheet1.A2"));

        CalcUtil.selectRange("Sheet1.B1");
        SC_InputBar_Input.activate();
        typeKeys("101");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.B1"));

        CalcUtil.selectRange("Sheet1.B2");
        SC_InputBar_Input.activate();
        typeKeys("45.5");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.B2"));

        CalcUtil.selectRange("Sheet1.C1");
        SC_InputBar_Input.activate();
        typeKeys("test");
        typeKeys("<enter>");
        assertEquals("Invalid value",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.C1"));
    }

    /**
     * test default message of Error Alert in Validity.
     */
    @Test
    public void testDefaultErrorAlertMessage() {

        SC_ValidityCriteriaTabpage.select();
        SC_ValidityCriteriaAllowList.select("Decimal");
        SC_ValidityDecimalCompareOperator.select("equal");
        SC_ValiditySourceInput.setText("1");

        SC_ValidityErrorAlertTabPage.select();
        SC_ValidityShowErrorMessage.check();
        SC_ValidityErrorAlertActionList.select("Stop");
        SC_ValidityErrorAlertTabPage.ok();

        CalcUtil.selectRange("Sheet1.A1");
        SC_InputBar_Input.activate();
        typeKeys("13");
        typeKeys("<enter>");
        assertEquals("OpenOffice.org Calc",ActiveMsgBox.getCaption());
        assertEquals("Invalid value.",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.A1"));
    }

    /**
     * test uncheck Error Alert in Validity.
     */
    @Test
    public void testUncheckErrorAlert() {

        SC_ValidityCriteriaTabpage.select();
        SC_ValidityCriteriaAllowList.select("Decimal");
        SC_ValidityDecimalCompareOperator.select("equal");
        SC_ValiditySourceInput.setText("1");

        SC_ValidityErrorAlertTabPage.select();
        SC_ValidityShowErrorMessage.uncheck();
        SC_ValidityErrorAlertActionList.select("Stop");
        SC_ValidityErrorAlertTabPage.ok();

        CalcUtil.selectRange("Sheet1.A1");
        SC_InputBar_Input.activate();
        typeKeys("13");
        typeKeys("<enter>");
        assertEquals("13",CalcUtil.getCellText("Sheet1.A1"));
    }

    /**
     * test Cell range source picker in Validity. Input from Edit Box.
     */
    @Test
    public void testValidityCellRangeSourcePicker() {

        SC_ValidityCriteriaTabpage.select();
        SC_ValidityCriteriaAllowList.select("Cell range");
        SC_ValiditySourcePicker.click();
        assertEquals(false,SC_ValidityCriteriaAllowList.exists());
        SC_ValiditySourceInput.setText("$Sheet1.$E$2:$G$5");
        SC_ValiditySourcePicker.click();
        assertEquals(true,SC_ValidityCriteriaAllowList.exists());

        SC_ValidityErrorAlertTabPage.select();
        SC_ValidityShowErrorMessage.check();
        SC_ValidityErrorAlertActionList.select("Stop");
        SC_ValidityErrorAlertTabPage.ok();

        // calc.focus();
        CalcUtil.selectRange("Sheet1.E2");
        SC_InputBar_Input.activate();
        typeKeys("test");
        typeKeys("<enter>");

        CalcUtil.selectRange("Sheet1.A1");
        SC_InputBar_Input.activate();
        typeKeys("test32");
        typeKeys("<enter>");
        assertEquals("Invalid value.",ActiveMsgBox.getMessage());
        ActiveMsgBox.ok();
        assertEquals("",CalcUtil.getCellText("Sheet1.A1"));

        CalcUtil.selectRange("Sheet1.B1");
        SC_InputBar_Input.activate();
        typeKeys("test");
        typeKeys("<enter>");
        assertEquals("test",CalcUtil.getCellText("Sheet1.B1"));
    }

    /**
     * test Allow Blank cell Checkbox in Validity.
     */
    @Test
    public void testAllowBlankCells() {
        SC_ValidityCriteriaTabpage.select();
        SC_ValidityCriteriaAllowList.select("Cell range");
        SC_ValiditySourceInput.setText("$E$1:$E$5");
        SC_ValidityAllowBlankCells.check();
        SC_ValidityCriteriaTabpage.ok();

        CalcUtil.selectRange("Sheet1.E1");
        typeKeys("A<enter>A<enter>A<enter>A<enter>A<enter>");

        CalcUtil.selectRange("Sheet1.A1");
        typeKeys("A<enter>");
        CalcUtil.selectRange("Sheet1.D1");
        SC_InputBar_Input.activate();
        typeKeys("<backspace><enter>");
        assertEquals("",CalcUtil.getCellText("Sheet1.D1"));

        CalcUtil.selectRange("Sheet1.B1");
        calc.menuItem("Data->Validity...").select();
        SC_ValidityCriteriaTabpage.select();
        SC_ValidityAllowBlankCells.uncheck();
        typeKeys("<enter>");

        CalcUtil.selectRange("Sheet1.B1");
        SC_InputBar_Input.activate();
        typeKeys("<backspace><enter>");
        typeKeys("<enter>");
    }
}

