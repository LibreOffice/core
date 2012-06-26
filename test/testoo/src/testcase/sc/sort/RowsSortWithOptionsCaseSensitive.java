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
public class RowsSortWithOptionsCaseSensitive {

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
     * Test rows sort with options setting: case sensitive
     * @throws Exception
     */
    @Test
    public void testRowsSortWithOptionsCaseSensitive() throws Exception{

        // Create a new spreadsheet document
        startcenter.menuItem("File->New->Spreadsheet").select();
        sleep(3);

        // Input some data: A1~A6: 1ColumnName,D,C,B,A,a
        String[][] data = new String[][] {
                {"D"},
                {"C"},
                {"B"},
                {"A"},
                {"a"},
        };
        String[][] expectedSortedResult = new String[][] {
                {"a"},
                {"A"},
                {"B"},
                {"C"},
                {"D"},
        };
        CalcUtil.selectRange("A1");
        typeKeys("1ColumnName<down>D<down>C<down>B<down>A<down>a<down>");
        CalcUtil.selectRange("A6");
        calc.menuItem("Format->Change Case->Lowercase").select();   // In case SC capitalize first letter automatically

        // "Data->Sort...", choose "Ascending", check "Case sensitive"
        calc.menuItem("Data->Sort...").select();
        SortOptionsPage.select();
        SortOptionsPage_CaseSensitive.check();
        SortOptionsPage.ok();

        // Verify sorted result
        assertArrayEquals("Sorted result", expectedSortedResult, CalcUtil.getCellTexts("A2:A6"));

        // Uodo/redo
        calc.menuItem("Edit->Undo: Sort").select();
        assertArrayEquals("Undo sorted result", data, CalcUtil.getCellTexts("A2:A6"));
        calc.menuItem("Edit->Redo: Sort").select();
        assertArrayEquals("Redo sorted result", expectedSortedResult, CalcUtil.getCellTexts("A2:A6"));

        // Save and close document
        String saveTo = fullPath("temp/" + "RowsSortWithOptionsCaseSensitive.ods");
        calc.menuItem("File->Save As...").select();
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        calc.menuItem("File->Close").select();
        openStartcenter();

        // Reopen and verify sorted result
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        assertArrayEquals("Saved sorted result", expectedSortedResult, CalcUtil.getCellTexts("A2:A6"));
    }
}
