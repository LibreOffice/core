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
package testcase;

import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;
import static testlib.AppUtil.*;
import static testlib.UIMap.*;

import java.awt.Rectangle;
import java.io.File;

import org.junit.AfterClass;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.GraphicsUtil;
import org.openoffice.test.common.SystemUtil;

import testlib.CalcUtil;
import testlib.Log;

/**
 *
 */
public class BVTFunction {


    @Rule
    public Log LOG = new Log();

    /**
     * @throws java.lang.Exception
     */
    @Before
    public void setUp() throws Exception {
        setUp();
    }


    @Test
    public void testExportAsPDF() throws Exception  {
        String file = testFile("export_pdf.odt");
        String exportTo = fullPath("temp/odt.pdf");

        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        writer.waitForExistence(10, 2);
        writer.menuItem("File->Export as PDF...").select();
        PDFGeneralPage.ok();
        FileUtil.deleteFile(exportTo);
        submitSaveDlg(exportTo);
        assertTrue("PDF is exported?", new File(exportTo).exists());

        // Via toolbar
        writer.menuItem("File->New->Text Document").select();
        assertTrue(toolbox(".HelpId:standardbar").exists(5));
        button(".uno:ExportDirectToPDF").click();
        assertEquals("PDF - Portable Document Format (.pdf)", FileSave_FileType.getSelText());
        FileSave.cancel();

    }

    /**
     * Test the File -- Print Dialog  show
     *
     */
    @Test
    public void testPrintDialog()
    {
        //Create a new text document
        startcenter.menuItem("File->New->Text Document").select();
        sleep(3);
        writer.menuItem("File->Print...").select();
        assertTrue(File_PrintDlg.exists(5));
        File_PrintDlg.cancel();
    }

    /**
     * Test the File -- Java Dialog  show
     *
     */
    @Test
    @Ignore
    public void testJavaDialog()
    {

        //Create a new text document and launch a Wizards dialog which need JVM work correctly.
        startcenter.menuItem("File->New->Text Document").select();
        File tempfile=new File(app.getUserInstallation(),"user/template/myAgendaTemplate.ott");
        FileUtil.deleteFile(tempfile);
        sleep(3);
        writer.menuItem("File->Wizards->Agenda").select();
        sleep(5);
        assertTrue(Wizards_AgendaDialog.exists(10));
        Wizards_AgendaDialog_FinishButton.click();
        sleep(10);
        writer.focus();
        sleep(1);
        writer.menuItem("Edit->Select All").select();
        typeKeys("<$copy>");
        //System.out.println("now txt:"+app.getClipboard());
        assertTrue(app.getClipboard().startsWith("<Name>"));
    }

    /**
     * Test the Tools / Macros / Organize Dialogs" show
     *
     */
    @Test
    public void testMacroToolsOrgDialog()
    {
        startcenter.menuItem("Tools->Macros->Organize Dialogs").select();
        assertTrue(MacroDialogsPage.exists(5));
        MacroDialogsPage.cancel();
    }


    /**
     * Test the About Dialog show
     *
     */
    @Test
    public void testAboutDialog()
    {
        if (SystemUtil.isMac()) {
            app.dispatch(".uno:About");
        } else {
            startcenter.menuItem("Help->About OpenOffice.org").select();
        }
        assertTrue(AboutDialog.exists(5));
        AboutDialog.ok();
    }

    /**
     * Test inserting a picture in text document
     * @throws Exception
     */


    @Test
    public void testInsertPictureInDocument() throws Exception {
        String bmp_green = testFile("pure_green_64x64.bmp");
        String bmp_red = testFile("pure_red_64x64.bmp");

        //Create a new text document
        startcenter.menuItem("File->New->Text Document").select();
        sleep(3);

        //Insert a picture fully filled with green
        writer.click(400, 400);
        writer.menuItem("Insert->Picture->From File...").select();
        submitOpenDlg(bmp_green);
        sleep(3);
        writer.click(0.5, 0.5);
        sleep(1);

        // Verify if the picture is inserted successfully
        Rectangle rectangle = GraphicsUtil.findRectangle(writer.getScreenRectangle(), 0xFF00FF00);

        assertNotNull("Green rectangle: " + rectangle, rectangle);

        //insert another picture
        writer.menuItem("Insert->Picture->From File...").select();
        submitOpenDlg(bmp_red);
        sleep(3);
        writer.click(0.5, 0.5);
        sleep(1);
        // Verify if the picture is inserted successfully
        rectangle = GraphicsUtil.findRectangle(writer.getScreenRectangle(), 0xFFFF0000);
        assertNotNull("Red rectangle: " + rectangle, rectangle);
    }


    @Test
    public void testInsertPictureInSpreadsheet() throws Exception {
        String bmp_green = testFile("pure_green_64x64.bmp");
        String bmp_red = testFile("pure_red_64x64.bmp");

        //Create a new text document
        startcenter.menuItem("File->New->Spreadsheet").select();
        sleep(3);

        //Insert a picture fully filled with green
        calc.menuItem("Insert->Picture->From File...").select();
        submitOpenDlg(bmp_green);
        sleep(3);
        calc.click(0.5, 0.5);
        sleep(1);

        // Verify if the picture is inserted successfully
        Rectangle rectangle = GraphicsUtil.findRectangle(calc.getScreenRectangle(), 0xFF00FF00);

        assertNotNull("Green rectangle: " + rectangle, rectangle);
//      assertEquals(new Rectangle(0,0,64,64), rectangle);
        CalcUtil.selectRange("C1");
        //insert another picture
        calc.menuItem("Insert->Picture->From File...").select();
        submitOpenDlg(bmp_red);
        sleep(3);
        calc.click(0.5, 0.5);
        sleep(1);
        // Verify if the picture is inserted successfully
        rectangle = GraphicsUtil.findRectangle(calc.getScreenRectangle(), 0xFFFF0000);
        assertNotNull("Red rectangle: " + rectangle, rectangle);
    }

    @Test
    public void testInsertPictureInPresentation() throws Exception {
        String bmp_green = testFile("pure_green_64x64.bmp");
        String bmp_red = testFile("pure_red_64x64.bmp");

        //Create a new text document
        startcenter.menuItem("File->New->Presentation").select();
        PresentationWizard.ok();
        sleep(3);

        //Insert a picture fully filled with green
        impress.menuItem("Insert->Picture->From File...").select();
        submitOpenDlg(bmp_green);
        sleep(3);
        impress.click(5,5);
        sleep(1);

        // Verify if the picture is inserted successfully
        Rectangle rectangle = GraphicsUtil.findRectangle(impress.getScreenRectangle(), 0xFF00FF00);

        assertNotNull("Green rectangle: " + rectangle, rectangle);
//      assertEquals(new Rectangle(0,0,64,64), rectangle);

        //insert another picture
        impress.menuItem("Insert->Picture->From File...").select();
        submitOpenDlg(bmp_red);
        sleep(3);
        impress.click(1, 1);
        sleep(1);
        // Verify if the picture is inserted successfully
        rectangle = GraphicsUtil.findRectangle(impress.getScreenRectangle(), 0xFFFF0000);
        assertNotNull("Red rectangle: " + rectangle, rectangle);
    }

    @Test
    public void testSlideShow() throws Exception {
        String file = testFile("slideshow.odp");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        impress.waitForExistence(10, 2);
        impress.menuItem("Slide Show->Slide Show").select();
        sleep(3);
        Rectangle rectangle = GraphicsUtil.findRectangle(SlideShow.getScreenRectangle(), 0xFFFF0000);
        assertNotNull("1st slide appears", rectangle);
        SlideShow.click(0.5, 0.5);
        sleep(2);
        rectangle = GraphicsUtil.findRectangle(SlideShow.getScreenRectangle(), 0xFF00FF00);
        assertNotNull("2nd slide appears", rectangle);
        typeKeys("<enter>");
        sleep(2);
        rectangle = GraphicsUtil.findRectangle(SlideShow.getScreenRectangle(), 0xFF0000FF);
        assertNotNull("3rd slide appears", rectangle);
        SlideShow.click(0.5, 0.5);
        sleep(2);
        rectangle = GraphicsUtil.findRectangle(SlideShow.getScreenRectangle(), 0xFF0000FF);
        assertNull("The end", rectangle);
        SlideShow.click(0.5, 0.5);
        sleep(3);
        assertFalse("Quit", SlideShow.exists());
    }

    @Test
    public void testFind() {
        String file = testFile("find.odt");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        writer.waitForExistence(10, 2);
        writer.menuItem("Edit->Find & Replace...").select();
        FindDlg_For.setText("OpenOffice");
        FindDlg_Find.click();
        sleep(1);
        writer.focus();
        typeKeys("<$copy>");
        assertEquals("OpenOffice", app.getClipboard());
        FindDlg_FindAll.click();
        sleep(1);
        writer.focus();
        typeKeys("<$copy>");
        assertEquals("OpenOfficeOpenOfficeOpenOffice", app.getClipboard());
        FindDlg_ReplaceWith.setText("Awesome OpenOffice");
        FindDlg_ReplaceAll.click();
        sleep(1);
        msgbox("Search key replaced 3 times.").ok();
        FindDlg.close();
        sleep(1);
        writer.menuItem("Edit->Select All").select();
        typeKeys("<$copy>");
        assertEquals("Apache Awesome OpenOffice is comprised of six personal productivity applications: a word processor (and its web-authoring component), spreadsheet, presentation graphics, drawing, equation editor, and database. Awesome OpenOffice is released on Windows, Solaris, Linux and Macintosh operation systems, with more communities joining, including a mature FreeBSD port. Awesome OpenOffice is localized, supporting over 110 languages worldwide. ", app.getClipboard());
    }

    @Test
    public void testFillInSpreadsheet() {
        String[][] expected1 = new String[][] {
                {"1"},
                {"1"},
                {"1"},
                {"1"},
                {"1"},
                {"1"},
        };
        String[][] expected2 = new String[][] {
                {"2"},
                {"2"},
                {"2"},
                {"2"},
                {"2"},
                {"2"},
        };

        String[][] expected3 = new String[][] {
                {"Hi friends","Hi friends","Hi friends", "Hi friends"}
        };

        String[][] expected4 = new String[][] {
                {"99999.999","99999.999","99999.999", "99999.999"}
        };
        String[][] expected5 = new String[][] {

        { "99999.999", "-10" },
        { "100000.999", "-9" },
        { "100001.999", "-8" },
                { "100002.999", "-7" },
            { "100003.999", "-6" }

        };
        //Create a new text document
        startcenter.menuItem("File->New->Spreadsheet").select();
        sleep(3);

        CalcUtil.selectRange("C5");
        typeKeys("1<enter>");
        CalcUtil.selectRange("C5:C10");
        calc.menuItem("Edit->Fill->Down").select();
        assertArrayEquals("Fill Down:", expected1, CalcUtil.getCellTexts("C5:C10"));

        CalcUtil.selectRange("D10");
        typeKeys("2<enter>");
        CalcUtil.selectRange("D5:D10");
        calc.menuItem("Edit->Fill->Up").select();
        assertArrayEquals("Fill Up:", expected2, CalcUtil.getCellTexts("D5:D10"));

        CalcUtil.selectRange("A1");
        typeKeys("Hi friends<enter>");
        CalcUtil.selectRange("A1:D1");
        calc.menuItem("Edit->Fill->Right").select();
        assertArrayEquals("Fill Right:", expected3, CalcUtil.getCellTexts("A1:D1"));

        CalcUtil.selectRange("D2");
        typeKeys("99999.999<enter>");
        CalcUtil.selectRange("A2:D2");
        calc.menuItem("Edit->Fill->Left").select();
        assertArrayEquals("Fill left:", expected4, CalcUtil.getCellTexts("A2:D2"));

        CalcUtil.selectRange("E1");
        typeKeys("99999.999<tab>-10<enter>");

        CalcUtil.selectRange("E1:F5");
        calc.menuItem("Edit->Fill->Series...").select();
        FillSeriesDlg.ok();
        sleep(1);
        assertArrayEquals("Fill series..", expected5, CalcUtil.getCellTexts("E1:F5"));
    }

    @Test
    public void testSort() {
        String[][] expected1 = new String[][] {
                { "-9999999" },
                { "-1.1" },
                { "-1.1" },
                { "0" },
                { "0" },
                { "0.1" },
                { "10" },
                { "12" },
                { "9999999" },
                { "9999999" },

        };
        String[][] expected2 = new String[][] {
                { "TRUE", "Oracle" },
                { "TRUE", "OpenOffice" },
                { "FALSE", "OpenOffice" },
                { "TRUE", "IBM" },
                { "FALSE", "IBM" },
                { "TRUE", "Google" },
                { "FALSE", "facebook " },
                { "TRUE", "Apache" },
                { "TRUE", "!yahoo" },
                { "TRUE", "" },

        };

        String[][] expected3 = new String[][] { { "Sunday" }, { "Monday" },
                { "Tuesday" }, { "Wednesday" }, { "Thursday" }, { "Friday" },
                { "Saturday" },

        };

        String[][] expected4 = new String[][] { { "-$10.00" }, { "$0.00" },
                { "$0.00" }, { "$1.00" }, { "$3.00" }, { "$9.00" },
                { "$123.00" }, { "$200.00" }, { "$400.00" }, { "$10,000.00" },

        };
        String file = testFile("sort.ods");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        calc.waitForExistence(10, 2);
        CalcUtil.selectRange("A1:A10");
        calc.menuItem("Data->Sort...").select();
        SortWarningDlg_Current.click();
        assertEquals("Column A", SortPage_By1.getSelText());
        SortPage.ok();
        sleep(1);
        assertArrayEquals("Sorted Data", expected1, CalcUtil.getCellTexts("A1:A10"));
        CalcUtil.selectRange("B1:C10");
        calc.menuItem("Data->Sort...").select();

        SortPage_By1.select("Column C");
        SortPage_Descending1.check();
        assertFalse(SortPage_By3.isEnabled());
        assertFalse(SortPage_Ascending3.isEnabled());
        assertFalse(SortPage_Descending3.isEnabled());
        SortPage_By2.select("Column B");
        assertTrue(SortPage_By3.isEnabled());
        assertTrue(SortPage_Ascending3.isEnabled());
        assertTrue(SortPage_Descending3.isEnabled());
        SortPage_Descending2.check();
        SortPage_By2.select("- undefined -");
        assertFalse(SortPage_By3.isEnabled());
        assertFalse(SortPage_Ascending3.isEnabled());
        assertFalse(SortPage_Descending3.isEnabled());
        SortPage_By2.select("Column B");
        SortPage.ok();
        sleep(1);

        assertArrayEquals("Sorted Data", expected2, CalcUtil.getCellTexts("B1:C10"));
        CalcUtil.selectRange("D1:D7");
        calc.menuItem("Data->Sort...").select();
        SortWarningDlg_Current.click();
        SortOptionsPage.select();
        SortOptionsPage_RangeContainsColumnLabels.uncheck();
        SortOptionsPage_CustomSortOrder.check();
        SortOptionsPage_CustomSortOrderList.select("Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Saturday");
        SortOptionsPage.ok();
        sleep(1);
        assertArrayEquals("Sorted Data", expected3, CalcUtil.getCellTexts("D1:D7"));

        CalcUtil.selectRange("E1:E10");
        calc.menuItem("Data->Sort...").select();
        SortWarningDlg_Current.click();
        SortPage.ok();
        sleep(1);
        assertArrayEquals("Sorted Data", expected4, CalcUtil.getCellTexts("E1:E10"));
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
