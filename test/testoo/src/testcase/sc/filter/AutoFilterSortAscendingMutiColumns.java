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
package testcase.sc.filter;

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
public class AutoFilterSortAscendingMutiColumns {
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
     * Verify Auto Filter sort ascending in muti-columns
     */
    @Test
    public void test() {
        String file = testFile("sc/sample_topN3.ods");
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        sleep(2);

        CalcUtil.selectRange("A1:E200");
        calc.menuItem("Data->Filter->AutoFilter").select();
        SpreadSheetCells.click(72, 8);
        typeKeys("<up>");
        typeKeys("<up>");
        typeKeys("<enter>");
        assertEquals("A",CalcUtil.getCellText("A1"));
        assertEquals("B",CalcUtil.getCellText("B1"));
        assertEquals("C",CalcUtil.getCellText("C1"));
        assertEquals("D",CalcUtil.getCellText("D1"));
        assertEquals("E",CalcUtil.getCellText("E1"));
        assertEquals("Item1",CalcUtil.getCellText("A2"));
        assertEquals("5",CalcUtil.getCellText("B2"));
        assertEquals("246",CalcUtil.getCellText("C2"));
        assertEquals("0",CalcUtil.getCellText("D2"));
        assertEquals("24",CalcUtil.getCellText("E2"));
        assertEquals("Item52",CalcUtil.getCellText("A200"));
        assertEquals("52",CalcUtil.getCellText("B200"));
        assertEquals("199",CalcUtil.getCellText("C200"));
        assertEquals("5",CalcUtil.getCellText("D200"));
        assertEquals("19",CalcUtil.getCellText("E200"));
        assertEquals("Item99",CalcUtil.getCellText("A251"));
        assertEquals("99",CalcUtil.getCellText("B251"));
        assertEquals("152",CalcUtil.getCellText("C251"));
        assertEquals("9",CalcUtil.getCellText("D251"));
        assertEquals("15",CalcUtil.getCellText("E251"));

        CalcUtil.selectRange("A1");
        SpreadSheetCells.click(152, 8);
        typeKeys("<up>");
        typeKeys("<up>");
        typeKeys("<enter>");
        assertEquals("A",CalcUtil.getCellText("A1"));
        assertEquals("B",CalcUtil.getCellText("B1"));
        assertEquals("C",CalcUtil.getCellText("C1"));
        assertEquals("D",CalcUtil.getCellText("D1"));
        assertEquals("E",CalcUtil.getCellText("E1"));
        assertEquals("Item1",CalcUtil.getCellText("A2"));
        assertEquals("5",CalcUtil.getCellText("B2"));
        assertEquals("246",CalcUtil.getCellText("C2"));
        assertEquals("0",CalcUtil.getCellText("D2"));
        assertEquals("24",CalcUtil.getCellText("E2"));
        assertEquals("Item199",CalcUtil.getCellText("A200"));
        assertEquals("199",CalcUtil.getCellText("B200"));
        assertEquals("52",CalcUtil.getCellText("C200"));
        assertEquals("19",CalcUtil.getCellText("D200"));
        assertEquals("5",CalcUtil.getCellText("E200"));
        assertEquals("Item250",CalcUtil.getCellText("A251"));
        assertEquals("250",CalcUtil.getCellText("B251"));
        assertEquals("1",CalcUtil.getCellText("C251"));
        assertEquals("25",CalcUtil.getCellText("D251"));
        assertEquals("0",CalcUtil.getCellText("E251"));


        CalcUtil.selectRange("A1");
        SpreadSheetCells.click(72, 8);
        typeKeys("<up>");
        typeKeys("<enter>");
        assertEquals("A",CalcUtil.getCellText("A1"));
        assertEquals("B",CalcUtil.getCellText("B1"));
        assertEquals("C",CalcUtil.getCellText("C1"));
        assertEquals("D",CalcUtil.getCellText("D1"));
        assertEquals("E",CalcUtil.getCellText("E1"));
        assertEquals("Item99",CalcUtil.getCellText("A2"));
        assertEquals("99",CalcUtil.getCellText("B2"));
        assertEquals("152",CalcUtil.getCellText("C2"));
        assertEquals("9",CalcUtil.getCellText("D2"));
        assertEquals("15",CalcUtil.getCellText("E2"));
        assertEquals("Item145",CalcUtil.getCellText("A200"));
        assertEquals("145",CalcUtil.getCellText("B200"));
        assertEquals("106",CalcUtil.getCellText("C200"));
        assertEquals("14",CalcUtil.getCellText("D200"));
        assertEquals("10",CalcUtil.getCellText("E200"));
        assertEquals("Item1",CalcUtil.getCellText("A251"));
        assertEquals("5",CalcUtil.getCellText("B251"));
        assertEquals("246",CalcUtil.getCellText("C251"));
        assertEquals("0",CalcUtil.getCellText("D251"));
        assertEquals("24",CalcUtil.getCellText("E251"));

        CalcUtil.selectRange("A1");
        SpreadSheetCells.click(232, 8);
        typeKeys("<up>");
        typeKeys("<enter>");
        assertEquals("A",CalcUtil.getCellText("A1"));
        assertEquals("B",CalcUtil.getCellText("B1"));
        assertEquals("C",CalcUtil.getCellText("C1"));
        assertEquals("D",CalcUtil.getCellText("D1"));
        assertEquals("E",CalcUtil.getCellText("E1"));
        assertEquals("Item9",CalcUtil.getCellText("A2"));
        assertEquals("5",CalcUtil.getCellText("B2"));
        assertEquals("246",CalcUtil.getCellText("C2"));
        assertEquals("0",CalcUtil.getCellText("D2"));
        assertEquals("24",CalcUtil.getCellText("E2"));
        assertEquals("Item199",CalcUtil.getCellText("A200"));
        assertEquals("199",CalcUtil.getCellText("B200"));
        assertEquals("52",CalcUtil.getCellText("C200"));
        assertEquals("19",CalcUtil.getCellText("D200"));
        assertEquals("5",CalcUtil.getCellText("E200"));
        assertEquals("Item250",CalcUtil.getCellText("A251"));
        assertEquals("250",CalcUtil.getCellText("B251"));
        assertEquals("1",CalcUtil.getCellText("C251"));
        assertEquals("25",CalcUtil.getCellText("D251"));
        assertEquals("0",CalcUtil.getCellText("E251"));


        CalcUtil.selectRange("A1");
        SpreadSheetCells.click(312, 8);
        typeKeys("<up>");
        typeKeys("<up>");
        typeKeys("<enter>");
        assertEquals("A",CalcUtil.getCellText("A1"));
        assertEquals("B",CalcUtil.getCellText("B1"));
        assertEquals("C",CalcUtil.getCellText("C1"));
        assertEquals("D",CalcUtil.getCellText("D1"));
        assertEquals("E",CalcUtil.getCellText("E1"));
        assertEquals("Item9",CalcUtil.getCellText("A2"));
        assertEquals("5",CalcUtil.getCellText("B2"));
        assertEquals("246",CalcUtil.getCellText("C2"));
        assertEquals("0",CalcUtil.getCellText("D2"));
        assertEquals("24",CalcUtil.getCellText("E2"));
        assertEquals("Item199",CalcUtil.getCellText("A200"));
        assertEquals("199",CalcUtil.getCellText("B200"));
        assertEquals("52",CalcUtil.getCellText("C200"));
        assertEquals("19",CalcUtil.getCellText("D200"));
        assertEquals("5",CalcUtil.getCellText("E200"));
        assertEquals("Item250",CalcUtil.getCellText("A251"));
        assertEquals("250",CalcUtil.getCellText("B251"));
        assertEquals("1",CalcUtil.getCellText("C251"));
        assertEquals("25",CalcUtil.getCellText("D251"));
        assertEquals("0",CalcUtil.getCellText("E251"));
    }

}
