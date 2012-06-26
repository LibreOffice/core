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


public class SubtotalsRecalculatesWhenSourceDataChanged {
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
     * Verify Recalculates when source data changed with Average function in SubTotals
     */
    @Test
    public void test() {
        String file = testFile("sc/SubtotalsSampleFile.ods");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        sleep(2);
        CalcUtil.selectRange("A1:E7");
        calc.menuItem("Data->Subtotals...").select();

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
        typeKeys("<tab>");
        typeKeys("<down>");
        typeKeys("<down>");
        sleep(1);
        SCSubTotalsGroup1Dialog.ok();
        sleep(1);

        assertEquals("20",CalcUtil.getCellText("B5"));
        assertEquals("=SUBTOTAL(1;$B$2:$B$4)",CalcUtil.getCellInput("B5"));
        assertEquals("A Average",CalcUtil.getCellText("D5"));

        assertEquals("15",CalcUtil.getCellText("B8"));
        assertEquals("=SUBTOTAL(1;$B$6:$B$7)",CalcUtil.getCellInput("B8"));
        assertEquals("B Average",CalcUtil.getCellText("D8"));

        assertEquals("20",CalcUtil.getCellText("B10"));
        assertEquals("=SUBTOTAL(1;$B$9:$B$9)",CalcUtil.getCellInput("B10"));
        assertEquals("C Average",CalcUtil.getCellText("D10"));

        assertEquals("18.33333333",CalcUtil.getCellText("B11"));
        assertEquals("=SUBTOTAL(1;$B$2:$B$10)",CalcUtil.getCellInput("B11"));
        assertEquals("Grand Total",CalcUtil.getCellText("D11"));

        CalcUtil.selectRange("B4");
        typeKeys("40" + "<enter>");
        sleep(1);
        assertEquals("30",CalcUtil.getCellText("B5"));
        assertEquals("23.33333333",CalcUtil.getCellText("B11"));

        CalcUtil.selectRange("B7");
        typeKeys("50" + "<enter>");
        sleep(1);
        assertEquals("35",CalcUtil.getCellText("B8"));
        assertEquals("30",CalcUtil.getCellText("B11"));

        CalcUtil.selectRange("B9");
        typeKeys("30" + "<enter>");
        sleep(1);
        assertEquals("30",CalcUtil.getCellText("B10"));
        assertEquals("31.66666667",CalcUtil.getCellText("B11"));
    }

}
