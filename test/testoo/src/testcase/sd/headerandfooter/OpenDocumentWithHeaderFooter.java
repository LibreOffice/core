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
package testcase.sd.headerandfooter;

import static testlib.AppUtil.fullPath;
import static testlib.AppUtil.openStartcenter;
import static testlib.AppUtil.submitSaveDlg;
import static testlib.AppUtil.submitOpenDlg;
import static testlib.AppUtil.testFile;
import static testlib.UIMap.*;
import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;
import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import testlib.Log;

public class OpenDocumentWithHeaderFooter {

    @Rule
    public Log LOG = new Log();

    @Before
    public void setUp() throws Exception {
        app.start();
    }

    @After
    public void tearDown() throws Exception {
        app.close();
    }

    /**
     * Test open AOO3.4 presentation with header and footer.
     * edit and save to ODP
     * @throws Exception
     */
    @Test
    public void testOpenAOO34WithHeaderFooter() throws Exception{
        //open sample file
        String file = testFile("sd/AOO3.4HeaderFooter.odp");
        app.dispatch(".uno:Open");
        submitOpenDlg(file);

        //check after reopen
        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        assertEquals(true,SD_DateAndTimeFooterOnSlide.isChecked());
        assertEquals("fixed date",SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals(true,SD_FooterTextOnSlide.isChecked());
        assertEquals("footer test",SD_FooterTextOnSlideInput.getText());
        assertEquals(true,SD_SlideNumAsFooterOnSlide.isChecked());

        SD_SlideNumAsFooterOnSlide.uncheck();
        SD_ApplyToAllButtonOnSlideFooter.click();

        //save to odp and reopen
        impress.menuItem("File->Save As...").select();
        String saveTo2 = fullPath("temp/" + "AOO3.4HeaderFooter.odp");
        FileUtil.deleteFile(saveTo2);
        submitSaveDlg(saveTo2);
        impress.menuItem("File->Close").select();
        sleep(1);

        openStartcenter();
        app.dispatch(".uno:Open");
        String openFrom2=fullPath("temp/" + "AOO3.4HeaderFooter.odp");
        submitOpenDlg(openFrom2);

        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        assertEquals(false,SD_SlideNumAsFooterOnSlide.isChecked());
        //end-save to odp and reopen
    }

    /**
     * Test open ppt file with header and footer.
     * edit and save to PPT/ODP
     * @throws Exception
     */
    @Test
    public void testOpenPPTWithHeaderFooter() throws Exception{
        //open sample file
        String file = testFile("sd/gfdd.ppt");
        app.dispatch(".uno:Open");
        submitOpenDlg(file);

        //check after reopen
        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        assertEquals(true,SD_DateAndTimeFooterOnSlide.isChecked());
        assertEquals("testdte",SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals(true,SD_FooterTextOnSlide.isChecked());
        assertEquals("yesy",SD_FooterTextOnSlideInput.getText());
        assertEquals(true,SD_SlideNumAsFooterOnSlide.isChecked());

        SD_SlideNumAsFooterOnSlide.uncheck();
        SD_ApplyToAllButtonOnSlideFooter.click();

        //save to ppt and reopen
        impress.menuItem("File->Save As...").select();
        String saveTo = fullPath("temp/" + "gfdd.ppt");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        if (AlienFormatDlg.exists(3))
            AlienFormatDlg.ok();
        sleep(1);
        impress.menuItem("File->Close").select();
        sleep(1);

        openStartcenter();
        app.dispatch(".uno:Open");
        String openFrom=fullPath("temp/" + "gfdd.ppt");
        submitOpenDlg(openFrom);

        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        assertEquals(false,SD_SlideNumAsFooterOnSlide.isChecked());
        //end-save to ppt and reopen

        //close Header and Footer dialog.
        SD_ApplyButtonOnSlideFooter.focus();
        typeKeys("<tab>");
        typeKeys("<enter>");
        //end

        //save to odp and reopen
        impress.menuItem("File->Save As...").select();
        String saveTo2 = fullPath("temp/" + "gfdd.odp");
        FileUtil.deleteFile(saveTo2);
        submitSaveDlg(saveTo2);
        impress.menuItem("File->Close").select();
        sleep(1);

        openStartcenter();
        app.dispatch(".uno:Open");
        String openFrom2=fullPath("temp/" + "gfdd.odp");
        submitOpenDlg(openFrom2);

        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        assertEquals(false,SD_SlideNumAsFooterOnSlide.isChecked());
        //end-save to odp and reopen
    }
}
