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


package testcase.uno.sc;

import static org.junit.Assert.assertEquals;

import java.util.Arrays;
import java.util.Collection;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;

import org.openoffice.test.uno.UnoApp;

import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheetView;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCell;
import com.sun.star.uno.UnoRuntime;

/**
 * Check the addition operator works in formula
 * @author test
 *
 */
@RunWith(value=Parameterized.class)
public class AddtionOperatorInFormula {

    private double[] inputData;
    private double expected;

    @Parameters
    public static Collection<Object[]> data(){
        double[] input1 = new double[] {1.34, 2004.1234};
        double[] input2 = new double[] {-0.4, -0.73};
        double[] input3 = new double[] {5.25, -0.35, 11.23, 45, -123.111};
        double[] input4 = new double[] {-0, 0, 0, -0.0000};
        return Arrays.asList(new Object[][]{
                {addtionExpectedData(input1), input1},
                {addtionExpectedData(input2), input2},
                {addtionExpectedData(input3), input3},
                {addtionExpectedData(input4), input4}
        });
    }

    public AddtionOperatorInFormula(double expected, double[] inputData) {
        this.inputData = inputData;
        this.expected = expected;
    }

    UnoApp unoApp = new UnoApp();

    XSpreadsheetDocument scDocument = null;
    XComponent scComponent = null;

    @Before
    public void setUp() throws Exception {
        unoApp.start();
    }

    @After
    public void tearDown() throws Exception {
        unoApp.closeDocument(scComponent);
        unoApp.close();
    }

    @Test
    public void testAddtion() throws Exception {
        String sheetname = "AddTest";
        String inputformula = null;
        double cellvalue = 0;

        //Create Spreadsheet file.
        scComponent = unoApp.newDocument("scalc");
        scDocument = (XSpreadsheetDocument) UnoRuntime.queryInterface(XSpreadsheetDocument.class, scComponent);

        //Create a sheet at the first place.
        XSpreadsheets spreadsheets = scDocument.getSheets();
        spreadsheets.insertNewByName(sheetname, (short) 0);
        Object sheetObj = spreadsheets.getByName(sheetname);
        XSpreadsheet sheet = (XSpreadsheet) UnoRuntime.queryInterface(XSpreadsheet.class, sheetObj);

        //Active the new sheet.
        XModel scModel = (XModel) UnoRuntime.queryInterface(XModel.class, scDocument);
        XController scController = scModel.getCurrentController();
        XSpreadsheetView sheetview = (XSpreadsheetView) UnoRuntime.queryInterface(XSpreadsheetView.class, scController);
        sheetview.setActiveSheet(sheet);

        //Input formula string in cell A1.
        XCell cell = sheet.getCellByPosition(0, 0);
        inputformula = toFormula(createFormulaString(inputData, "+"));
        cell.setFormula(inputformula);

        //Get the formula calculation result.
        cellvalue = cell.getValue();

        //Verify whether the actual result equal to the expected.
        assertEquals("Unexpected calculate result.", expected, cellvalue, 0);

    }

    //Create formula string
    private static String toFormula(String expression) {
        return "=" + expression;
    }

    private static String createFormulaString(double[] inputData, String operator) {
        StringBuffer buffer = new StringBuffer();

        for (int i = 0; i < inputData.length; i++) {
            buffer.append(inputData[i]);
            if (i < inputData.length - 1) {
                buffer.append(operator);
            }
        }
        return buffer.toString();
    }

    //Calculate the expected result
    private static double addtionExpectedData(double[] inputData){
        double data = 0;
        for (double input : inputData) {
            data += input;
        }
        return data;
    }

}
