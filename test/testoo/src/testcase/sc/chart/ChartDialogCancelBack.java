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
package testcase.sc.chart;

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
 *
 */
public class ChartDialogCancelBack {
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
     * Test cancel and back button in chart wizard dialog
     *
     * @throws java.lang.Exception
     */
    @Test
    public void test() {
        //Create a new text document
        startcenter.menuItem("File->New->Spreadsheet").select();
        sleep(3);
        calc.menuItem("Insert->Chart...").select();
        sleep(1);
        WizardNextButton.click();
        sleep(1);
        assertTrue(ChartRangeChooseTabPage.isEnabled());
        WizardBackButton.click();
        sleep(1);
        assertTrue(ChartTypeChooseTabPage.isEnabled());
        Chart_Wizard.cancel();
        sleep(1);
        assertFalse(Chart_Wizard.exists());
    }

}
