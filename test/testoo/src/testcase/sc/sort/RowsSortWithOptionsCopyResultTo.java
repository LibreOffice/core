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

/**
 *
 */
public class RowsSortWithOptionsCopyResultTo {

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
     * Test rows sort with options setting: copy sort result to
     * @throws Exception
     */
    @Ignore("Redo is not work when copy sort result to other sheet")
    public void testRowsSortWithOptionsCopyResultTo() throws Exception{

        // Create a new spreadsheet document
        startcenter.menuItem("File->New->Spreadsheet").select();
        sleep(3);

        // Input some data
        String[][] data = new String[][] {
                {"3", "D"},
                {"5", "FLK"},
                {"4", "E"},
                {"2", "BC"},
                {"6", "GE"},
                {"1", "AB"},
        };
        String[][] expectedSortedResult = new String[][] {
                {"1", "AB"},
                {"2", "BC"},
                {"3", "D"},
                {"4", "E"},
                {"5", "FLK"},
                {"6", "GE"},
        };
        CalcUtil.selectRange("A1");
        typeKeys("3<down>5<down>4<down>2<down>6<down>1");
        CalcUtil.selectRange("B1");
        typeKeys("D<down>FLK<down>E<down>BC<down>GE<down>AB");

        // Data->Sort..., choose "Ascending", check "Copy sort results to:"
        calc.menuItem("Data->Sort...").select();
        SortOptionsPage.select();
        SortOptionsPage_CopyResultTo.check();
        SortOptionsPage_CopyResultToEdit.setText("$Sheet3.$A4");
        SortOptionsPage.ok();

        // Verify sorted result
        assertArrayEquals("Copy sorted result to", expectedSortedResult, CalcUtil.getCellTexts("$Sheet3.$A4:$B9"));

        // Uodo/redo
        calc.menuItem("Edit->Undo: Sort").select();
        sleep(1);
        assertEquals("Undo sorted result", "", CalcUtil.getCellText("$Sheet3.$A4"));
        calc.menuItem("Edit->Redo: Sort").select();
        sleep(1);
        assertArrayEquals("Redo sorted result", expectedSortedResult, CalcUtil.getCellTexts("$Sheet3.$A4:$B9"));

        // Move focus to sheet2 then select a cell range, Insert->Names->Define...
        CalcUtil.selectRange("$Sheet2.$A1:$B3");
        calc.menuItem("Insert->Names->Define...").select();
        sleep(1);
        DefineNamesDlg_NameEdit.setText("cellRange");
        DefineNamesDlg.ok();

        // Set focus to the original data, Data->Sort...
        CalcUtil.selectRange("$Sheet1.$B1");
        calc.menuItem("Data->Sort...").select();
        SortOptionsPage.select();
        SortOptionsPage_CopyResultTo.check();
        SortOptionsPage_CopyResultToCellRange.select("cellRange");
        SortOptionsPage.ok();

        // Verify sorted result
        assertArrayEquals("Copy sorted result to cell range", expectedSortedResult, CalcUtil.getCellTexts("$Sheet2.$A1:$B6"));

        // Uodo/redo
        calc.menuItem("Edit->Undo: Sort").select();
        sleep(1);
        assertEquals("Undo sorted result", "", CalcUtil.getCellText("$Sheet2.$A1"));
        calc.menuItem("Edit->Redo: Sort").select();
        sleep(1);
        assertArrayEquals("Redo sorted result", expectedSortedResult, CalcUtil.getCellTexts("$Sheet2.$A1:$B6"));

        // Save and close document
        String saveTo = fullPath("temp/" + "RowsSortWithOptionsCopyResultTo.ods");
        calc.menuItem("File->Save As...").select();
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        calc.menuItem("File->Close").select();
        openStartcenter();

        // Reopen and verify sorted result
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        assertArrayEquals("Saved sorted result", expectedSortedResult, CalcUtil.getCellTexts("$Sheet3.$A4:$B9"));
        assertArrayEquals("Saved sorted result to cell range", expectedSortedResult, CalcUtil.getCellTexts("$Sheet2.$A1:$B6"));
    }
}
