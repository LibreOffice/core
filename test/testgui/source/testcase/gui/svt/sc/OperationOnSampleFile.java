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



package testcase.gui.svt.sc;

import static org.openoffice.test.common.Testspace.prepareData;
import static testlib.gui.AppUtil.typeKeys;
import static org.openoffice.test.vcl.Tester.sleep;
import static org.openoffice.test.vcl.Tester.typeKeys;
import static testlib.gui.AppUtil.submitOpenDlg;
import static testlib.gui.UIMap.SCAfterCurrentSheet;
import static testlib.gui.UIMap.SCInsertSheetDlg;
import static testlib.gui.UIMap.SCNewSheetName;
import static testlib.gui.UIMap.app;
import static testlib.gui.UIMap.writer;
import static testlib.gui.UIMap.*;

import java.io.FileOutputStream;
import java.io.PrintStream;
import java.util.HashMap;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.OpenOffice;
import org.openoffice.test.common.SystemUtil;
import org.openoffice.test.common.Testspace;

import testlib.gui.CalcUtil;
import testlib.gui.Log;

public class OperationOnSampleFile {
    @Rule
    public Log LOG = new Log();

    private PrintStream result = null;

    private String pid = null;

    /**
     * @throws java.lang.Exception
     */
    @Before
    public void setUp() throws Exception {
        OpenOffice.killAll();
        app.start();
        result = new PrintStream(new FileOutputStream(Testspace.getFile("output/svt_sc_sample1.csv")));
        HashMap<String, Object> proccessInfo = SystemUtil.findProcess(".*(soffice\\.bin|soffice.*-env).*");
        pid = (String)proccessInfo.get("pid");
        result.println("Iterator,Time,Memory(KB),CPU(%)");
        LOG.info("Result will be saved to " + Testspace.getPath("output/svt_sc_sample1.csv"));
    }

    @After
    public void tearDown() throws Exception {
        app.close();
        result.close();
    }

    @Test
    public void operationOnSampleFile() throws Exception {
        String file = prepareData("svt/complex.ods");
        String pic = prepareData("svt/Sunset.jpg");
        String [][] inputStr = { { "Area", "Item", "Count" }, { "1", "2", "3" },
                { "4", "5", "6" }, { "7", "8", "9" }, { "10", "11", "12" }};
        for(int i = 0; i < 1000; i++)
        {
            app.dispatch(".uno:Open");
            submitOpenDlg(file);
            calc.waitForExistence(20, 2);
            sleep(2);

            //Insert Chart
            app.dispatch(".uno:Insert");
            SCAfterCurrentSheet.check();
            SCNewSheetName.setText("Instant Chart");
            SCInsertSheetDlg.ok();
            sleep(5);
            inputCells(inputStr);
            sleep(2);
            CalcUtil.selectRange("A1:C5");
            calc.menuItem("Insert->Chart...").select();
            sleep(1);
            Chart_Wizard.ok();
            sleep(5);
            calc.typeKeys("<esc>");
            sleep(5);
            calc.typeKeys("<esc>");
            sleep(5);
            calc.typeKeys("<esc>");
            sleep(5);

            //Insert Graphic and Fontwork
            app.dispatch(".uno:Insert");
            SCAfterCurrentSheet.check();
            SCNewSheetName.setText("Instant Graphic and fontwork");
            SCInsertSheetDlg.ok();
            sleep(5);
            calc.menuItem("Insert->Picture->From File...").select();
            sleep(2);
            FilePicker_Path.setText(pic);
            sleep(1);
            FilePicker_Open.click();
            sleep(5);
            calc.typeKeys("<esc>");
            sleep(2);
            calc.menuItem("View->Toolbars->Fontwork").select();
            sleep(2);
            app.dispatch(".uno:FontworkGalleryFloater");
            sleep(2);
            typeKeys("<right>");
            FontworkGalleryDlg.ok();
            sleep(2);
            calc.typeKeys("<esc>");
            sleep(2);

            // Close file
            calc.menuItem("File->Close").select();
            ActiveMsgBox.no();
            sleep(2);

            HashMap<String, Object> perfData = SystemUtil.getProcessPerfData(pid);
            String record = i + "," + System.currentTimeMillis() + "," + perfData.get("rss") + "," + perfData.get("pcpu");
            LOG.info("Record: " + record);
            result.println(record);
            result.flush();

            sleep(3);

        }


    }
    public static void inputCells(String [][]inputs)
    {
        String back = "";
        for (int i = 0; i < inputs.length; i++) {
            calc.typeKeys(back);
            sleep(1);
            back = "";
            for (int j = 0; j < inputs[i].length; j++) {
                typeKeys(inputs[i][j]);
                typeKeys("<Right>");
                sleep(0.5);
                back += "<Left>";
            }
            back += "<enter>";
        }
        sleep(2);
    }
}
