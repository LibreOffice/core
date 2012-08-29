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



package testcase.gui.bvt;

import static org.junit.Assert.assertTrue;
import static org.openoffice.test.common.FileUtil.readFileAsString;
import static org.openoffice.test.common.Testspace.prepareData;
import static testlib.gui.AppUtil.submitOpenDlg;
import static testlib.gui.UIMap.app;
import static testlib.gui.UIMap.oo;
import static testlib.gui.UIMap.writer;

import java.io.File;

import org.junit.AfterClass;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;

import testlib.gui.Log;

public class SmokeTest {

    @Rule
    public Log LOG = new Log();

    File smoketestOutput;

    @Before
    public void setUp() throws Exception {
//      app.getOpenOffice().cleanUserInstallation();
//      app.start();
        app.start(true);    // Default to clean user installation
        smoketestOutput = new File(oo.getUserInstallation(), "user/temp");
    }

    @AfterClass
    public static void afterClass() throws Exception {
        app.close();
    }

    @Test
    public void testMacro() {
        prepareData("TestExtension.oxt");
        String file = prepareData("smoketestdoc.sxw");
        // Open sample file smoketestdoc.sxw
        app.dispatch(".uno:Open", 3);
        submitOpenDlg(file);
        writer.waitForEnabled(10, 2);
        // Run test cases
        app.dispatch("vnd.sun.star.script:Standard.Global.StartTestWithDefaultOptions?language=Basic&location=document", 120);
        String smoketestlog = readFileAsString(new File(smoketestOutput, "smoketest.log"));
        String testclosurelog = readFileAsString(new File(smoketestOutput, "testclosure.log"));
        LOG.info(smoketestlog + "\n" + testclosurelog);
        assertTrue("No Error", !smoketestlog.contains("error") && !testclosurelog.contains("error"));

    }
}
