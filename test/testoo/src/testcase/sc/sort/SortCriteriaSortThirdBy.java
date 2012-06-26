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
public class SortCriteriaSortThirdBy {

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
     * Test sort criteria: sort third by
     * @throws Exception
     */
    @Test
    public void testSortCriteriaSortThirdBy() throws Exception{

        // Create a new spreadsheet document
        startcenter.menuItem("File->New->Spreadsheet").select();
        sleep(3);

        // Input some data
        String[][] data = new String[][] {
                {"3", "AB", "2"},
                {"8", "FLK", "5"},
                {"6", "E", "4"},
                {"1", "AB", "1"},
                {"9", "GE", "6"},
                {"2", "AB", "2"},
                {"7", "EFYU", "7"},
                {"5", "DS", "8"},
                {"4", "AB", "1"},
        };
        String[][] expectedResultSortFirstByB = new String[][] {
                {"3", "AB", "2"},
                {"1", "AB", "1"},
                {"2", "AB", "2"},
                {"4", "AB", "1"},
                {"5", "DS", "8"},
                {"6", "E", "4"},
                {"7", "EFYU", "7"},
                {"8", "FLK", "5"},
                {"9", "GE", "6"},
        };
        String[][] expectedResultSortSecondByC= new String[][] {
                {"3", "AB", "2"},
                {"2", "AB", "2"},
                {"1", "AB", "1"},
                {"4", "AB", "1"},
                {"5", "DS", "8"},
                {"6", "E", "4"},
                {"7", "EFYU", "7"},
                {"8", "FLK", "5"},
                {"9", "GE", "6"},
        };
        String[][] expectedResultSortThirdByA= new String[][] {
                {"3", "AB", "2"},
                {"2", "AB", "2"},
                {"4", "AB", "1"},
                {"1", "AB", "1"},
                {"5", "DS", "8"},
                {"6", "E", "4"},
                {"7", "EFYU", "7"},
                {"8", "FLK", "5"},
                {"9", "GE", "6"},
        };
        CalcUtil.selectRange("A1");
        typeKeys("3<down>8<down>6<down>1<down>9<down>2<down>7<down>5<down>4");
        CalcUtil.selectRange("B1");
        typeKeys("AB<down>FLK<down>E<down>AB<down>GE<down>AB<down>EFYU<down>DS<down>AB");
        CalcUtil.selectRange("C1");
        typeKeys("2<down>5<down>4<down>1<down>6<down>2<down>7<down>8<down>1");
        sleep(0.5);

        // "Data->Sort...", choose "Ascending", sort first by Column B
        calc.menuItem("Data->Sort...").select();
        SortPage_By1.select("Column B");
        SortPage_Ascending1.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result", expectedResultSortFirstByB, CalcUtil.getCellTexts("A1:C9"));

        // Uodo/redo
        calc.menuItem("Edit->Undo: Sort").select();
        sleep(1);
        assertArrayEquals("Undo sorted result", data, CalcUtil.getCellTexts("A1:C9"));
        calc.menuItem("Edit->Redo: Sort").select();
        sleep(1);
        assertArrayEquals("Redo sorted result", expectedResultSortFirstByB, CalcUtil.getCellTexts("A1:C9"));
        calc.menuItem("Edit->Undo: Sort").select();
        sleep(1);

        // Save and close document
        String saveTo = fullPath("temp/" + "SortCriteriaSortThirdBy.ods");
        calc.menuItem("File->Save As...").select();
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        calc.menuItem("File->Close").select();
        openStartcenter();

        // Reopen, "Data->Sort...", sort first by Column B "Ascending", sort second by Column C "Descending"
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        calc.menuItem("Data->Sort...").select();
        SortPage_By1.select("Column B");
        SortPage_Ascending1.check();
        SortPage_By2.select("Column C");
        SortPage_Descending2.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result", expectedResultSortSecondByC, CalcUtil.getCellTexts("A1:C9"));

        // "Data->Sort...", sort first by Column B "Ascending", sort second by Column C "Descending", sort third by Column A "Descending"
        calc.menuItem("Data->Sort...").select();
        SortPage_By1.select("Column B");
        SortPage_Ascending1.check();
        SortPage_By2.select("Column C");
        SortPage_Descending2.check();
        SortPage_By3.select("Column A");
        SortPage_Descending3.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result", expectedResultSortThirdByA, CalcUtil.getCellTexts("A1:C9"));

        // Uodo/redo
        calc.menuItem("Edit->Undo: Sort").select();
        sleep(1);
        assertArrayEquals("Undo sorted result", expectedResultSortSecondByC, CalcUtil.getCellTexts("A1:C9"));
        calc.menuItem("Edit->Redo: Sort").select();
        sleep(1);
        assertArrayEquals("Redo sorted result", expectedResultSortThirdByA, CalcUtil.getCellTexts("A1:C9"));

        // Save and close document
        saveTo = fullPath("temp/" + "SortCriteriaSortThirdBy1.ods");
        calc.menuItem("File->Save As...").select();
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        calc.menuItem("File->Close").select();
        openStartcenter();

        // Reopen and verify data sort is not lost
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        assertArrayEquals("Saved sorted result", expectedResultSortThirdByA, CalcUtil.getCellTexts("A1:C9"));
    }
}
