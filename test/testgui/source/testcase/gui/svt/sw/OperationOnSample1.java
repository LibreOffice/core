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
package testcase.gui.svt.sw;

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
import org.openoffice.test.common.Logger;
import org.openoffice.test.common.SystemUtil;
import org.openoffice.test.common.Testspace;

public class OperationOnSample1 {

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
        result = new PrintStream(new FileOutputStream(Testspace.getFile("output/svt_sw_sample1.csv")));
        String processSoffice = null;
        if (SystemUtil.isWindows()) {
            processSoffice = ".*soffice\\.exe.*";
        } else {
            processSoffice = ".*soffice\\.bin.*";
        }
        HashMap<String, Object> proccessInfo = SystemUtil.findProcess(processSoffice);
        pid = (String) proccessInfo.get("pid");
        result.println("Iterator,Time,Memory(KB),CPU(%)");
        log.info("Result will be saved to " + Testspace.getPath("output/svt_sw_sample1.csv"));
    }

    @After
    public void tearDown() throws Exception {
        app.close();
        result.close();
    }

    @Test
    public void operationOnSample1() throws Exception {
        String file = prepareData("svt/ScenarioDesign.odt");
        for (int i = 0; i < 1000; i++) {
            System.out.println("This is the " + i + " round");
            app.dispatch(".uno:Open");
            submitOpenDlg(file);
            writer.waitForExistence(10, 2);
            writer.typeKeys("<down>");
            writer.typeKeys("<down>");
            sleep(2);

            writer.menuItem("Table->Split Cells").select();
            sleep(2);
            Writer_SplitCellDlg.ok();
            sleep(2);

            writer.typeKeys("<shift down>");
            writer.typeKeys("<shift down>");
            writer.typeKeys("<shift down>");
            writer.typeKeys("<shift down>");
            writer.typeKeys("<shift down>");
            sleep(2);
            writer.menuItem("Table->Merge Cells").select();
            sleep(2);

            writer.typeKeys("<enter>");
            // writer.menuItem("Table->Insert->Rows...").select();
            app.dispatch(".uno:InsertRowDialog");
            Writer_InsertRowsDlg.ok();
            sleep(2);

            writer.menuItem("Table->Delete->Rows").select();
            sleep(2);

            app.dispatch(".uno:InsertColumnDialog");
            Writer_InsertColumnsDlg.ok();
            sleep(2);

            writer.menuItem("Table->Delete->Columns").select();
            sleep(2);

            writer.menuItem("File->Close").select();
            ActiveMsgBox.no();
            sleep(2);

            HashMap<String, Object> perfData = SystemUtil.getProcessPerfData(pid);
            String record = i + "," + System.currentTimeMillis() + "," + perfData.get("rss") + "," + perfData.get("pcpu");
            log.info("Record: " + record);
            result.println(record);
            result.flush();

            sleep(3);
        }
    }

}
