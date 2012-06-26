/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package testcase.sc.validity;

import static testlib.AppUtil.fullPath;
import static testlib.AppUtil.initApp;
import static testlib.AppUtil.openStartcenter;
import static testlib.AppUtil.submitSaveDlg;
import static testlib.AppUtil.submitOpenDlg;
import static testlib.AppUtil.testFile;
import static testlib.UIMap.startcenter;
import static testlib.UIMap.*;

import java.io.File;

import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;
import static org.openoffice.test.vcl.Tester.typeKeys;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.vcl.IDList;
import org.openoffice.test.vcl.widgets.VclMessageBox;

import testlib.CalcUtil;
import testlib.Log;
import testlib.UIMap;

public class FFCIgnoreBlank {
        /**
     * TestCapture helps us to doCalcUtil
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
     * Test open MS 2003 spreadsheet with ignore blank validity.
     *
     * @throws Exception
     */
    @Test
    public void testFFCNotIgnoreBlank() throws Exception{
        //open sample file
        String file = testFile("sc/FFC252FFCSC_XML_Datarange0235.xls");
        startcenter.menuItem("File->Open").select();
        submitOpenDlg(file);

        CalcUtil.selectRange("D5");
        SC_CellInput.activate();
        for(int i=1;i<=10;i++)
        typeKeys("<backspace>");
        typeKeys("<enter>");

        assertEquals("",CalcUtil.getCellText("D5"));
    }



}
