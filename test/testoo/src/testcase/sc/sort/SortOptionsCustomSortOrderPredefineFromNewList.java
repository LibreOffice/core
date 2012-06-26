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
package testcase.sc.sort;

import static testlib.AppUtil.*;
import static testlib.UIMap.*;

import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.sleep;
import static org.openoffice.test.vcl.Tester.typeKeys;

import org.junit.Before;
import org.junit.Ignore;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;

import testlib.CalcUtil;
import testlib.Log;

/**
 *
 */
public class SortOptionsCustomSortOrderPredefineFromNewList {

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
     * Test sort options: custom sort order, predefined in preferences from new list
     * @throws Exception
     */
    @Test
    public void testSortOptionsCustomSortOrderPredefineFromNewList() throws Exception{

        // Dependencies start
        // Create a new spreadsheet document
        startcenter.menuItem("File->New->Spreadsheet").select();
        sleep(3);

        // "Tools->Options...->OpenOffice.org Spreadsheets->Sort Lists"
        typeKeys("<alt T O>");
        OptionsDlgList.select(15);  // Select "OpenOffice.org Spreadsheets"
        typeKeys("<shift =>");  // Expand "OpenOffice.org Spreadsheets"
        OptionsDlgList.select(19);  // Select "Sort Lists"

        // Click "New" button, input "white,red,yellow,blue,green,black", press "Add" and "OK", close the document
        OptionsDlg_New.click();
        typeKeys("white,red,yellow,blue,green,black");
        OptionsDlg_Add.click();
        sleep(0.5);
        OptionsDlg.ok();
        calc.menuItem("File->Close").select();
        // Dependencies end

        // Create a new spreadsheet document
        startcenter.menuItem("File->New->Spreadsheet").select();
        sleep(3);

        // Input some data
        String[][] data = new String[][] {
                {"Color"},
                {"black"},
                {"yellow"},
                {"blue"},
                {"black"},
                {"white"},
                {"red"},
        };
        String[][] expectedResultNoCustomSortOrder = new String[][] {
                {"Color"},
                {"black"},
                {"black"},
                {"blue"},
                {"red"},
                {"white"},
                {"yellow"},
        };
        String[][] expectedResultCustomSortOrder = new String[][] {
                {"Color"},
                {"white"},
                {"red"},
                {"yellow"},
                {"blue"},
                {"black"},
                {"black"},
        };
        CalcUtil.selectRange("A1");
        typeKeys("Color<down>black<down>yellow<down>blue<down>black<down>white<down>red");
        CalcUtil.selectRange("A2:A7");
        calc.menuItem("Format->Change Case->Lowercase").select();   // In case SC capitalize first letter automatically

        // "Data->Sort...", "Options" tab, check "Range contains column labels", no custom sort order, "Ascending", sort first by Color
        CalcUtil.selectRange("A1:A7");
        calc.menuItem("Data->Sort...").select();
        SortOptionsPage.select();
        SortOptionsPage_RangeContainsColumnLabels.check();
        SortOptionsPage_CustomSortOrder.uncheck();
        SortPage.select();
        SortPage_By1.select("Color");
        SortPage_Ascending1.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result without custom sort order", expectedResultNoCustomSortOrder, CalcUtil.getCellTexts("A1:A7"));

        // Uodo/redo
        calc.menuItem("Edit->Undo: Sort").select();
        sleep(1);
        assertArrayEquals("Undo sorted result", data, CalcUtil.getCellTexts("A1:A7"));
        calc.menuItem("Edit->Redo: Sort").select();
        sleep(1);
        assertArrayEquals("Redo sorted result", expectedResultNoCustomSortOrder, CalcUtil.getCellTexts("A1:A7"));
        calc.menuItem("Edit->Undo: Sort").select();

        // Copy original data to sheet2
        CalcUtil.selectRange("A1:A7");
        calc.menuItem("Edit->Copy").select();
        CalcUtil.selectRange("Sheet2.A1");
        calc.menuItem("Edit->Paste").select();
        sleep(1);

        // "Data->Sort...", "Options" tab, check "Range contains column labels", choose custom sort order, "Ascending", sort first by Color
        calc.menuItem("Data->Sort...").select();
        SortOptionsPage.select();
        SortOptionsPage_RangeContainsColumnLabels.check();
        SortOptionsPage_CustomSortOrder.check();
        SortOptionsPage_CustomSortOrderList.select("white,red,yellow,blue,green,black");
        SortPage.select();
        SortPage_By1.select("Color");
        SortPage_Ascending1.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result with custom sort order", expectedResultCustomSortOrder, CalcUtil.getCellTexts("A1:A7"));

        // Uodo/redo
        calc.menuItem("Edit->Undo: Sort").select();
        sleep(1);
        assertArrayEquals("Undo sorted result", data, CalcUtil.getCellTexts("A1:A7"));
        calc.menuItem("Edit->Redo: Sort").select();
        sleep(1);
        assertArrayEquals("Redo sorted result", expectedResultCustomSortOrder, CalcUtil.getCellTexts("A1:A7"));

        // Save and close document
        String saveTo = fullPath("temp/" + "SortOptionsCustomSortOrderPredefineFromNewList.ods");
        calc.menuItem("File->Save As...").select();
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        calc.menuItem("File->Close").select();
        openStartcenter();

        // Reopen and verify sorted result
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        assertArrayEquals("Original data", data, CalcUtil.getCellTexts("$Sheet1.$A1:$A7"));
        assertArrayEquals("Saved sorted result", expectedResultCustomSortOrder, CalcUtil.getCellTexts("$Sheet2.$A1:$A7"));
    }
}
