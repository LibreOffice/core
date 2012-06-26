/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package testcase;

import static org.junit.Assert.*;
import static testlib.AppUtil.*;
import static testlib.UIMap.*;
import static org.openoffice.test.common.FileUtil.*;

import java.io.File;

import org.junit.AfterClass;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;

import testlib.AppUtil;
import testlib.Log;

public class SmokeTest {

    @Rule
    public Log LOG = new Log();

    File smoketestOutput;
    @Before
    public void setUp() throws Exception {
        initApp();
        smoketestOutput = new File(AppUtil.getUserInstallationDir(), "user/temp");
        deleteFile(smoketestOutput);
    }

    @Test
    public void test() {
        testFile("TestExtension.oxt");
        String file = testFile("smoketestdoc.sxw");
        // Open sample file smoketestdoc.sxw
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        writer.waitForEnabled(10, 2);
        // Run test cases
        app.dispatch("vnd.sun.star.script:Standard.Global.StartTestWithDefaultOptions?language=Basic&location=document", 120);
        String smoketestlog = readFileAsString(new File(smoketestOutput, "smoketest.log"));
        String testclosurelog = readFileAsString(new File(smoketestOutput, "testclosure.log"));
        LOG.info(smoketestlog + "\n" + testclosurelog);
        assertTrue("No Error", !smoketestlog.contains("error") && !testclosurelog.contains("error"));

    }

    @AfterClass
    public static void afterClass() {
        app.kill();
    }
}
