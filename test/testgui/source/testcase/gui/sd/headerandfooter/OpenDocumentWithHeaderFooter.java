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

import testlib.gui.Log;

public class OpenDocumentWithHeaderFooter {

    @Rule
    public Log LOG = new Log();

    @Before
    public void setUp() throws Exception {
        app.start(true);    // Bug 120476
    }

    @After
    public void tearDown() throws Exception {

    }

    /**
     * Test open AOO3.4 presentation with header and footer.
     * edit and save to ODP
     * @throws Exception
     */
    @Test
    public void testOpenAOO34WithHeaderFooter() throws Exception{
        //open sample file
        String file = prepareData("sd/AOO3.4HeaderFooter.odp");
        app.dispatch(".uno:Open");
        submitOpenDlg(file);
        impress.waitForExistence(10, 2);

        //check after reopen
        app.dispatch(".uno:HeaderAndFooter");
        assertEquals(true,SD_DateAndTimeFooterOnSlide.isChecked());
        assertEquals("fixed date",SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals(true,SD_FooterTextOnSlide.isChecked());
        assertEquals("footer test",SD_FooterTextOnSlideInput.getText());
        assertEquals(true,SD_SlideNumAsFooterOnSlide.isChecked());

        SD_SlideNumAsFooterOnSlide.uncheck();
        SD_ApplyToAllButtonOnSlideFooter.click();

        //save to odp and reopen
        app.dispatch(".uno:SaveAs");
        String saveTo2 = getPath("temp/" + "AOO3.4HeaderFooter.odp");
        FileUtil.deleteFile(saveTo2);
        submitSaveDlg(saveTo2);
        app.dispatch(".uno:CloseDoc");

        openStartcenter();
        app.dispatch(".uno:Open");
        String openFrom2=getPath("temp/" + "AOO3.4HeaderFooter.odp");
        submitOpenDlg(openFrom2);

        app.dispatch(".uno:HeaderAndFooter");
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
        String file = prepareData("sd/gfdd.ppt");
        app.dispatch(".uno:Open");
        submitOpenDlg(file);
        impress.waitForExistence(10, 2);

        //check after reopen
        app.dispatch(".uno:HeaderAndFooter");
        assertEquals(true,SD_DateAndTimeFooterOnSlide.isChecked());
        assertEquals("testdte",SD_FixedDateAndTimeOnSlideInput.getText());
        assertEquals(true,SD_FooterTextOnSlide.isChecked());
        assertEquals("yesy",SD_FooterTextOnSlideInput.getText());
        assertEquals(true,SD_SlideNumAsFooterOnSlide.isChecked());

        SD_SlideNumAsFooterOnSlide.uncheck();
        SD_ApplyToAllButtonOnSlideFooter.click();

        //save to ppt and reopen
        app.dispatch(".uno:SaveAs");
        String saveTo = getPath("temp/" + "gfdd.ppt");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        if (AlienFormatDlg.exists(3))
            AlienFormatDlg.ok();
        app.dispatch(".uno:CloseDoc");

        openStartcenter();
        app.dispatch(".uno:Open");
        String openFrom=getPath("temp/" + "gfdd.ppt");
        submitOpenDlg(openFrom);

        app.dispatch(".uno:HeaderAndFooter");
        assertEquals(false,SD_SlideNumAsFooterOnSlide.isChecked());
        //end-save to ppt and reopen

        //close Header and Footer dialog.
        SD_ApplyButtonOnSlideFooter.focus();
        typeKeys("<tab>");
        typeKeys("<enter>");
        //end

        //save to odp and reopen
        app.dispatch(".uno:SaveAs");
        String saveTo2 = getPath("temp/" + "gfdd.odp");
        FileUtil.deleteFile(saveTo2);
        submitSaveDlg(saveTo2);
        app.dispatch(".uno:CloseDoc");

        openStartcenter();
        app.dispatch(".uno:Open");
        String openFrom2=getPath("temp/" + "gfdd.odp");
        submitOpenDlg(openFrom2);

        app.dispatch(".uno:HeaderAndFooter");
        assertEquals(false,SD_SlideNumAsFooterOnSlide.isChecked());
        //end-save to odp and reopen
    }
}
