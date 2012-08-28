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
package testcase.gui;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.openoffice.test.common.Testspace.getPath;
import static org.openoffice.test.common.Testspace.prepareData;
import static org.openoffice.test.vcl.Tester.sleep;
import static testlib.gui.AppUtil.submitOpenDlg;
import static testlib.gui.AppUtil.submitSaveDlg;
import static testlib.gui.AppUtil.typeKeys;
import static testlib.gui.UIMap.AboutDialog;
import static testlib.gui.UIMap.Chart_Wizard;
import static testlib.gui.UIMap.FileSave;
import static testlib.gui.UIMap.FileSave_FileType;
import static testlib.gui.UIMap.File_PrintDlg;
import static testlib.gui.UIMap.FillSeriesDlg;
import static testlib.gui.UIMap.FindDlg;
import static testlib.gui.UIMap.FindDlg_Find;
import static testlib.gui.UIMap.FindDlg_FindAll;
import static testlib.gui.UIMap.FindDlg_For;
import static testlib.gui.UIMap.FindDlg_ReplaceAll;
import static testlib.gui.UIMap.FindDlg_ReplaceWith;
import static testlib.gui.UIMap.InsertTable;
import static testlib.gui.UIMap.MacroDialogsPage;
import static testlib.gui.UIMap.PDFGeneralPage;
import static testlib.gui.UIMap.PresentationWizard;
import static testlib.gui.UIMap.SC_FunctionWizardDlg;
import static testlib.gui.UIMap.SC_FunctionWizardDlg_Edit1;
import static testlib.gui.UIMap.SC_FunctionWizardDlg_FunctionList;
import static testlib.gui.UIMap.SC_FunctionWizardDlg_Next;
import static testlib.gui.UIMap.SC_InputBar_Input;
import static testlib.gui.UIMap.SC_InputBar_Sum;
import static testlib.gui.UIMap.SlideShow;
import static testlib.gui.UIMap.SortOptionsPage;
import static testlib.gui.UIMap.SortOptionsPage_CustomSortOrder;
import static testlib.gui.UIMap.SortOptionsPage_CustomSortOrderList;
import static testlib.gui.UIMap.SortOptionsPage_RangeContainsColumnLabels;
import static testlib.gui.UIMap.SortPage;
import static testlib.gui.UIMap.SortPage_Ascending3;
import static testlib.gui.UIMap.SortPage_By1;
import static testlib.gui.UIMap.SortPage_By2;
import static testlib.gui.UIMap.SortPage_By3;
import static testlib.gui.UIMap.SortPage_Descending1;
import static testlib.gui.UIMap.SortPage_Descending2;
import static testlib.gui.UIMap.SortPage_Descending3;
import static testlib.gui.UIMap.SortWarningDlg_Current;
import static testlib.gui.UIMap.Table_Toolbar;
import static testlib.gui.UIMap.Wizards_AgendaDialog;
import static testlib.gui.UIMap.Wizards_AgendaDialog_FinishButton;
import static testlib.gui.UIMap.app;
import static testlib.gui.UIMap.button;
import static testlib.gui.UIMap.calc;
import static testlib.gui.UIMap.chart;
import static testlib.gui.UIMap.draw;
import static testlib.gui.UIMap.impress;
import static testlib.gui.UIMap.msgbox;
import static testlib.gui.UIMap.oo;
import static testlib.gui.UIMap.toolbox;
import static testlib.gui.UIMap.writer;
import static testlib.gui.UIMap.writer_InsertTable;

import java.awt.Rectangle;
import java.io.File;

import org.junit.AfterClass;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.GraphicsUtil;

import testlib.gui.CalcUtil;
import testlib.gui.Log;

/**
 *
 */
public class BVTFunction {

    @Rule
    public Log LOG = new Log();

    @Before
    public void setUp() throws Exception {
        app.start(true);
    }

    @AfterClass
    public static void afterClass() throws Exception {
        app.close();
    }

    @Test
    public void testExportAsPDF() throws Exception {
        String file = prepareData("export_pdf.odt");
        String exportTo = getPath("temp/odt.pdf");

        app.dispatch(".uno:Open");
        submitOpenDlg(file);
        writer.waitForExistence(10, 2);
        app.dispatch(".uno:ExportToPDF");
        PDFGeneralPage.ok();
        FileUtil.deleteFile(exportTo);
        submitSaveDlg(exportTo);
        assertTrue("PDF is exported?", new File(exportTo).exists());
        assertTrue(toolbox(".HelpId:standardbar").exists(5));
        button(".uno:ExportDirectToPDF").click();
        assertEquals("PDF - Portable Document Format (.pdf)",
                FileSave_FileType.getSelText());
        FileSave.cancel();
    }

    /**
     * Test the File -- Print Dialog show
     *
     */
    @Test
    public void testPrintDialog() {
        // Create a new text document
        app.dispatch("private:factory/swriter");
        writer.waitForExistence(10, 2);
        app.dispatch(".uno:Print");
        assertTrue(File_PrintDlg.exists(5));
        File_PrintDlg.cancel();
    }

    /**
     * Test the File -- Java Dialog show
     *
     */
    @Test
    public void testJavaDialog() {

        // Create a new text document and launch a Wizards dialog which need JVM
        // work correctly.
        app.dispatch("private:factory/swriter");
        File tempfile = new File(oo.getUserInstallation(),
                "user/template/myAgendaTemplate.ott");
        FileUtil.deleteFile(tempfile);
        sleep(3);
        app.dispatch("service:com.sun.star.wizards.agenda.CallWizard?start");
        sleep(5);
        assertTrue(Wizards_AgendaDialog.exists(10));
        Wizards_AgendaDialog_FinishButton.click();
        sleep(10);
        writer.focus();
        sleep(1);
        app.dispatch(".uno:SelectAll");
        typeKeys("<$copy>");
        // System.out.println("now txt:"+app.getClipboard());
        // assertTrue(app.getClipboard().startsWith("<Name>"));
        assertNotNull(app.getClipboard());
    }

    /**
     * Test the Tools / Macros / Organize Dialogs" show
     *
     */
    @Test
    public void testMacroToolsOrgDialog() {
        app.dispatch(".uno:MacroOrganizer?TabId:short=1");
        assertTrue(MacroDialogsPage.exists(5));
        MacroDialogsPage.cancel();
    }

    /**
     * Test the About Dialog show
     *
     */
    @Test
    public void testAboutDialog() {
        app.dispatch(".uno:About");
        assertTrue(AboutDialog.exists(5));
        AboutDialog.ok();
    }

    /**
     * Test inserting a picture in text document
     *
     * @throws Exception
     */

    @Test
    public void testInsertPictureInDocument() throws Exception {
        String bmp_green = prepareData("pure_green_64x64.bmp");
        String bmp_red = prepareData("pure_red_64x64.bmp");

        // Create a new text document
        app.dispatch("private:factory/swriter");
        writer.waitForExistence(10, 2);

        // Insert a picture fully filled with green
        writer.click(400, 400);
        app.dispatch(".uno:InsertGraphic");
        submitOpenDlg(bmp_green);
        sleep(3);
        writer.click(0.5, 0.5);
        sleep(1);

        // Verify if the picture is inserted successfully
        Rectangle rectangle = GraphicsUtil.findRectangle(
                writer.getScreenRectangle(), 0xFF00FF00);

        assertNotNull("Green rectangle: " + rectangle, rectangle);

        // insert another picture
        app.dispatch(".uno:InsertGraphic");
        submitOpenDlg(bmp_red);
        sleep(3);
        writer.click(0.5, 0.5);
        sleep(1);
        // Verify if the picture is inserted successfully
        rectangle = GraphicsUtil.findRectangle(writer.getScreenRectangle(),
                0xFFFF0000);
        assertNotNull("Red rectangle: " + rectangle, rectangle);
    }

    @Test
    public void testInsertPictureInSpreadsheet() throws Exception {
        String bmp_green = prepareData("pure_green_64x64.bmp");
        String bmp_red = prepareData("pure_red_64x64.bmp");

        // Create a new text document
        app.dispatch("private:factory/scalc");
        calc.waitForExistence(10, 2);

        // Insert a picture fully filled with green
        app.dispatch(".uno:InsertGraphic");
        submitOpenDlg(bmp_green);
        sleep(3);
        calc.click(0.5, 0.5);
        sleep(1);

        // Verify if the picture is inserted successfully
        Rectangle rectangle = GraphicsUtil.findRectangle(
                calc.getScreenRectangle(), 0xFF00FF00);

        assertNotNull("Green rectangle: " + rectangle, rectangle);
        // assertEquals(new Rectangle(0,0,64,64), rectangle);
        CalcUtil.selectRange("C1");
        // insert another picture
        app.dispatch(".uno:InsertGraphic");
        submitOpenDlg(bmp_red);
        sleep(3);
        calc.click(0.5, 0.5);
        sleep(1);
        // Verify if the picture is inserted successfully
        rectangle = GraphicsUtil.findRectangle(calc.getScreenRectangle(),
                0xFFFF0000);
        assertNotNull("Red rectangle: " + rectangle, rectangle);
    }

    @Test
    public void testInsertPictureInPresentation() throws Exception {
        String bmp_green = prepareData("pure_green_64x64.bmp");
        String bmp_red = prepareData("pure_red_64x64.bmp");

        // Create a new text document
        app.dispatch("private:factory/simpress?slot=6686");
        PresentationWizard.ok();
        impress.waitForExistence(10, 2);

        // Insert a picture fully filled with green
        app.dispatch(".uno:InsertGraphic");
        submitOpenDlg(bmp_green);
        sleep(3);
        impress.click(5, 5);
        sleep(1);

        // Verify if the picture is inserted successfully
        Rectangle rectangle = GraphicsUtil.findRectangle(
                impress.getScreenRectangle(), 0xFF00FF00);

        assertNotNull("Green rectangle: " + rectangle, rectangle);
        // assertEquals(new Rectangle(0,0,64,64), rectangle);

        // insert another picture
        app.dispatch(".uno:InsertGraphic");
        submitOpenDlg(bmp_red);
        sleep(3);
        impress.click(1, 1);
        sleep(1);
        // Verify if the picture is inserted successfully
        rectangle = GraphicsUtil.findRectangle(impress.getScreenRectangle(),
                0xFFFF0000);
        assertNotNull("Red rectangle: " + rectangle, rectangle);
    }

    @Test
    public void testSlideShow() throws Exception {
        String file = prepareData("slideshow.odp");
        app.dispatch(".uno:Open");
        submitOpenDlg(file);
        impress.waitForExistence(10, 2);
        app.dispatch(".uno:Presentation");
        sleep(3);
        Rectangle rectangle = GraphicsUtil.findRectangle(
                SlideShow.getScreenRectangle(), 0xFFFF0000);
        assertNotNull("1st slide appears", rectangle);
        SlideShow.click(0.5, 0.5);
        sleep(2);
        rectangle = GraphicsUtil.findRectangle(SlideShow.getScreenRectangle(),
                0xFF00FF00);
        assertNotNull("2nd slide appears", rectangle);
        typeKeys("<enter>");
        sleep(2);
        rectangle = GraphicsUtil.findRectangle(SlideShow.getScreenRectangle(),
                0xFF0000FF);
        assertNotNull("3rd slide appears", rectangle);
        SlideShow.click(0.5, 0.5);
        sleep(2);
        rectangle = GraphicsUtil.findRectangle(SlideShow.getScreenRectangle(),
                0xFF0000FF);
        assertNull("The end", rectangle);
        SlideShow.click(0.5, 0.5);
        sleep(3);
        assertFalse("Quit", SlideShow.exists());
    }

    @Test
    public void testFind() {
        String file = prepareData("find.odt");
        app.dispatch(".uno:Open");
        submitOpenDlg(file);
        writer.waitForExistence(10, 2);
        app.dispatch(".uno:SearchDialog");
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
        app.dispatch(".uno:SelectAll");
        typeKeys("<$copy>");
        assertEquals(
                "Apache Awesome OpenOffice is comprised of six personal productivity applications: a word processor (and its web-authoring component), spreadsheet, presentation graphics, drawing, equation editor, and database. Awesome OpenOffice is released on Windows, Solaris, Linux and Macintosh operation systems, with more communities joining, including a mature FreeBSD port. Awesome OpenOffice is localized, supporting over 110 languages worldwide. ",
                app.getClipboard());
    }

    @Test
    public void testFillInSpreadsheet() {
        String[][] expected1 = new String[][] { { "1" }, { "1" }, { "1" },
                { "1" }, { "1" }, { "1" }, };
        String[][] expected2 = new String[][] { { "2" }, { "2" }, { "2" },
                { "2" }, { "2" }, { "2" }, };

        String[][] expected3 = new String[][] { { "Hi friends", "Hi friends",
                "Hi friends", "Hi friends" } };

        String[][] expected4 = new String[][] { { "99999.999", "99999.999",
                "99999.999", "99999.999" } };
        String[][] expected5 = new String[][] {

        { "99999.999", "-10" }, { "100000.999", "-9" }, { "100001.999", "-8" },
                { "100002.999", "-7" }, { "100003.999", "-6" }

        };
        // Create a new text document
        app.dispatch("private:factory/scalc");
        calc.waitForExistence(10, 2);

        CalcUtil.selectRange("C5");
        typeKeys("1<enter>");
        CalcUtil.selectRange("C5:C10");
        app.dispatch(".uno:FillDown");
        assertArrayEquals("Fill Down:", expected1,
                CalcUtil.getCellTexts("C5:C10"));

        CalcUtil.selectRange("D10");
        typeKeys("2<enter>");
        CalcUtil.selectRange("D5:D10");
        app.dispatch(".uno:FillUp");
        assertArrayEquals("Fill Up:", expected2,
                CalcUtil.getCellTexts("D5:D10"));

        CalcUtil.selectRange("A1");
        typeKeys("Hi friends<enter>");
        CalcUtil.selectRange("A1:D1");
        app.dispatch(".uno:FillRight");
        assertArrayEquals("Fill Right:", expected3,
                CalcUtil.getCellTexts("A1:D1"));

        CalcUtil.selectRange("D2");
        typeKeys("99999.999<enter>");
        CalcUtil.selectRange("A2:D2");
        app.dispatch(".uno:FillLeft");
        assertArrayEquals("Fill left:", expected4,
                CalcUtil.getCellTexts("A2:D2"));

        CalcUtil.selectRange("E1");
        typeKeys("99999.999<tab>-10<enter>");

        CalcUtil.selectRange("E1:F5");
        app.dispatch(".uno:FillSeries");
        FillSeriesDlg.ok();
        sleep(1);
        assertArrayEquals("Fill series..", expected5,
                CalcUtil.getCellTexts("E1:F5"));
    }

    @Test
    public void testSort() {
        String[][] expected1 = new String[][] { { "-9999999" }, { "-1.1" },
                { "-1.1" }, { "0" }, { "0" }, { "0.1" }, { "10" }, { "12" },
                { "9999999" }, { "9999999" },

        };
        String[][] expected2 = new String[][] { { "TRUE", "Oracle" },
                { "TRUE", "OpenOffice" }, { "FALSE", "OpenOffice" },
                { "TRUE", "IBM" }, { "FALSE", "IBM" }, { "TRUE", "Google" },
                { "FALSE", "facebook " }, { "TRUE", "Apache" },
                { "TRUE", "!yahoo" }, { "TRUE", "" },

        };

        String[][] expected3 = new String[][] { { "Sunday" }, { "Monday" },
                { "Tuesday" }, { "Wednesday" }, { "Thursday" }, { "Friday" },
                { "Saturday" },

        };

        String[][] expected4 = new String[][] { { "-$10.00" }, { "$0.00" },
                { "$0.00" }, { "$1.00" }, { "$3.00" }, { "$9.00" },
                { "$123.00" }, { "$200.00" }, { "$400.00" }, { "$10,000.00" },

        };
        String file = prepareData("sort.ods");
        app.dispatch(".uno:Open");
        submitOpenDlg(file);
        calc.waitForExistence(10, 2);
        CalcUtil.selectRange("A1:A10");
        app.dispatch(".uno:DataSort");
        SortWarningDlg_Current.click();
        assertEquals(1, SortPage_By1.getSelIndex());
        SortPage.ok();
        sleep(1);
        assertArrayEquals("Sorted Data", expected1,
                CalcUtil.getCellTexts("A1:A10"));
        CalcUtil.selectRange("B1:C10");
        app.dispatch(".uno:DataSort");

        SortPage_By1.select(2);
        SortPage_Descending1.check();
        assertFalse(SortPage_By3.isEnabled());
        assertFalse(SortPage_Ascending3.isEnabled());
        assertFalse(SortPage_Descending3.isEnabled());
        SortPage_By2.select(1);
        assertTrue(SortPage_By3.isEnabled());
        assertTrue(SortPage_Ascending3.isEnabled());
        assertTrue(SortPage_Descending3.isEnabled());
        SortPage_Descending2.check();
        SortPage_By2.select(0);
        assertFalse(SortPage_By3.isEnabled());
        assertFalse(SortPage_Ascending3.isEnabled());
        assertFalse(SortPage_Descending3.isEnabled());
        SortPage_By2.select(1);
        SortPage.ok();
        sleep(1);

        assertArrayEquals("Sorted Data", expected2,
                CalcUtil.getCellTexts("B1:C10"));
        CalcUtil.selectRange("D1:D7");
        app.dispatch(".uno:DataSort");
        SortWarningDlg_Current.click();
        SortOptionsPage.select();
        SortOptionsPage_RangeContainsColumnLabels.uncheck();
        SortOptionsPage_CustomSortOrder.check();
        SortOptionsPage_CustomSortOrderList
                .select("Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Saturday");
        SortOptionsPage.ok();
        sleep(1);
        assertArrayEquals("Sorted Data", expected3,
                CalcUtil.getCellTexts("D1:D7"));

        CalcUtil.selectRange("E1:E10");
        app.dispatch(".uno:DataSort");
        SortWarningDlg_Current.click();
        SortPage.ok();
        sleep(1);
        assertArrayEquals("Sorted Data", expected4,
                CalcUtil.getCellTexts("E1:E10"));
    }

    /**
     * Test insert a chart in a draw document 1. New a draw document 2. Insert a
     * chart 3. Check if the chart is inserted successfully
     *
     * @throws Exception
     */
    @Test
    public void testInsertChartInDraw() throws Exception {

        // Create a new drawing document
        app.dispatch("private:factory/sdraw");
        draw.waitForExistence(10, 2);

        // Insert a chart
        app.dispatch(".uno:InsertObjectChart");
        sleep(3);

        // Verify if the chart is inserted successfully
        assertTrue(chart.exists(3));
        // Focus on edit pane
        draw.click(5, 5);
        sleep(1);
    }

    /**
     * Test insert a chart in a text document 1. New a text document 2. Insert a
     * chart 3. Check if the chart is inserted successfully
     *
     * @throws Exception
     */
    @Test
    public void testInsertChartInDocument() throws Exception {

        // Create a new text document
        app.dispatch("private:factory/swriter");
        writer.waitForExistence(10, 2);

        // Insert a chart
        app.dispatch(".uno:InsertObjectChart");
        sleep(3);

        // Verify if the chart is inserted successfully
        assertTrue(chart.exists(3));
        // Focus on edit pane
        writer.click(5, 5);
        sleep(1);
    }

    /**
     * Test insert a chart in a spreadsheet document 1. New a spreadsheet
     * document 2. Insert a chart 3. Check if the chart is inserted successfully
     *
     * @throws Exception
     */
    @Test
    public void testInsertChartInSpreadsheet() throws Exception {

        // Create a new spreadsheet document
        app.dispatch("private:factory/scalc");
        calc.waitForExistence(10, 2);

        // Insert a chart
        app.dispatch(".uno:InsertObjectChart");
        sleep(3);
        Chart_Wizard.ok();

        // Verify if the chart is inserted successfully
        assertTrue(chart.exists(3));
        // Focus on edit pane
        calc.click(5, 5);
        sleep(1);
    }

    /**
     * Test insert a chart in a presentation document 1. New a presentation
     * document 2. Insert a chart 3. Check if the chart is inserted successfully
     *
     * @throws Exception
     */
    @Test
    public void testInsertChartInPresentation() throws Exception {

        // Create a new presentation document
        app.dispatch("private:factory/simpress?slot=6686");
        PresentationWizard.ok();
        impress.waitForExistence(10, 2);

        // Insert a chart
        app.dispatch(".uno:InsertObjectChart");
        sleep(3);

        // Verify if the chart is inserted successfully
        assertTrue(chart.exists(3));
        // Focus on edit pane
        impress.click(5, 5);
        sleep(1);
    }

    /**
     * Test insert a table in a draw document 1. New a draw document 2. Insert a
     * default table 3. Check if the table is inserted successfully
     *
     * @throws Exception
     */
    @Test
    public void testInsertTableInDraw() throws Exception {

        // Create a new drawing document
        app.dispatch("private:factory/sdraw");
        draw.waitForExistence(10, 2);

        // Insert a table
        app.dispatch(".uno:InsertTable");
        InsertTable.ok();
        sleep(3);

        // Verify if the table toolbar is active
        typeKeys("123");    // Insert text to focus on table, to avoid Bug 120171 on linux
        assertTrue(Table_Toolbar.exists(3));

        // Focus on edit pane
        draw.click(5, 5);
        sleep(1);
    }

    /**
     * Test insert a table in a text document 1. New a text document 2. Insert a
     * default table 3. Check if the table is inserted successfully
     *
     * @throws Exception
     */
    @Test
    public void testInsertTableInDocument() throws Exception {

        // Create a new text document
        app.dispatch("private:factory/swriter");
        writer.waitForExistence(10, 2);

        // Insert a table
        app.dispatch(".uno:InsertTable");
        writer_InsertTable.ok();
        sleep(3);

        // Verify if the table toolbar is active
        assertTrue(Table_Toolbar.exists(3));

        // // Check the statusbar to verify if the table is inserted
        // successfully
        // assertEquals("Table1:A1", StatusBar.getItemText(7));
        // Focus on edit pane
        writer.click(5, 5);
        sleep(1);
    }

    /**
     * Test insert a table in a presentation document 1. New a presentation
     * document 2. Insert a default table 3. Check if the table is inserted
     * successfully
     *
     * @throws Exception
     */
    @Test
    public void testInsertTableInPresentation() throws Exception {

        // Create a new presentation document
        app.dispatch("private:factory/simpress?slot=6686");
        PresentationWizard.ok();
        impress.waitForExistence(10, 2);

        // Insert a table
        app.dispatch(".uno:InsertTable");
        InsertTable.ok();
        sleep(3);

        // Verify if the table toolbar is active
        typeKeys("123");    // Insert text to focus on table, to avoid Bug 120171 on linux
        assertTrue(Table_Toolbar.exists(3));

        // // Check the statusbar to verify if the table is inserted
        // successfully
        // assertEquals("Table selected", StatusBar.getItemText(0));
        // Focus on edit pane
        impress.click(5, 5);
        sleep(1);
    }

    /**
     * Test insert a function in a spreadsheet document via Sum button 1. New a
     * spreadsheet document 2. Insert a function via Sum button 3. Check if the
     * result is correct
     *
     * @throws Exception
     */
    @Test
    public void testInsertFunctionInSCViaSumButton() throws Exception {

        // Create a new spreadsheet document
        app.dispatch("private:factory/scalc");
        calc.waitForExistence(10, 2);

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
        assertEquals("The calculated result", expectedResult,
                CalcUtil.getCellText("C1"));
    }

    /**
     * Test insert a function in a spreadsheet document via inputbar 1. New a
     * spreadsheet document 2. Insert a function via inputbar: COS 3. Check if
     * the result is correct
     *
     * @throws Exception
     */
    @Test
    public void testInsertFunctionInSCViaInputbar() throws Exception {

        // Create a new spreadsheet document
        app.dispatch("private:factory/scalc");
        calc.waitForExistence(10, 2);

        // Insert source numbers and expected result
        String sourceData = "0";
        String expectedResult = "1";
        CalcUtil.selectRange("A1");
        typeKeys(sourceData);

        // Insert a function via inputbar: COS
        CalcUtil.selectRange("D1");
        SC_InputBar_Input.inputKeys("=COS(A1)");
        typeKeys("<enter>");

        // Verify if the calculated result is equal to the expected result
        assertEquals("The calculated result", expectedResult,
                CalcUtil.getCellText("D1"));
    }

    /**
     * Test insert a function in a spreadsheet document via Function Wizard
     * Dialog 1. New a spreadsheet document 2. Insert a function via Function
     * Wizard Dialog: ABS 3. Check if the result is correct
     *
     * @throws Exception
     */
    @Test
    public void testInsertFunctionInSCViaFunctionWizard() throws Exception {

        // Create a new spreadsheet document
        app.dispatch("private:factory/scalc");
        calc.waitForExistence(10, 2);

        // Insert source number
        String sourceNumber = "-5";
        String expectedResult = "5";
        CalcUtil.selectRange("A1");
        typeKeys(sourceNumber);
        typeKeys("<enter>");

        // Insert a function via Function Wizard Dialog: ABS
        CalcUtil.selectRange("B1");
        app.dispatch(".uno:FunctionDialog");
//      SC_FunctionWizardDlg_FunctionList.doubleClick(5, 5);
        SC_FunctionWizardDlg_FunctionList.select("ABS");
        SC_FunctionWizardDlg_Next.click();  // Use "Next" button

        SC_FunctionWizardDlg_Edit1.inputKeys("A1");
        SC_FunctionWizardDlg.ok();

        // Verify if the calculated result is equal to the expected result
        assertEquals("The calculated result", expectedResult,
                CalcUtil.getCellText("B1"));
    }
}
