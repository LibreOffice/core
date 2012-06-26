/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package testcase.sd.headerandfooter;

import static testlib.AppUtil.initApp;
import static testlib.UIMap.startcenter;
import static testlib.UIMap.*;

import java.io.File;


import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;
import static org.openoffice.test.vcl.Tester.typeKeys;


import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.vcl.IDList;
import org.openoffice.test.vcl.widgets.VclButton;
import org.openoffice.test.vcl.widgets.VclDialog;
import org.openoffice.test.vcl.widgets.VclMessageBox;
import org.openoffice.test.vcl.widgets.VclTabControl;

import testlib.Log;
import testlib.UIMap;

public class InsertApplyFooterOnSlide {
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
     * Test Insert Footer to focus slide.
     * @throws Exception
     */
    @Test
    public void testInsertApplyFooterOnSlide() throws Exception{

        startcenter.menuItem("File->New->Presentation").select();
        sleep(2);

        for(int i=0; i<5;i++){
        SD_InsertPageButtonOnToolbar.click();
        sleep(1);
        }

        //add header and footer to focus slide.
        impress.menuItem("View->Header and Footer...").select();
        sleep(2);

        SD_DateAndTimeFooterOnSlide.check();
        SD_FixedDateAndTimeFooterOnSlide.check();
        sleep(1);
        SD_FixedDateAndTimeOnSlideInput.setText("Fix Date: 20120323");
        sleep(1);
        SD_FooterTextOnSlide.check();
        sleep(1);
        SD_FooterTextOnSlideInput.setText("Footer Test");
        sleep(1);
        SD_SlideNumAsFooterOnSlide.check();
        sleep(1);
        SD_ApplyButtonOnSlideFooter.click();
        sleep(1);

        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        assertEquals(true,SD_DateAndTimeFooterOnSlide.isChecked());
        assertEquals(true,SD_FixedDateAndTimeFooterOnSlide.isChecked());
        assertEquals("Fix Date: 20120323",SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals(true,SD_FooterTextOnSlide.isChecked());
        assertEquals("Footer Test",SD_FooterTextOnSlideInput.getText());
        assertEquals(true,SD_SlideNumAsFooterOnSlide.isChecked());
        //close header and footer dialog.
        SD_ApplyButtonOnSlideFooter.focus();
        typeKeys("<tab>");
        typeKeys("<enter>");
        //end close

        SD_SlidesPanel.focus();
        typeKeys("<up>");

        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        assertEquals("",SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("",SD_FooterTextOnSlideInput.getText());
        assertEquals(false,SD_SlideNumAsFooterOnSlide.isChecked());
        //close header and footer dialog.
        SD_ApplyButtonOnSlideFooter.focus();
        typeKeys("<tab>");
        typeKeys("<enter>");
        //end close

        SD_InsertPageButtonOnToolbar.click();
        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        assertEquals("",SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("",SD_FooterTextOnSlideInput.getText());
        assertEquals(false,SD_SlideNumAsFooterOnSlide.isChecked());
    }

}
