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

import static org.openoffice.test.common.Testspace.prepareData;
import static testlib.gui.AppUtil.submitOpenDlg;
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
import static org.openoffice.test.vcl.Tester.*;

import testlib.gui.Log;

public class OperationOnSample2 {

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
        result = new PrintStream(new FileOutputStream(Testspace.getFile("output/svt_sw_sample2.csv")));
        String processSoffice = null;
        if(SystemUtil.isWindows())
        {
            processSoffice = ".*soffice\\.exe.*";
        }
        else
        {
            processSoffice = ".*soffice\\.bin.*";
        }
        HashMap<String, Object> proccessInfo = SystemUtil.findProcess(processSoffice);
        pid = (String)proccessInfo.get("pid");
        result.println("Iterator,Time,Memory(KB),CPU(%)");
        LOG.info("Result will be saved to " + Testspace.getPath("output/svt_sw_sample2.csv"));
    }

    @After
    public void tearDown() throws Exception {
        app.close();
        result.close();
    }

    @Test
    public void operationOnSample2() throws Exception {
        String file = prepareData("svt/complex.odt");
        String pic = prepareData("svt/Sunset.jpg");

        for(int i = 0; i < 1000; i++)
        {
            System.out.println("This is the " + i + " round");
            app.dispatch(".uno:Open");
            submitOpenDlg(file);
            writer.waitForExistence(20, 2);

//  Set Paragraph properties
            app.dispatch(".uno:SelectAll");
            sleep(2);
            writer.menuItem("Format->Paragraph...").select();
            AlignmentTabPage.select();
            Right.check();
            sleep(2);
            VerticalAlignment.select("Bottom");
            sleep(2);
            AlignmentTabPage.ok();
            sleep(2);
            writer.typeKeys("<ctrl end>");
            sleep(5);

// Insert Picture from file
            writer.menuItem("Insert->Picture->From File...").select();
            sleep(2);
            FilePicker_Path.setText(pic);
            sleep(1);
            FilePicker_Open.click();
            sleep(5);
            writer.typeKeys("<esc>");
            sleep(2);

// Insert OLE Object
//          writer.menuItem("Insert->Object->OLE Object...").select();
//          sleep(2);
            app.dispatch(".uno:InsertObject");
            sleep(2);
            InsertObject.ok();
            sleep(10);
            typeKeys("<esc>");
            sleep(5);
            typeKeys("<esc>");
            sleep(5);

// Insert Chart
            writer.menuItem("Insert->Object->Chart...").select();
            sleep(5);
            typeKeys("<esc>");
            sleep(5);
            typeKeys("<esc>");
            sleep(5);
            typeKeys("<esc>");
            sleep(5);

// Close file
            writer.menuItem("File->Close").select();
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

}
