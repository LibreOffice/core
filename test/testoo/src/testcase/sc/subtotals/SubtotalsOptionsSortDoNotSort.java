/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package testcase.sc.subtotals;

import static testlib.AppUtil.*;
import static testlib.UIMap.*;

import java.awt.Rectangle;
import java.io.File;

import org.junit.After;
import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.GraphicsUtil;

import testlib.CalcUtil;
import testlib.Log;


public class SubtotalsOptionsSortDoNotSort {
    /**
     * TestCapture helps us to do 1. Take a screenshot when failure occurs. 2.
     * Collect extra data when OpenOffice crashes.
     */
    @Rule
    public Log LOG = new Log();

    /**
     * initApp helps us to do 1. Patch the OpenOffice to enable automation if
     * necessary. 2. Start OpenOffice with automation enabled if necessary. 3.
     * Reset OpenOffice to startcenter.
     *
     * @throws java.lang.Exception
     */
    @Before
    public void setUp() throws Exception {
        initApp();
    }
    /**
     *
     * Verify Don't sort in Sub totals
     */
    @Test
    public void test() {
        String file = testFile("sc/SubtotalsSampleFile.ods");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        sleep(2);
        CalcUtil.selectRange("A1:E7");
        calc.menuItem("Data->Subtotals...").select();
        sleep(1);

        assertTrue(SCSubTotalsGroup1Dialog.exists());
        assertEquals("- none -",SCSubTotalsGroupByListBox.getItemText(0));
        assertEquals("Level",SCSubTotalsGroupByListBox.getItemText(1));
        assertEquals("Code",SCSubTotalsGroupByListBox.getItemText(2));
        assertEquals("No.",SCSubTotalsGroupByListBox.getItemText(3));
        assertEquals("Team",SCSubTotalsGroupByListBox.getItemText(4));
        assertEquals("Name",SCSubTotalsGroupByListBox.getItemText(5));

        SCSubTotalsGroupByListBox.select("Team");
        sleep(1);
        SCCalcSubTotalForColumns.click(10,25);
        sleep(1);

        SCSubTotalsOptionsTabPage.select();
        SCSubtotalsPreSortToGroupCheckBox.uncheck();
        SCSubTotalsGroup1Dialog .select();
        SCSubTotalsGroup1Dialog .ok();
        sleep(1);

        assertEquals("20",CalcUtil.getCellText("B3"));
        assertEquals("=SUBTOTAL(9;$B$2:$B$2)",CalcUtil.getCellInput("B3"));
        assertEquals("B Sum",CalcUtil.getCellText("D3"));

        assertEquals("20",CalcUtil.getCellText("B5"));
        assertEquals("=SUBTOTAL(9;$B$4:$B$4)",CalcUtil.getCellInput("B5"));
        assertEquals("C Sum",CalcUtil.getCellText("D5"));

        assertEquals("60",CalcUtil.getCellText("B9"));
        assertEquals("=SUBTOTAL(9;$B$6:$B$8)",CalcUtil.getCellInput("B9"));
        assertEquals("A Sum",CalcUtil.getCellText("D9"));

        assertEquals("10",CalcUtil.getCellText("B11"));
        assertEquals("=SUBTOTAL(9;$B$10:$B$10)",CalcUtil.getCellInput("B11"));
        assertEquals("B Sum",CalcUtil.getCellText("D11"));

        assertEquals("110",CalcUtil.getCellText("B12"));
        assertEquals("=SUBTOTAL(9;$B$2:$B$11)",CalcUtil.getCellInput("B12"));
        assertEquals("Grand Total",CalcUtil.getCellText("D12"));
    }

}
