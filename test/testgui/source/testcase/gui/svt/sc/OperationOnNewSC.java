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

import static org.openoffice.test.common.Testspace.*;
import static org.openoffice.test.vcl.Tester.*;
import static testlib.gui.AppUtil.*;
import static testlib.gui.UIMap.*;

import java.io.FileOutputStream;
import java.io.PrintStream;
import java.util.HashMap;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.OpenOffice;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.Logger;
import org.openoffice.test.common.SystemUtil;
import org.openoffice.test.common.Testspace;

import testlib.gui.CalcUtil;

public class OperationOnNewSC {
    @Rule
    public Logger log = Logger.getLogger(this);

    private PrintStream result = null;

    private String pid = null;

    /**
     * @throws java.lang.Exception
     */
    @Before
    public void setUp() throws Exception {
        OpenOffice.killAll();
        app.start();
        result = new PrintStream(new FileOutputStream(Testspace.getFile("output/svt_sc_new.csv")));
        HashMap<String, Object> proccessInfo = SystemUtil.findProcess(".*(soffice\\.bin|soffice.*-env).*");
        pid = (String) proccessInfo.get("pid");
        result.println("Iterator,Time,Memory(KB),CPU(%)");
        log.info("Result will be saved to " + Testspace.getPath("output/svt_sc_new.csv"));
    }

    @After
    public void tearDown() throws Exception {
        app.close();
        result.close();
    }

    @Test
    public void operationOnNewSC() throws Exception {
        String[][] inputStr = { { "3" }, { "2" }, { "5" }, { "1" }, { "6" }, { "4" }, { "10" }, { "8" }, { "9" }, { "7" } };
        String[][] inputStr_InstantFilter = { { "A" }, { "1" }, { "2" }, { "3" }, { "1" }, { "2" }, { "3" }, { "1" }, { "2" }, { "3" } };
        String pic = prepareData("svt/Sunset.jpg");

        for (int i = 0; i < 1000; i++) {
            // Data Sort
            app.dispatch("private:factory/scalc");
            CalcUtil.selectRange("A1");
            typeKeys("3<down>2<down>5<down>1<down>6<down>4<down>10<down>8<down>9<down>7");
            sleep(1);
            app.dispatch(".uno:DataSort");
            SortOptionsPage.select();
            SortOptionsPage_RangeContainsColumnLabels.uncheck();
            SortPage.select();
            SortPage_By1.select(1); // "Column A"
            SortPage_Ascending1.check();
            SortPage.ok();
            sleep(5);

            // Insert Sheet
            app.dispatch(".uno:Insert");
            SCAfterCurrentSheet.check();
            SCNewSheetName.setText("Instant Filter");
            SCInsertSheetDlg.ok();
            sleep(5);

            // Standard Filter
            CalcUtil.selectRange("A1");
            typeKeys("A<down>1<down>2<down>3<down>1<down>2<down>3<down>1<down>2<down>3");
            sleep(1);
            CalcUtil.selectRange("A1");
            app.dispatch(".uno:DataFilterStandardFilter");
            sleep(2);
            FilterValue1.setText("1");
            StandardFilterDlg.ok();
            sleep(5);

            // Data Validate
            app.dispatch(".uno:Insert");
            SCAfterCurrentSheet.check();
            SCNewSheetName.setText("Data Validate");
            SCInsertSheetDlg.ok();
            sleep(5);
            CalcUtil.selectRange("B2:E5");
            sleep(2);

            app.dispatch(".uno:Validation");
            SC_ValidityCriteriaTabpage.select();
            SC_ValidityCriteriaAllowList.select("Whole Numbers");
            SC_ValidityDecimalCompareOperator.select("greater than");
            SC_ValiditySourceInput.setText("1");
            SC_ValidityErrorAlertTabPage.select();
            SC_ValidityShowErrorMessage.check();
            SC_ValidityErrorMessageTitle.setText("Error");
            SC_ValidityErrorMessage.setText("Must greater than 1");
            SC_ValidityErrorAlertTabPage.ok();

            CalcUtil.selectRange("B2");
            typeKeys("0<enter>");
            ActiveMsgBox.ok();
            sleep(1);

            CalcUtil.selectRange("E5");
            typeKeys("1<enter>");
            ActiveMsgBox.ok();
            sleep(1);

            CalcUtil.selectRange("E2");
            typeKeys("2<enter>");
            sleep(5);

            // Input cells, insert pics and chart
            app.dispatch(".uno:Insert");
            SCBeforeCurrentSheet.check();
            SCNewSheetName.setText("InsertObjects");
            SCInsertSheetDlg.ok();
            sleep(5);
            calc.menuItem("Insert->Picture->From File...").select();
            sleep(2);
            FilePicker_Path.setText(pic);
            sleep(1);
            FilePicker_Open.click();
            sleep(5);
            typeKeys("<esc>");
            sleep(5);

            // Save file and close
            String saveTo = "tempSC_New" + i + ".ods";
            calc.menuItem("File->Save As...").select();
            FileUtil.deleteFile(saveTo);
            submitSaveDlg(saveTo);
            if (ActiveMsgBox.exists()) {
                ActiveMsgBox.yes();
                sleep(2);
            }
            calc.menuItem("File->Close").select();

            HashMap<String, Object> perfData = SystemUtil.getProcessPerfData(pid);
            String record = i + "," + System.currentTimeMillis() + "," + perfData.get("rss") + "," + perfData.get("pcpu");
            log.info("Record: " + record);
            result.println(record);
            result.flush();

            sleep(3);
        }
    }

}
