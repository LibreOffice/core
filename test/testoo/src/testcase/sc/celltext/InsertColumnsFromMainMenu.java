/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package testcase.sc.celltext;

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

public class InsertColumnsFromMainMenu {
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
     * Insert columns from main menu.
     *
     * @throws Exception
     */
    @Test
    public void insertColumnsFromMainMenu(){
        //Create a new text document
        startcenter.menuItem("File->New->Spreadsheet").select();
        sleep(3);
        //input Test123 into cell A1
        String expect="Test123";
        CalcUtil.selectRange("A1");
        typeKeys(expect+"<enter>");
        CalcUtil.selectRange("A1");
        //Insert 1 columns before column A form main menu,so Cell A1 move B1, check it
        calc.menuItem("Insert->Columns...").select();
        assertEquals("1",SC_NumberOfColumnsRowsToInsert.getText());
        SC_InsertColumnsRowsdialog.ok();
        sleep(1);
        assertEquals(expect,CalcUtil.getCellText("B1"));

        //Check the default insert column number is same as columns you select
        CalcUtil.selectRange("E1");
        typeKeys(expect + "<enter>");
        CalcUtil.selectRange("A1:D4");
        calc.menuItem("Insert->Columns...").select();
        assertEquals("4",SC_NumberOfColumnsRowsToInsert.getText());

        //insert 4 column number after select column, then E1 move to I1, check it
        SC_InsertColumnsRowsAfterSelectColumn.check();
        assertTrue(SC_InsertColumnsRowsAfterSelectColumn.isEnabled());
        SC_InsertColumnsRowsdialog.ok();
        sleep(1);
        assertEquals(expect,CalcUtil.getCellText("I1"));

        //input 1024 in insert columns number, then the warning dialog pop up check it
        CalcUtil.selectRange("A1");
        calc.menuItem("Insert->Columns...").select();
        SC_NumberOfColumnsRowsToInsert.setText("1024");
        SC_InsertColumnsRowsdialog.ok();
        assertEquals("Filled cells cannot be shifted" + "\n"
                + "beyond the sheet.",
                ActiveMsgBox.getMessage());
        typeKeys("<enter>");
    }

}
