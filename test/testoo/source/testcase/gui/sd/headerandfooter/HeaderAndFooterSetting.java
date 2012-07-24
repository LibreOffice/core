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
import static org.openoffice.test.common.Testspace.*;
import static testlib.gui.AppUtil.*;
import static testlib.gui.UIMap.*;
import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;
import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;

import testlib.gui.ImpressUtil;
import testlib.gui.Log;

public class HeaderAndFooterSetting{

    @Rule
    public Log LOG = new Log();

    @Before
    public void setUp() throws Exception {
        app.start();

        // New a impress, insert some slides
        app.dispatch("private:factory/simpress?slot=6686");
        PresentationWizard.ok();
        sleep(1);

        for(int i=0; i<5;i++){
            SD_InsertPageButtonOnToolbar.click();
            sleep(1);
        }
        // Pop up navigator panel
        impress.menuItem("View").select();
        if(!impress.menuItem("View->Navigator").isSelected()){
            impress.menuItem("View->Navigator").select();
        }
    }

    @After
    public void tearDown() throws Exception {
        app.close();
    }

    /**
     * Test Copy slide with Apply Footer to same file and different file
     * @throws Exception
     */
    @Test
    public void testCopySlideWithApplyFooter() throws Exception{

        //add header and footer
        impress.menuItem("View->Header and Footer...").select();
        sleep(2);

        SD_DateAndTimeFooterOnSlide.check();
        SD_FixedDateAndTimeFooterOnSlide.check();
        SD_FixedDateAndTimeOnSlideInput.setText("Fix Date: 20120323");
        SD_FooterTextOnSlide.check();
        SD_FooterTextOnSlideInput.setText("Footer Test");
        SD_SlideNumAsFooterOnSlide.check();
        SD_ApplyToAllButtonOnSlideFooter.click();
        sleep(1);

        ImpressSlideSorter.focus();
        for(int j=0; j<=2;j++){
            typeKeys("<up>");
        }
        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        SD_SlideNumAsFooterOnSlide.uncheck();
        SD_ApplyButtonOnSlideFooter.click();

        //paste to the same file
        ImpressSlideSorter.focus();
        app.dispatch(".uno:Copy");
        app.dispatch(".uno:Paste");

        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        assertEquals("Fix Date: 20120323", SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("Footer Test", SD_FooterTextOnSlideInput.getText());
        assertEquals(false, SD_SlideNumAsFooterOnSlide.isChecked());

        //close header and footer dialog.
        SD_ApplyButtonOnSlideFooter.focus();
        typeKeys("<tab>");
        typeKeys("<enter>");

        //paste to different file
        impress.focus();
        app.dispatch("private:factory/simpress?slot=6686");
        PresentationWizard.ok();
        sleep(1);
        app.dispatch(".uno:Paste");
        ImpressSlideSorter.focus();
        typeKeys("<down>");
        sleep(1);

        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        assertEquals("Fix Date: 20120323", SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("Footer Test", SD_FooterTextOnSlideInput.getText());
        assertEquals(false, SD_SlideNumAsFooterOnSlide.isChecked());
    }

    /**
     * Test Copy slide with Apply to all Footer to same file and different file
     * @throws Exception
     */
    @Test
    public void testCopySlideWithApplyToAllFooter() throws Exception{

        //add header and footer
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
        SD_ApplyToAllButtonOnSlideFooter.click();
        sleep(1);

        ImpressSlideSorter.focus();
        for(int j=0; j<=2;j++){
            typeKeys("<up>");
            sleep(1);
        }
        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        SD_SlideNumAsFooterOnSlide.uncheck();
        SD_ApplyButtonOnSlideFooter.click();

        //paste to the same file
        ImpressSlideSorter.focus();
        typeKeys("<up>");
        app.dispatch(".uno:Copy");
        typeKeys("<down>");
        sleep(1);
        app.dispatch(".uno:Paste");

        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        assertEquals("Fix Date: 20120323",SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("Footer Test",SD_FooterTextOnSlideInput.getText());
        assertEquals(true,SD_SlideNumAsFooterOnSlide.isChecked());

        //close header and footer dialog.
        SD_ApplyButtonOnSlideFooter.focus();
        typeKeys("<tab>");
        typeKeys("<enter>");

        //paste to different file
        impress.focus();
        impress.menuItem("File->New->Presentation").select();
        sleep(1);
        app.dispatch(".uno:Paste");
        ImpressSlideSorter.focus();
        typeKeys("<down>");
        sleep(1);

        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        assertEquals("Fix Date: 20120323",SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("Footer Test",SD_FooterTextOnSlideInput.getText());
        assertEquals(true,SD_SlideNumAsFooterOnSlide.isChecked());
    }

    /**
     * Test Copy slide with Notes Footer to same file and different file
     * @throws Exception
     */
    @Test
    public void testCopySlideWithNotesHeaderFooter() throws Exception{

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

        //paste to the same file
        ImpressSlideSorter.focus();
        typeKeys("<up>");
        app.dispatch(".uno:Copy");
        typeKeys("<down>");
        sleep(1);
        app.dispatch(".uno:Paste");

        ImpressUtil.getCurView().activate();

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

        //paste to different file
        impress.focus();
        impress.menuItem("File->New->Presentation").select();
        sleep(1);
        app.dispatch(".uno:Paste");
        ImpressSlideSorter.focus();
        typeKeys("<down>");
        sleep(1);
        ImpressUtil.getCurView().activate();
        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        SD_HeaderAndFooterOnNotesTabPage.select();
        assertEquals("Header Test",SD_HeaderTextOnNotesInput.getText());
        assertEquals("Fix Date: 20120329",SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("Footer Test",SD_FooterTextOnSlideInput.getText());
        assertEquals(true,SD_SlideNumAsFooterOnSlide.isChecked());
    }

    /**
     * Test duplicate slide with Apply to all Footer to same file
     * @throws Exception
     */
    @Test
    public void testDuplicateSlideWithApplyToAllFooter() throws Exception{

        //add header and footer
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
        SD_ApplyToAllButtonOnSlideFooter.click();
        sleep(1);


        ImpressSlideSorter.focus();
        for(int j=0; j<=2;j++){
            typeKeys("<up>");
            sleep(1);
        }
        impress.menuItem("Insert->Duplicate Slide").select();
        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        assertEquals("Fix Date: 20120323",SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("Footer Test",SD_FooterTextOnSlideInput.getText());
        assertEquals(true,SD_SlideNumAsFooterOnSlide.isChecked());
    }

    /**
     * Test footer not show on the first slide.
     * @throws Exception
     */
    @Test
    public void testFooterNotShowOn1stSlide() throws Exception{

        //add header and footer
        impress.menuItem("View->Header and Footer...").select();
        sleep(2);
        SD_FooterTextOnSlide.check();
        sleep(1);
        SD_FooterTextOnSlideInput.setText("Footer Test");
        sleep(1);
        SD_FooterNotShowOn1stSlide.check();
        SD_ApplyToAllButtonOnSlideFooter.click();
        sleep(1);

        ImpressSlideSorter.focus();
        for(int j=0; j<5; j++){
            typeKeys("<up>");
        }

        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        assertEquals(true,SD_FooterNotShowOn1stSlide.isChecked());
        assertEquals(false,SD_FooterTextOnSlide.isChecked());
    }

    /**
     * Test Insert Footer to focus slide.
     * @throws Exception
     */
    @Test
    public void testInsertApplyFooterOnSlide() throws Exception{

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

        ImpressSlideSorter.focus();
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

    /**
     * Test Insert Footer to Notes View
     * @throws Exception
     */
    @Test
    public void testInsertApplyToAllFooterOnNotes() throws Exception{

        //add header and footer to focus slide.
        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        SD_HeaderAndFooterOnNotesTabPage.select();
        SD_HeaderTextOnNotes.check();
        SD_HeaderTextOnNotesInput.setText("Header Test");
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
        SD_ApplyToAllButtonOnSlideFooter.click();
        sleep(1);

        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        SD_HeaderAndFooterOnNotesTabPage.select();
        assertEquals(true,SD_HeaderTextOnNotes.isChecked());
        assertEquals("Header Test",SD_HeaderTextOnNotesInput.getText());
        assertEquals(true,SD_DateAndTimeFooterOnSlide.isChecked());
        assertEquals(true,SD_FixedDateAndTimeFooterOnSlide.isChecked());
        assertEquals("Fix Date: 20120323",SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals(true,SD_FooterTextOnSlide.isChecked());
        assertEquals("Footer Test",SD_FooterTextOnSlideInput.getText());
        assertEquals(true,SD_SlideNumAsFooterOnSlide.isChecked());
        //close header and footer dialog.
        SD_ApplyToAllButtonOnSlideFooter.focus();
        typeKeys("<tab>");
        typeKeys("<enter>");
        //end close

        ImpressSlideSorter.focus();
        typeKeys("<up>");

        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        SD_HeaderAndFooterOnNotesTabPage.select();
        assertEquals(true,SD_HeaderTextOnNotes.isChecked());
        assertEquals("Header Test",SD_HeaderTextOnNotesInput.getText());
        assertEquals(true,SD_DateAndTimeFooterOnSlide.isChecked());
        assertEquals(true,SD_FixedDateAndTimeFooterOnSlide.isChecked());
        assertEquals("Fix Date: 20120323",SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals(true,SD_FooterTextOnSlide.isChecked());
        assertEquals("Footer Test",SD_FooterTextOnSlideInput.getText());
        assertEquals(true,SD_SlideNumAsFooterOnSlide.isChecked());
        //close header and footer dialog.
        SD_ApplyToAllButtonOnSlideFooter.focus();
        typeKeys("<tab>");
        typeKeys("<enter>");
        //end close

        SD_InsertPageButtonOnToolbar.click();
        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        SD_HeaderAndFooterOnNotesTabPage.select();
        assertEquals(true,SD_HeaderTextOnNotes.isChecked());
        assertEquals("Header Test",SD_HeaderTextOnNotesInput.getText());
        assertEquals(true,SD_DateAndTimeFooterOnSlide.isChecked());
        assertEquals(true,SD_FixedDateAndTimeFooterOnSlide.isChecked());
        assertEquals("Fix Date: 20120323",SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals(true,SD_FooterTextOnSlide.isChecked());
        assertEquals("Footer Test",SD_FooterTextOnSlideInput.getText());
        assertEquals(true,SD_SlideNumAsFooterOnSlide.isChecked());
    }

    /**
     * Test Insert Footer to Slide, use Apply to All
     * @throws Exception
     */
    @Test
    public void testInsertApplyToAllFooterOnSlide() throws Exception{

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
        SD_ApplyToAllButtonOnSlideFooter.click();
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

        ImpressSlideSorter.focus();
        typeKeys("<up>");

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

        SD_InsertPageButtonOnToolbar.click();
        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        assertEquals(true,SD_DateAndTimeFooterOnSlide.isChecked());
        assertEquals(true,SD_FixedDateAndTimeFooterOnSlide.isChecked());
        assertEquals("Fix Date: 20120323",SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals(true,SD_FooterTextOnSlide.isChecked());
        assertEquals("Footer Test",SD_FooterTextOnSlideInput.getText());
        assertEquals(true,SD_SlideNumAsFooterOnSlide.isChecked());
    }

    /**
     * Test Insert update automatically time footer to slides.
     * @throws Exception
     */
    @Test
    public void testInsertAutoUpdateTimeFooter() throws Exception{

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

    /**
     * Test Insert update automatically time footer to Notes view.
     * @throws Exception
     */
    @Test
    public void testInsertAutoUpdateTimeFooterOnNotes() throws Exception{

        //add header and footer
        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        SD_HeaderAndFooterOnNotesTabPage.select();
        SD_AutoUpdateTimeFooter.check();
        SD_AutoUpdateTimeFooterType.select(7);
        String currentTime=SD_AutoUpdateTimeFooterType.getItemText(7);
        SD_ApplyToAllButtonOnSlideFooter.click();
        sleep(2);

        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        SD_HeaderAndFooterOnNotesTabPage.select();
        String currentTime2=SD_AutoUpdateTimeFooterType.getItemText(7);

        assertFalse(currentTime.equals(currentTime2));
    }

    /**
     * Test Insert Header and Footer to Notes view.
     * Save and ReOpen
     * @throws Exception
     */
    @Test
    public void testInsertHeaderFooterOnNotes() throws Exception{

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

        ImpressSlideSorter.focus();
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
        String saveTo = getPath("temp/" + "hello.odp");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        impress.menuItem("File->Close").select();
        sleep(2);

        //Reopen this file
        openStartcenter();
        app.dispatch(".uno:Open");
        String openFrom=getPath("temp/" + "hello.odp");
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

    /**
     * Test Insert Header and Footer to Slide
     * Save and Reopen
     * @throws Exception
     */
    @Test
    public void testInsertHeaderFooterOnSlide() throws Exception{

        //add header and footer
        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
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

        ImpressSlideSorter.focus();
        typeKeys("<up>");

        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        assertEquals("Fix Date: 20120329",SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("Footer Test",SD_FooterTextOnSlideInput.getText());
        assertEquals(true,SD_SlideNumAsFooterOnSlide.isChecked());

        //close header and footer dialog.
        SD_ApplyButtonOnSlideFooter.focus();
        typeKeys("<tab>");
        typeKeys("<enter>");

        //save this file
        impress.menuItem("File->Save").select();
        String saveTo = getPath("temp/" + "hello.odp");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        impress.menuItem("File->Close").select();
        sleep(2);

        //Reopen this file
        openStartcenter();
        app.dispatch(".uno:Open");
        String openFrom=getPath("temp/" + "hello.odp");
        submitOpenDlg(openFrom);

        //check after reopen
        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        assertEquals("Fix Date: 20120329",SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals("Footer Test",SD_FooterTextOnSlideInput.getText());
        assertEquals(true,SD_SlideNumAsFooterOnSlide.isChecked());
    }
}
