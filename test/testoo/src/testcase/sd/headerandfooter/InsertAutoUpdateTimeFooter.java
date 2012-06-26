/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package testcase.sd.headerandfooter;

import static testlib.AppUtil.fullPath;
import static testlib.AppUtil.initApp;
import static testlib.AppUtil.openStartcenter;
import static testlib.AppUtil.submitSaveDlg;
import static testlib.AppUtil.submitOpenDlg;
import static testlib.UIMap.startcenter;
import static testlib.UIMap.*;

import java.io.File;
import java.util.Calendar;

import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;
import static org.openoffice.test.vcl.Tester.typeKeys;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;

import testlib.Log;
import testlib.UIMap;

public class InsertAutoUpdateTimeFooter {
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
     * Test Insert update automatically time footer to slides.
     * @throws Exception
     */
    @Test
    public void testInsertAutoUpdateTimeFooter() throws Exception{

        startcenter.menuItem("File->New->Presentation").select();
        sleep(2);

        for(int i=0; i<3;i++){
        SD_InsertPageButtonOnToolbar.click();
        sleep(1);
        }

        //add header and footer
        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        SD_AutoUpdateTimeFooter.check();
        SD_AutoUpdateTimeFooterType.select(7);
        String currentTime=SD_AutoUpdateTimeFooterType.getItemText(7);
        SD_ApplyToAllButtonOnSlideFooter.click();
        sleep(2);

        impress.menuItem("View->Header and Footer...").select();
        sleep(1);

        String currentTime2=SD_AutoUpdateTimeFooterType.getItemText(7);
        assertFalse(currentTime.equals(currentTime2));

    }

}
