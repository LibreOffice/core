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

/**
 *
 *
 */
public class SubtotalsFunctionStDevPPopulation {
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
     * Verify Product Numbers function in SubTotals
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

        SCCalcSubTotalForColumns.click(10,45);
        sleep(1);
        typeKeys("<tab>");
        typeKeys("<down>");
        typeKeys("<down>");
        typeKeys("<down>");
        typeKeys("<down>");
        typeKeys("<down>");
        typeKeys("<down>");
        typeKeys("<down>");
        typeKeys("<down>");
        sleep(1);
        SCSubTotalsGroup1Dialog.ok();
        sleep(1);

        assertEquals("BS StDev",CalcUtil.getCellText("A5"));
        assertEquals("=SUBTOTAL(8;$C$2:$C$4)",CalcUtil.getCellInput("C5"));
        assertEquals("1.63299316",CalcUtil.getCellText("C5"));

        assertEquals("CS StDev",CalcUtil.getCellText("A7"));
        assertEquals("=SUBTOTAL(8;$C$6:$C$6)",CalcUtil.getCellInput("C7"));
        assertEquals("0",CalcUtil.getCellText("C7"));

        assertEquals("MS StDev",CalcUtil.getCellText("A10"));
        assertEquals("=SUBTOTAL(8;$C$8:$C$9)",CalcUtil.getCellInput("C10"));
        assertEquals("1",CalcUtil.getCellText("C10"));

        assertEquals("Grand Total",CalcUtil.getCellText("A11"));
        assertEquals("=SUBTOTAL(8;$C$2:$C$10)",CalcUtil.getCellInput("C11"));
        assertEquals("1.70782513",CalcUtil.getCellText("C11"));
    }

}
