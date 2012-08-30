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

package testcase.gui;

import static org.openoffice.test.common.Testspace.*;
import static org.openoffice.test.vcl.Tester.*;
import static testlib.gui.AppUtil.*;
import static testlib.gui.UIMap.*;

import java.io.File;
import java.util.ArrayList;
import java.util.Collection;

import junit.framework.Assert;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;
import org.openoffice.test.common.Condition;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.Logger;

import testlib.gui.ImpressUtil;

@RunWith(Parameterized.class)
public class TestSample {

    public static String repos = "samples";

    public static String[][] params = {};

    @Parameters
    public static Collection<Object[]> data() {
        File dir = new File(repos);
        ArrayList<Object[]> list = new ArrayList<Object[]>();
        collect(dir, list);
        return list;
    }

    /**
     * @see <a href="http://www.ibm.com">Manual Case</a>
     * @param dir
     * @param list
     */
    public static void collect(File dir, ArrayList<Object[]> list) {
        File[] files = dir.listFiles();
        if (files == null)
            return;

        for (File file : files) {
            if (file.isDirectory()) {
                collect(file, list);
            } else {
                String fileName = file.getName().toLowerCase();
                for (String[] param : params) {
                    String filter = param[0];
                    if (filter != null && fileName.matches(filter)) {
                        Object[] data = { file, param[1], param[2] };
                        list.add(data);
                        System.out.println(file + param[1] + param[2]);
                        break;
                    }
                }
            }
        }
    }

    private static final String writerFilter = ".*\\.((odt)|(ott)|(sxw)|(stw)|(doc)|(dot)|(docx)|(docm)|(dotx)|(dotm))$";
    private static final String calcFilter = ".*\\.((ods)|(ots)|(sxc)|(stc)|(xls)|(xlt)|(xlsx)|(xltx)|(xlsm)|(xltm))$";
    private static final String impressFilter = ".*\\.((odp)|(otp)|(sxi)|(sti)|(ppt)|(pot)|(pptx)|(pptm)|(potm)|(potx))$";
    private static final String drawFilter = ".*\\.((odg)|(otg)|(sxd)|(sxt))$";
    private static final String databaseFilter = ".*\\.(odb)$";

    @Rule
    public Logger log = Logger.getLogger(this);
    private File originalFile = null;
    private String saveas = null;
    private String editor = null;
    private File file = null;
    private String saveTo = null;
    private boolean passed = false;

    public TestSample(File file, String saveas, String editor) {
        this.originalFile = file;
        this.saveas = saveas;
        this.editor = editor;
    }

    /**
     * @throws java.lang.Exception
     */
    @Before
    public void setUp() {
        app.start();

        FileUtil.deleteFile(getPath("temp"));
        File temp = new File(getPath("temp"));
        temp.mkdirs();
        log.info("Load sample file from \"" + originalFile.getAbsolutePath() + "\"");
        file = new File(temp + "/origin", "sample." + FileUtil.getFileExtName(originalFile.getName()) /*
                                                                                                     * file
                                                                                                     * .
                                                                                                     * getName
                                                                                                     * (
                                                                                                     * )
                                                                                                     */);
        FileUtil.copyFile(originalFile, file); // We use the copy to do test
        saveTo = getPath("temp/" + file.getName() + (saveas == null ? "" : "." + saveas));
    }

    @After
    public void tearDown() {
        if (!passed) {
            // Collect the failed sample files.
            File failedDir = new File(getPath("output/TestSample.Failed"));
            FileUtil.copyFile(originalFile, new File(failedDir, originalFile.getName()));
        }
    }

    @Test
    public void test() {
        if (editor == null) {
            String name = file.getName();
            if (name.matches(writerFilter)) {
                testWriter();
            } else if (name.matches(calcFilter)) {
                testCalc();
            } else if (name.matches(impressFilter)) {
                testImpress();
            } else if (name.matches(drawFilter)) {

            } else if (name.matches(databaseFilter)) {

            } else {
                Assert.assertTrue("It's supported", false);
            }
        } else {
            if (editor.equals("writer"))
                testWriter();
            if (editor.equals("calc"))
                testCalc();
            if (editor.equals("impress"))
                testImpress();
            if (editor.equals("draw"))
                testDraw();
            if (editor.equals("database"))
                testDatabase();
        }
    }

    private void testDatabase() {
        // TODO Auto-generated method stub

    }

    private void testDraw() {
        // TODO Auto-generated method stub

    }

    public void testWriter() {
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file.getAbsolutePath());
        handleBlocker(writer);
        sleep(10);

        // Assert.assertTrue("File Passed:" + file,
        // writer.getCaption().contains(file.getName()));

        writer.menuItem("File->Save As...").select();
        submitSaveDlg(saveTo);
        if (AlienFormatDlg.exists(3))
            AlienFormatDlg.ok();
        sleep(2);
        writer.waitForEnabled(120, 2);

        writer.menuItem("File->Close").select();

        openStartcenter();
        // Reopen the saved file
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(saveTo);
        handleBlocker(writer);
        sleep(10);

        // Assert.assertTrue("File Passed:" + file,
        // writer.getCaption().contains(file.getName()));
        writer.menuItem("File->Close").select();
        passed = true;
    }

    public void testCalc() {
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file.getAbsolutePath());
        handleBlocker(calc);
        sleep(10); // Wait. Crash maybe occurs when the file is shown!

        // Assert.assertTrue("File Passed:" + file,
        // calc.getCaption().contains(file.getName()));

        calc.menuItem("File->Save As...").select();
        submitSaveDlg(saveTo);
        if (AlienFormatDlg.exists(3))
            AlienFormatDlg.ok();
        sleep(2);

        new Condition() {
            @Override
            public boolean value() {
                if (MsgBox_AdditionalRowsNotSaved.exists()) {
                    MsgBox_AdditionalRowsNotSaved.ok();
                }
                return calc.isEnabled();
            }

        }.waitForTrue("Time out to wait the control to be enabled!", 120, 2);

        calc.menuItem("File->Close").select();
        openStartcenter();
        // Reopen the saved file
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(saveTo);
        handleBlocker(calc);
        sleep(10);

        // Assert.assertTrue("File Passed:" + file,
        // calc.getCaption().contains(file.getName()));
        calc.menuItem("File->Close").select();
        passed = true;
    }

    public void testImpress() {
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file.getAbsolutePath());
        handleBlocker(impress, ImpressSlideSorter, ImpressOutline, ImpressHandout);
        sleep(10); // Wait. Crash maybe occurs when the file is shown!
        ImpressUtil.getCurView().menuItem("View->Normal").select();

        // Assert.assertTrue("File Passed:" + file,
        // impress.getCaption().contains(file.getName()));

        impress.menuItem("File->Save As...").select();
        submitSaveDlg(saveTo);
        if (AlienFormatDlg.exists(3))
            AlienFormatDlg.ok();
        sleep(2);
        impress.waitForEnabled(120, 2);
        impress.menuItem("File->Close").select();
        openStartcenter();
        // Reopen the saved file
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(saveTo);
        handleBlocker(impress);
        sleep(10); // Wait.

        // Assert.assertTrue("File Passed:" + file,
        // impress.getCaption().contains(file.getName()));
        impress.menuItem("File->Close").select();
        passed = true;
    }
}
