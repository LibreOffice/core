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

import testlib.Log;
import testlib.UIMap;

public class OpenAOO34WithHeaderFooter {
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
     * Test open AOO3.4 presentation with header and footer.
     * edit and save to ODP
     * @throws Exception
     */
    @Test
    public void testOpenAOO34WithHeaderFooter() throws Exception{
        //open sample file
        String file = testFile("sd/AOO3.4HeaderFooter.odp");
        startcenter.menuItem("File->Open").select();
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
        startcenter.menuItem("File->Open").select();
        String openFrom2=fullPath("temp/" + "AOO3.4HeaderFooter.odp");
        submitOpenDlg(openFrom2);

        impress.menuItem("View->Header and Footer...").select();
        sleep(1);
        assertEquals(false,SD_SlideNumAsFooterOnSlide.isChecked());
        //end-save to odp and reopen
    }



}
