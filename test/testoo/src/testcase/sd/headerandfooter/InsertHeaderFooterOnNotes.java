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
