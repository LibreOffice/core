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
public class SortCriteriaSortSecondBy {

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
     * Test sort criteria: sort second by
     * @throws Exception
     */
    @Test
    public void testSortCriteriaSortSecondBy() throws Exception{

        // Create a new spreadsheet document
        startcenter.menuItem("File->New->Spreadsheet").select();
        sleep(3);

        // Input some data
        String[][] data = new String[][] {
                {"3", "D"},
                {"5", "FLK"},
                {"4", "E"},
                {"1", "AB"},
                {"6", "GE"},
                {"2", "AB"},
        };
        String[][] expectedResultSortFirstByB = new String[][] {
                {"1", "AB"},
                {"2", "AB"},
                {"3", "D"},
                {"4", "E"},
                {"5", "FLK"},
                {"6", "GE"},
        };
        String[][] expectedResultSortSecondByA= new String[][] {
                {"2", "AB"},
                {"1", "AB"},
                {"3", "D"},
                {"4", "E"},
                {"5", "FLK"},
                {"6", "GE"},
        };
        CalcUtil.selectRange("A1");
        typeKeys("3<down>5<down>4<down>1<down>6<down>2");
        CalcUtil.selectRange("B1");
        typeKeys("D<down>FLK<down>E<down>AB<down>GE<down>AB");

        // "Data->Sort...", choose "Ascending", sort first by Column B
        calc.menuItem("Data->Sort...").select();
        SortPage_Ascending1.check();
        SortPage_By1.select("Column B");
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result", expectedResultSortFirstByB, CalcUtil.getCellTexts("A1:B6"));

        // Uodo/redo
        calc.menuItem("Edit->Undo: Sort").select();
        sleep(1);
        assertArrayEquals("Undo sorted result", data, CalcUtil.getCellTexts("A1:B6"));
        calc.menuItem("Edit->Redo: Sort").select();
        sleep(1);
        assertArrayEquals("Redo sorted result", expectedResultSortFirstByB, CalcUtil.getCellTexts("A1:B6"));
        calc.menuItem("Edit->Undo: Sort").select();

        // Save and close document
        String saveTo = fullPath("temp/" + "SortCriteriaSortSecondBy.ods");
        calc.menuItem("File->Save As...").select();
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        calc.menuItem("File->Close").select();
        openStartcenter();

        // Reopen, "Data->Sort...", sort first by Column B "Ascending", sort second by Column A "Descending"
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        calc.menuItem("Data->Sort...").select();
        SortPage_By1.select("Column B");
        SortPage_Ascending1.check();
        SortPage_By2.select("Column A");
        SortPage_Descending2.check();
        SortPage.ok();

        // Verify sorted result
        assertArrayEquals("Saved sorted result", expectedResultSortSecondByA, CalcUtil.getCellTexts("A1:B6"));
    }
}
