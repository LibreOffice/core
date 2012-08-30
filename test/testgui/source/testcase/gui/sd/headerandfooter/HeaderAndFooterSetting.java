/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

/**
 *
 */
package testcase.gui.sd.headerandfooter;

import static org.junit.Assert.*;
import static org.openoffice.test.common.Testspace.*;
import static org.openoffice.test.vcl.Tester.*;
import static testlib.gui.AppUtil.*;
import static testlib.gui.UIMap.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.Logger;

import testlib.gui.ImpressUtil;

public class HeaderAndFooterSetting {

    @Rule
    public Logger log = Logger.getLogger(this);

    @Before
    public void setUp() throws Exception {
        app.start(true); // Bug 120476

        // New a impress, insert some slides
        app.dispatch("private:factory/simpress?slot=6686");
        PresentationWizard.ok();
        impress.waitForExistence(10, 2);

        for (int i = 0; i < 5; i++) {
            SD_InsertPageButtonOnToolbar.click();
        }
        // Pop up navigator panel
        if (!SD_NavigatorDlg.exists()) {
            app.dispatch(".uno:Navigator");
        }
    }

    @After
    public void tearDown() throws Exception {

    }

    /**
     * Test Copy slide with Apply Footer to same file and different file
     *
     * @throws Exception
     */
    @Test
    public void testCopySlideWithApplyFooter() throws Exception {

        // add header and footer
        app.dispatch(".uno:HeaderAndFooter");

        SD_DateAndTimeFooterOnSlide.check();
        SD_FixedDateAndTimeFooterOnSlide.check();
        SD_FixedDateAndTimeOnSlideInput.setText("Fix Date: 20120323");
        SD_FooterTextOnSlide.check();
        SD_FooterTextOnSlideInput.setText("Footer Test");
        SD_SlideNumAsFooterOnSlide.check();
        SD_ApplyToAllButtonOnSlideFooter.click();

        // Click slide 3
        ImpressSlideSorter.focus();
        typeKeys("<up><up><up>");
        sleep(1);
        app.dispatch(".uno:HeaderAndFooter");
        SD_SlideNumAsFooterOnSlide.uncheck();
        SD_ApplyButtonOnSlideFooter.click();

        // paste to the same file
        ImpressSlideSorter.focus();
        app.dispatch(".uno:Copy");
        app.dispatch(".uno:Paste");

        app.dispatch(".uno:HeaderAndFooter");
        assertEquals("Fix Date: 20120323", SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("Footer Test", SD_FooterTextOnSlideInput.getText());
        assertEquals(false, SD_SlideNumAsFooterOnSlide.isChecked());
        SD_HeaderAndFooterDlgSlideTab.cancel();

        // paste to different file
        impress.focus();
        app.dispatch("private:factory/simpress?slot=6686");
        PresentationWizard.ok();
        app.dispatch(".uno:Paste");
        ImpressSlideSorter.focus();
        typeKeys("<down>");
        sleep(1);

        app.dispatch(".uno:HeaderAndFooter");
        assertEquals("Fix Date: 20120323", SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("Footer Test", SD_FooterTextOnSlideInput.getText());
        assertEquals(false, SD_SlideNumAsFooterOnSlide.isChecked());
    }

    /**
     * Test Copy slide with Apply to all Footer to same file and different file
     *
     * @throws Exception
     */
    @Test
    public void testCopySlideWithApplyToAllFooter() throws Exception {

        // add header and footer
        app.dispatch(".uno:HeaderAndFooter");

        SD_DateAndTimeFooterOnSlide.check();
        SD_FixedDateAndTimeFooterOnSlide.check();
        SD_FixedDateAndTimeOnSlideInput.setText("Fix Date: 20120323");
        SD_FooterTextOnSlide.check();
        SD_FooterTextOnSlideInput.setText("Footer Test");
        SD_SlideNumAsFooterOnSlide.check();
        SD_ApplyToAllButtonOnSlideFooter.click();

        ImpressSlideSorter.focus();
        for (int j = 0; j <= 2; j++) {
            typeKeys("<up>");
        }
        app.dispatch(".uno:HeaderAndFooter");
        SD_SlideNumAsFooterOnSlide.uncheck();
        SD_ApplyButtonOnSlideFooter.click();

        // paste to the same file
        ImpressSlideSorter.focus();
        typeKeys("<up>");
        app.dispatch(".uno:Copy");
        typeKeys("<down>");
        app.dispatch(".uno:Paste");

        app.dispatch(".uno:HeaderAndFooter");
        assertEquals("Fix Date: 20120323", SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("Footer Test", SD_FooterTextOnSlideInput.getText());
        assertEquals(true, SD_SlideNumAsFooterOnSlide.isChecked());
        SD_HeaderAndFooterDlgSlideTab.cancel();

        // paste to different file
        app.dispatch("private:factory/simpress?slot=6686");
        PresentationWizard.ok();
        app.dispatch(".uno:Paste");
        ImpressSlideSorter.focus();
        typeKeys("<down>");
        sleep(1); // If no sleep, error occur

        app.dispatch(".uno:HeaderAndFooter");
        assertEquals("Fix Date: 20120323", SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("Footer Test", SD_FooterTextOnSlideInput.getText());
        assertEquals(true, SD_SlideNumAsFooterOnSlide.isChecked());
    }

    /**
     * Test Copy slide with Notes Footer to same file and different file
     *
     * @throws Exception
     */
    @Test
    public void testCopySlideWithNotesHeaderFooter() throws Exception {

        // add header and footer
        app.dispatch(".uno:HeaderAndFooter");
        SD_HeaderAndFooterOnNotesTabPage.select();
        SD_HeaderTextOnNotes.check();
        SD_HeaderTextOnNotesInput.setText("Header Test");
        SD_DateAndTimeFooterOnSlide.check();
        SD_FixedDateAndTimeFooterOnSlide.check();
        SD_FixedDateAndTimeOnSlideInput.setText("Fix Date: 20120329");
        SD_FooterTextOnSlide.check();
        SD_FooterTextOnSlideInput.setText("Footer Test");
        SD_SlideNumAsFooterOnSlide.check();
        SD_ApplyToAllButtonOnSlideFooter.click();

        // paste to the same file
        ImpressSlideSorter.focus();
        typeKeys("<up>");
        app.dispatch(".uno:Copy");
        typeKeys("<down>");
        app.dispatch(".uno:Paste");

        app.dispatch(".uno:HeaderAndFooter");
        SD_HeaderAndFooterOnNotesTabPage.select();
        assertEquals("Header Test", SD_HeaderTextOnNotesInput.getText());
        assertEquals("Fix Date: 20120329", SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("Footer Test", SD_FooterTextOnSlideInput.getText());
        assertEquals(true, SD_SlideNumAsFooterOnSlide.isChecked());
        SD_HeaderAndFooterOnNotesTabPage.cancel();

        // paste to different file
        impress.focus();
        app.dispatch("private:factory/simpress?slot=6686");
        PresentationWizard.ok();
        app.dispatch(".uno:Paste");
        ImpressSlideSorter.focus();
        typeKeys("<down>");
        ImpressUtil.getCurView().activate();
        app.dispatch(".uno:HeaderAndFooter");
        SD_HeaderAndFooterOnNotesTabPage.select();
        assertEquals("Header Test", SD_HeaderTextOnNotesInput.getText());
        assertEquals("Fix Date: 20120329", SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("Footer Test", SD_FooterTextOnSlideInput.getText());
        assertEquals(true, SD_SlideNumAsFooterOnSlide.isChecked());
    }

    /**
     * Test duplicate slide with Apply to all Footer to same file
     *
     * @throws Exception
     */
    @Test
    public void testDuplicateSlideWithApplyToAllFooter() throws Exception {

        // add header and footer
        app.dispatch(".uno:HeaderAndFooter");

        SD_DateAndTimeFooterOnSlide.check();
        SD_FixedDateAndTimeFooterOnSlide.check();
        SD_FixedDateAndTimeOnSlideInput.setText("Fix Date: 20120323");
        SD_FooterTextOnSlide.check();
        SD_FooterTextOnSlideInput.setText("Footer Test");
        SD_SlideNumAsFooterOnSlide.check();
        SD_ApplyToAllButtonOnSlideFooter.click();

        ImpressSlideSorter.focus();
        for (int j = 0; j <= 2; j++) {
            typeKeys("<up>");
        }
        app.dispatch(".uno:DuplicatePage");
        app.dispatch(".uno:HeaderAndFooter");
        assertEquals("Fix Date: 20120323", SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("Footer Test", SD_FooterTextOnSlideInput.getText());
        assertEquals(true, SD_SlideNumAsFooterOnSlide.isChecked());
    }

    /**
     * Test footer not show on the first slide.
     *
     * @throws Exception
     */
    @Test
    public void testFooterNotShowOn1stSlide() throws Exception {

        // add header and footer
        app.dispatch(".uno:HeaderAndFooter");
        SD_FooterTextOnSlide.check();
        SD_FooterTextOnSlideInput.setText("Footer Test");
        SD_FooterNotShowOn1stSlide.check();
        SD_ApplyToAllButtonOnSlideFooter.click();

        // Check slide 1
        ImpressSlideSorter.focus();
        // typeKeys("<up><up><up><up><up>"); // Not stable on ubuntu10.04
        // sleep(1); // If no sleep, error occur
        for (int i = 0; i < 5; i++) {
            typeKeys("<up>");
            sleep(1);
        }

        app.dispatch(".uno:HeaderAndFooter");
        assertEquals(true, SD_FooterNotShowOn1stSlide.isChecked());
        assertEquals(false, SD_FooterTextOnSlide.isChecked());
    }

    /**
     * Test Insert Footer to focus slide.
     *
     * @throws Exception
     */
    @Test
    public void testInsertApplyFooterOnSlide() throws Exception {

        // add header and footer to focus slide.
        app.dispatch(".uno:HeaderAndFooter");

        SD_DateAndTimeFooterOnSlide.check();
        SD_FixedDateAndTimeFooterOnSlide.check();
        SD_FixedDateAndTimeOnSlideInput.setText("Fix Date: 20120323");
        SD_FooterTextOnSlide.check();
        SD_FooterTextOnSlideInput.setText("Footer Test");
        SD_SlideNumAsFooterOnSlide.check();
        SD_ApplyButtonOnSlideFooter.click();

        app.dispatch(".uno:HeaderAndFooter");
        assertEquals(true, SD_DateAndTimeFooterOnSlide.isChecked());
        assertEquals(true, SD_FixedDateAndTimeFooterOnSlide.isChecked());
        assertEquals("Fix Date: 20120323", SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals(true, SD_FooterTextOnSlide.isChecked());
        assertEquals("Footer Test", SD_FooterTextOnSlideInput.getText());
        assertEquals(true, SD_SlideNumAsFooterOnSlide.isChecked());
        // close header and footer dialog.
        SD_ApplyButtonOnSlideFooter.focus();
        typeKeys("<tab>");
        typeKeys("<enter>");
        // end close

        ImpressSlideSorter.focus();
        typeKeys("<up>");
        sleep(1); // If no sleep, error occur

        app.dispatch(".uno:HeaderAndFooter");
        assertEquals("", SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("", SD_FooterTextOnSlideInput.getText());
        assertEquals(false, SD_SlideNumAsFooterOnSlide.isChecked());
        // close header and footer dialog.
        SD_ApplyButtonOnSlideFooter.focus();
        typeKeys("<tab>");
        typeKeys("<enter>");
        // end close

        SD_InsertPageButtonOnToolbar.click();
        app.dispatch(".uno:HeaderAndFooter");
        assertEquals("", SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("", SD_FooterTextOnSlideInput.getText());
        assertEquals(false, SD_SlideNumAsFooterOnSlide.isChecked());
    }

    /**
     * Test Insert Footer to Notes View
     *
     * @throws Exception
     */
    @Test
    public void testInsertApplyToAllFooterOnNotes() throws Exception {

        // add header and footer to focus slide.
        app.dispatch(".uno:HeaderAndFooter");
        SD_HeaderAndFooterOnNotesTabPage.select();
        SD_HeaderTextOnNotes.check();
        SD_HeaderTextOnNotesInput.setText("Header Test");
        SD_DateAndTimeFooterOnSlide.check();
        SD_FixedDateAndTimeFooterOnSlide.check();
        SD_FixedDateAndTimeOnSlideInput.setText("Fix Date: 20120323");
        SD_FooterTextOnSlide.check();
        SD_FooterTextOnSlideInput.setText("Footer Test");
        SD_SlideNumAsFooterOnSlide.check();
        SD_ApplyToAllButtonOnSlideFooter.click();

        app.dispatch(".uno:HeaderAndFooter");
        SD_HeaderAndFooterOnNotesTabPage.select();
        assertEquals(true, SD_HeaderTextOnNotes.isChecked());
        assertEquals("Header Test", SD_HeaderTextOnNotesInput.getText());
        assertEquals(true, SD_DateAndTimeFooterOnSlide.isChecked());
        assertEquals(true, SD_FixedDateAndTimeFooterOnSlide.isChecked());
        assertEquals("Fix Date: 20120323", SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals(true, SD_FooterTextOnSlide.isChecked());
        assertEquals("Footer Test", SD_FooterTextOnSlideInput.getText());
        assertEquals(true, SD_SlideNumAsFooterOnSlide.isChecked());
        // close header and footer dialog.
        SD_ApplyToAllButtonOnSlideFooter.focus();
        typeKeys("<tab>");
        typeKeys("<enter>");
        // end close

        ImpressSlideSorter.focus();
        typeKeys("<up>");

        app.dispatch(".uno:HeaderAndFooter");
        SD_HeaderAndFooterOnNotesTabPage.select();
        assertEquals(true, SD_HeaderTextOnNotes.isChecked());
        assertEquals("Header Test", SD_HeaderTextOnNotesInput.getText());
        assertEquals(true, SD_DateAndTimeFooterOnSlide.isChecked());
        assertEquals(true, SD_FixedDateAndTimeFooterOnSlide.isChecked());
        assertEquals("Fix Date: 20120323", SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals(true, SD_FooterTextOnSlide.isChecked());
        assertEquals("Footer Test", SD_FooterTextOnSlideInput.getText());
        assertEquals(true, SD_SlideNumAsFooterOnSlide.isChecked());
        // close header and footer dialog.
        SD_ApplyToAllButtonOnSlideFooter.focus();
        typeKeys("<tab>");
        typeKeys("<enter>");
        // end close

        SD_InsertPageButtonOnToolbar.click();
        app.dispatch(".uno:HeaderAndFooter");
        SD_HeaderAndFooterOnNotesTabPage.select();
        assertEquals(true, SD_HeaderTextOnNotes.isChecked());
        assertEquals("Header Test", SD_HeaderTextOnNotesInput.getText());
        assertEquals(true, SD_DateAndTimeFooterOnSlide.isChecked());
        assertEquals(true, SD_FixedDateAndTimeFooterOnSlide.isChecked());
        assertEquals("Fix Date: 20120323", SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals(true, SD_FooterTextOnSlide.isChecked());
        assertEquals("Footer Test", SD_FooterTextOnSlideInput.getText());
        assertEquals(true, SD_SlideNumAsFooterOnSlide.isChecked());
    }

    /**
     * Test Insert Footer to Slide, use Apply to All
     *
     * @throws Exception
     */
    @Test
    public void testInsertApplyToAllFooterOnSlide() throws Exception {

        // add header and footer to focus slide.
        app.dispatch(".uno:HeaderAndFooter");

        SD_DateAndTimeFooterOnSlide.check();
        SD_FixedDateAndTimeFooterOnSlide.check();
        SD_FixedDateAndTimeOnSlideInput.setText("Fix Date: 20120323");
        SD_FooterTextOnSlide.check();
        SD_FooterTextOnSlideInput.setText("Footer Test");
        SD_SlideNumAsFooterOnSlide.check();
        SD_ApplyToAllButtonOnSlideFooter.click();

        app.dispatch(".uno:HeaderAndFooter");
        assertEquals(true, SD_DateAndTimeFooterOnSlide.isChecked());
        assertEquals(true, SD_FixedDateAndTimeFooterOnSlide.isChecked());
        assertEquals("Fix Date: 20120323", SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals(true, SD_FooterTextOnSlide.isChecked());
        assertEquals("Footer Test", SD_FooterTextOnSlideInput.getText());
        assertEquals(true, SD_SlideNumAsFooterOnSlide.isChecked());
        // close header and footer dialog.
        SD_ApplyButtonOnSlideFooter.focus();
        typeKeys("<tab>");
        typeKeys("<enter>");
        // end close

        ImpressSlideSorter.focus();
        typeKeys("<up>");

        app.dispatch(".uno:HeaderAndFooter");
        assertEquals(true, SD_DateAndTimeFooterOnSlide.isChecked());
        assertEquals(true, SD_FixedDateAndTimeFooterOnSlide.isChecked());
        assertEquals("Fix Date: 20120323", SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals(true, SD_FooterTextOnSlide.isChecked());
        assertEquals("Footer Test", SD_FooterTextOnSlideInput.getText());
        assertEquals(true, SD_SlideNumAsFooterOnSlide.isChecked());
        // close header and footer dialog.
        SD_ApplyButtonOnSlideFooter.focus();
        typeKeys("<tab>");
        typeKeys("<enter>");
        // end close

        SD_InsertPageButtonOnToolbar.click();
        app.dispatch(".uno:HeaderAndFooter");
        assertEquals(true, SD_DateAndTimeFooterOnSlide.isChecked());
        assertEquals(true, SD_FixedDateAndTimeFooterOnSlide.isChecked());
        assertEquals("Fix Date: 20120323", SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals(true, SD_FooterTextOnSlide.isChecked());
        assertEquals("Footer Test", SD_FooterTextOnSlideInput.getText());
        assertEquals(true, SD_SlideNumAsFooterOnSlide.isChecked());
    }

    /**
     * Test Insert update automatically time footer to slides.
     *
     * @throws Exception
     */
    @Test
    public void testInsertAutoUpdateTimeFooter() throws Exception {

        // add header and footer
        app.dispatch(".uno:HeaderAndFooter");
        SD_AutoUpdateTimeFooter.check();
        SD_AutoUpdateTimeFooterType.select(7);
        String currentTime = SD_AutoUpdateTimeFooterType.getItemText(7);
        SD_ApplyToAllButtonOnSlideFooter.click();
        sleep(1); // Wait some time to check the time update

        app.dispatch(".uno:HeaderAndFooter");
        String updatedTime = SD_AutoUpdateTimeFooterType.getItemText(7);
        assertNotSame("Time can not update", currentTime, updatedTime);
    }

    /**
     * Test Insert update automatically time footer to Notes view.
     *
     * @throws Exception
     */
    @Test
    public void testInsertAutoUpdateTimeFooterOnNotes() throws Exception {

        // add header and footer
        app.dispatch(".uno:HeaderAndFooter");
        SD_HeaderAndFooterOnNotesTabPage.select();
        SD_AutoUpdateTimeFooter.check();
        String currentTime = SD_AutoUpdateTimeFooterType.getItemText(7);
        SD_ApplyToAllButtonOnSlideFooter.click();
        sleep(1); // Wait some time to check the time update

        app.dispatch(".uno:HeaderAndFooter");
        SD_HeaderAndFooterOnNotesTabPage.select();
        String updatedTime = SD_AutoUpdateTimeFooterType.getItemText(7);

        assertNotSame("Time can not update", currentTime, updatedTime);
    }

    /**
     * Test Insert Header and Footer to Notes view. Save and ReOpen
     *
     * @throws Exception
     */
    @Test
    public void testInsertHeaderFooterOnNotes() throws Exception {

        // add header and footer
        app.dispatch(".uno:HeaderAndFooter");
        SD_HeaderAndFooterOnNotesTabPage.select();
        SD_HeaderTextOnNotes.check();
        SD_HeaderTextOnNotesInput.setText("Header Test");
        SD_DateAndTimeFooterOnSlide.check();
        SD_FixedDateAndTimeFooterOnSlide.check();
        SD_FixedDateAndTimeOnSlideInput.setText("Fix Date: 20120329");
        SD_FooterTextOnSlide.check();
        SD_FooterTextOnSlideInput.setText("Footer Test");
        SD_SlideNumAsFooterOnSlide.check();
        SD_ApplyToAllButtonOnSlideFooter.click();

        ImpressSlideSorter.focus();
        typeKeys("<up>");

        app.dispatch(".uno:HeaderAndFooter");
        SD_HeaderAndFooterOnNotesTabPage.select();
        assertEquals("Header Test", SD_HeaderTextOnNotesInput.getText());
        assertEquals("Fix Date: 20120329", SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("Footer Test", SD_FooterTextOnSlideInput.getText());
        assertEquals(true, SD_SlideNumAsFooterOnSlide.isChecked());
        SD_HeaderAndFooterOnNotesTabPage.cancel();

        // save this file
        app.dispatch(".uno:SaveAs");
        String saveTo = getPath("temp/" + "hello.odp");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        app.dispatch(".uno:CloseDoc");

        // Reopen this file
        openStartcenter();
        app.dispatch(".uno:Open");
        String openFrom = getPath("temp/" + "hello.odp");
        submitOpenDlg(openFrom);

        // check after reopen
        app.dispatch(".uno:HeaderAndFooter");
        SD_HeaderAndFooterOnNotesTabPage.select();
        assertEquals("Header Test", SD_HeaderTextOnNotesInput.getText());
        assertEquals("Fix Date: 20120329", SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("Footer Test", SD_FooterTextOnSlideInput.getText());
        assertEquals(true, SD_SlideNumAsFooterOnSlide.isChecked());
    }

    /**
     * Test Insert Header and Footer to Slide Save and Reopen
     *
     * @throws Exception
     */
    @Test
    public void testInsertHeaderFooterOnSlide() throws Exception {

        // add header and footer
        app.dispatch(".uno:HeaderAndFooter");
        SD_DateAndTimeFooterOnSlide.check();
        SD_FixedDateAndTimeFooterOnSlide.check();
        SD_FixedDateAndTimeOnSlideInput.setText("Fix Date: 20120329");
        SD_FooterTextOnSlide.check();
        SD_FooterTextOnSlideInput.setText("Footer Test");
        SD_SlideNumAsFooterOnSlide.check();
        SD_ApplyToAllButtonOnSlideFooter.click();

        ImpressSlideSorter.focus();
        typeKeys("<up>");

        app.dispatch(".uno:HeaderAndFooter");
        assertEquals("Fix Date: 20120329", SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("Footer Test", SD_FooterTextOnSlideInput.getText());
        assertEquals(true, SD_SlideNumAsFooterOnSlide.isChecked());

        // close header and footer dialog.
        SD_ApplyButtonOnSlideFooter.focus();
        typeKeys("<tab>");
        typeKeys("<enter>");

        // save this file
        app.dispatch(".uno:SaveAs");
        String saveTo = getPath("temp/" + "hello.odp");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        app.dispatch(".uno:CloseDoc");

        // Reopen this file
        openStartcenter();
        app.dispatch(".uno:Open");
        String openFrom = getPath("temp/" + "hello.odp");
        submitOpenDlg(openFrom);

        // check after reopen
        app.dispatch(".uno:HeaderAndFooter");
        assertEquals("Fix Date: 20120329", SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("Footer Test", SD_FooterTextOnSlideInput.getText());
        assertEquals(true, SD_SlideNumAsFooterOnSlide.isChecked());
    }
}
