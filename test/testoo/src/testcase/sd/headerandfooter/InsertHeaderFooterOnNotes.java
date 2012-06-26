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

import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;
import static org.openoffice.test.vcl.Tester.typeKeys;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;

import testlib.Log;
import testlib.UIMap;

public class InsertHeaderFooterOnNotes {
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
     * Test Insert Header and Footer to Notes view.
     * Save and ReOpen
     * @throws Exception
     */
    @Test
    public void testInsertHeaderFooterOnNotes() throws Exception{

        startcenter.menuItem("File->New->Presentation").select();
        sleep(2);

        for(int i=0; i<5;i++){
        SD_InsertPageButtonOnToolbar.click();
        sleep(1);
        }
        //pop up navigator panel
        impress.menuItem("View").select();
        if(!impress.menuItem("View->Navigator").isSelected()){
            impress.menuItem("View->Navigator").select();
            sleep(1);
        }

        //add header and footer
        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        SD_HeaderAndFooterOnNotesTabPage.select();
        SD_HeaderTextOnNotes.check();
        SD_HeaderTextOnNotesInput.setText("Header Test");
        SD_DateAndTimeFooterOnSlide.check();
        SD_FixedDateAndTimeFooterOnSlide.check();
        sleep(1);
        SD_FixedDateAndTimeOnSlideInput.setText("Fix Date: 20120329");
        sleep(1);
        SD_FooterTextOnSlide.check();
        sleep(1);
        SD_FooterTextOnSlideInput.setText("Footer Test");
        sleep(1);
        SD_SlideNumAsFooterOnSlide.check();
        sleep(1);
        SD_ApplyToAllButtonOnSlideFooter.click();
        sleep(1);

        SD_SlidesPanel.focus();
        typeKeys("<up>");

        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        SD_HeaderAndFooterOnNotesTabPage.select();
        assertEquals("Header Test",SD_HeaderTextOnNotesInput.getText());
        assertEquals("Fix Date: 20120329",SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("Footer Test",SD_FooterTextOnSlideInput.getText());
        assertEquals(true,SD_SlideNumAsFooterOnSlide.isChecked());

        //close header and footer dialog.
        SD_ApplyToAllButtonOnSlideFooter.focus();
        typeKeys("<tab>");
        typeKeys("<enter>");

        //save this file
        impress.menuItem("File->Save").select();
        String saveTo = fullPath("temp/" + "hello.odp");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        impress.menuItem("File->Close").select();
        sleep(2);

        //Reopen this file
        openStartcenter();
        startcenter.menuItem("File->Open").select();
        String openFrom=fullPath("temp/" + "hello.odp");
        submitOpenDlg(openFrom);

        //check after reopen
        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        SD_HeaderAndFooterOnNotesTabPage.select();
        assertEquals("Header Test",SD_HeaderTextOnNotesInput.getText());
        assertEquals("Fix Date: 20120329",SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("Footer Test",SD_FooterTextOnSlideInput.getText());
        assertEquals(true,SD_SlideNumAsFooterOnSlide.isChecked());

    }

}
