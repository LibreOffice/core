/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

/**
 *
 */
package testcase;

import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;
import static testlib.AppUtil.*;
import static testlib.UIMap.*;

import org.junit.AfterClass;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Rule;
import org.junit.Test;

import testlib.CalcUtil;
import testlib.Log;

/**
 *
 */
public class BVTFunction2 {

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
     * Test insert a chart in a draw document
     * 1. New a draw document
     * 2. Insert a chart
     * 3. Check if the chart is inserted successfully
     * @throws Exception
     */
    @Test
    public void testInsertChartInDraw() throws Exception{

        // Create a new drawing document
        startcenter.menuItem("File->New->Drawing").select();
        sleep(3);

        // Insert a chart
        draw.menuItem("Insert->Chart...").select();
        sleep(3);

        // Verify if the chart is inserted successfully
        assertTrue(chart.exists(3));
        // Focus on edit pane
        draw.click(5,5);
        sleep(1);
    }

    /**
     * Test insert a chart in a text document
     * 1. New a text document
     * 2. Insert a chart
     * 3. Check if the chart is inserted successfully
     * @throws Exception
     */
    @Test
    public void testInsertChartInDocument() throws Exception{

        // Create a new text document
        startcenter.menuItem("File->New->Text Document").select();
        sleep(3);

        // Insert a chart
        writer.menuItem("Insert->Object->Chart...").select();
        sleep(3);

        // Verify if the chart is inserted successfully
        assertTrue(chart.exists(3));
        // Focus on edit pane
        writer.click(5,5);
        sleep(1);
    }

    /**
     * Test insert a chart in a spreadsheet document
     * 1. New a spreadsheet document
     * 2. Insert a chart
     * 3. Check if the chart is inserted successfully
     * @throws Exception
     */
    @Test
    public void testInsertChartInSpreadsheet() throws Exception{

        // Create a new spreadsheet document
        startcenter.menuItem("File->New->Spreadsheet").select();
        sleep(3);

        // Insert a chart
        calc.menuItem("Insert->Chart...").select();
        sleep(3);
        Chart_Wizard.ok();

        // Verify if the chart is inserted successfully
        assertTrue(chart.exists(3));
        // Focus on edit pane
        calc.click(5,5);
        sleep(1);
    }

    /**
     * Test insert a chart in a presentation document
     * 1. New a presentation document
     * 2. Insert a chart
     * 3. Check if the chart is inserted successfully
     * @throws Exception
     */
    @Ignore("There is bug in presentation")
    public void testInsertChartInPresentation() throws Exception{

        // Create a new presentation document
        startcenter.menuItem("File->New->Presentation").select();
        PresentationWizard.ok();
        sleep(3);

        // Insert a chart
        impress.menuItem("Insert->Chart...").select();
        sleep(3);

        // Verify if the chart is inserted successfully
        assertTrue(chart.exists(3));
        // Focus on edit pane
        impress.click(5,5);
        sleep(1);
    }

    /**
     * Test insert a table in a draw document
     * 1. New a draw document
     * 2. Insert a default table
     * 3. Check if the table is inserted successfully
     * @throws Exception
     */
    @Test
    public void testInsertTableInDraw() throws Exception{

        // Create a new drawing document
        startcenter.menuItem("File->New->Drawing").select();
        sleep(3);

        // Insert a table
        draw.menuItem("Insert->Table...").select();
        InsertTable.ok();
        sleep(3);

        // Verify if the table toolbar is active
//      assertTrue(Table_Toolbar.exists(3));

        // Check the statusbar to verify if the table is inserted successfully
        assertEquals("Table selected", StatusBar.getItemText(0));
        // Focus on edit pane
        draw.click(5,5);
        sleep(1);
    }

    /**
     * Test insert a table in a text document
     * 1. New a text document
     * 2. Insert a default table
     * 3. Check if the table is inserted successfully
     * @throws Exception
     */
    @Test
    public void testInsertTableInDocument() throws Exception{

        // Create a new text document
        startcenter.menuItem("File->New->Text Document").select();
        sleep(3);

        // Insert a table
        writer.menuItem("Insert->Table...").select();
        writer_InsertTable.ok();
        sleep(3);

        // Verify if the table toolbar is active
        assertTrue(Table_Toolbar.exists(3));

        // Check the statusbar to verify if the table is inserted successfully
        assertEquals("Table1:A1", StatusBar.getItemText(7));
        // Focus on edit pane
        writer.click(5,5);
        sleep(1);
    }

    /**
     * Test insert a table in a presentation document
     * 1. New a presentation document
     * 2. Insert a default table
     * 3. Check if the table is inserted successfully
     * @throws Exception
     */
    @Test
    public void testInsertTableInPresentation() throws Exception{

        // Create a new presentation document
        startcenter.menuItem("File->New->Presentation").select();
        PresentationWizard.ok();
        sleep(3);

        // Insert a table
        impress.menuItem("Insert->Table...").select();
        InsertTable.ok();
        sleep(3);

        // Verify if the table toolbar is active
        assertTrue(Table_Toolbar.exists(3));

        // Check the statusbar to verify if the table is inserted successfully
        assertEquals("Table selected", StatusBar.getItemText(0));
        // Focus on edit pane
        impress.click(5,5);
        sleep(1);
    }


    /**
     * Test insert a function in a spreadsheet document via Sum button
     * 1. New a spreadsheet document
     * 2. Insert a function via Sum button
     * 3. Check if the result is correct
     * @throws Exception
     */
    @Test
    public void testInsertFunctionInSCViaSumButton() throws Exception{

        // Create a new spreadsheet document
        startcenter.menuItem("File->New->Spreadsheet").select();
        sleep(3);

        // Insert source numbers
        String sourceNumber1 = "5";
        String sourceNumber2 = "3";
        String expectedResult = "8";
        CalcUtil.selectRange("A1");
        typeKeys(sourceNumber1);
        CalcUtil.selectRange("B1");
        typeKeys(sourceNumber2);

        // Insert a function via Sum button
        CalcUtil.selectRange("C1");
        SC_InputBar_Sum.click();
        typeKeys("<enter>");

        // Verify if the calculated result is equal to the expected result
        assertEquals("The calculated result", expectedResult, CalcUtil.getCellText("C1"));
    }

    /**
     * Test insert a function in a spreadsheet document via inputbar
     * 1. New a spreadsheet document
     * 2. Insert a function via inputbar: POWER
     * 3. Check if the result is correct
     * @throws Exception
     */
    @Test
    public void testInsertFunctionInSCViaInputbar() throws Exception{

        // Create a new spreadsheet document
        startcenter.menuItem("File->New->Spreadsheet").select();
        sleep(3);

        // Insert source numbers and expected result
        String sourceBase = "5";
        String sourcePower = "3";
        String expectedResult = "125";
        CalcUtil.selectRange("A1");
        typeKeys(sourceBase);
        CalcUtil.selectRange("B1");
        typeKeys(sourcePower);

        // Insert a function via inputbar: POWER
        CalcUtil.selectRange("D1");
        SC_InputBar_Input.inputKeys("=POWER(A1;B1)");
        typeKeys("<enter>");

        // Verify if the calculated result is equal to the expected result
        assertEquals("The calculated result", expectedResult, CalcUtil.getCellText("D1"));
    }

    /**
     * Test insert a function in a spreadsheet document via Function Wizard Dialog
     * 1. New a spreadsheet document
     * 2. Insert a function via Function Wizard Dialog: ABS
     * 3. Check if the result is correct
     * @throws Exception
     */
    @Test
    public void testInsertFunctionInSCViaFunctionWizard() throws Exception{

        // Create a new spreadsheet document
        startcenter.menuItem("File->New->Spreadsheet").select();
        sleep(3);

        // Insert source number
        String sourceNumber = "-5";
        String expectedResult = "5";
        CalcUtil.selectRange("A1");
        typeKeys(sourceNumber);
        typeKeys("<enter>");

        // Insert a function via Function Wizard Dialog: ABS
        CalcUtil.selectRange("B1");
        calc.menuItem("Insert->Function...").select();
        SC_FunctionWizardDlg_FunctionList.doubleClick(5,5);

        SC_FunctionWizardDlg_Edit1.inputKeys("A1");
        SC_FunctionWizardDlg.ok();

        // Verify if the calculated result is equal to the expected result
        assertEquals("The calculated result", expectedResult, CalcUtil.getCellText("B1"));
    }

    @AfterClass
    public static void afterClass() {
        app.kill();
    }
}
