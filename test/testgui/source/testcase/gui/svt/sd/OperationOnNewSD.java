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

package testcase.gui.svt.sd;

import static testlib.gui.AppUtil.submitSaveDlg;
import static testlib.gui.UIMap.PresentationWizard;
import static testlib.gui.UIMap.app;
import static testlib.gui.UIMap.startcenter;
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
import org.openoffice.test.common.SystemUtil;
import org.openoffice.test.common.Testspace;

import static org.openoffice.test.common.Testspace.prepareData;
import static org.openoffice.test.vcl.Tester.*;

import testlib.gui.Log;

public class OperationOnNewSD {
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
        result = new PrintStream(new FileOutputStream(Testspace.getFile("output/svt_sd_new.csv")));
        HashMap<String, Object> proccessInfo = SystemUtil.findProcess(".*(soffice\\.bin|soffice.*-env).*");
        pid = (String)proccessInfo.get("pid");
        result.println("Iterator,Time,Memory(KB),CPU(%)");
        LOG.info("Result will be saved to " + Testspace.getPath("output/svt_sd_new.csv"));
    }

    @After
    public void tearDown() throws Exception {
        app.close();
        result.close();
    }

    @Test
    public void operationOnNewSW() throws Exception {
        String externalFile = prepareData("svt/sd_plain_50p.odp");
        String pic = prepareData("svt/Sunset.jpg");
        for(int i = 0; i < 1000; i++)
        {
//          Create a new SD
            app.dispatch("private:factory/simpress?slot=6686");
            PresentationWizard.ok();
            sleep(2);

//          Create a new slide
            impress.menuItem("Insert->Slide").select();
            sleep(2);

//          Insert a table
            app.dispatch(".uno:InsertTable", 3);
            sleep(2);
            NumberofCol.setText("5");
            NumberofRow.setText("4");
            sleep(2);
            InsertTable.ok();
            impress.typeKeys("<enter>");
            impress.typeKeys("2");
            impress.typeKeys("<right>");
            impress.typeKeys("2");
            impress.typeKeys("<right>");
            impress.typeKeys("<ctrl end>");
            impress.typeKeys("2");
            sleep(2);

//          Insert a picture
            impress.menuItem("Insert->Slide").select();
            sleep(2);
            impress.menuItem("Insert->Picture->From File...").select();
            sleep(2);
            FilePicker_Path.setText(pic);
            sleep(1);
            FilePicker_Open.click();
            sleep(5);
            impress.typeKeys("<esc>");
            sleep(2);

//          Insert Slides from External Sample files
            impress.menuItem("Insert->File...").select();
            sleep(2);
            FilePicker_Path.setText(externalFile);
            FilePicker_Open.click();
            sleep(2);
            InsertSlideObjects.ok();
            sleep(2);
            ActiveMsgBox.yes();
            sleep(20);

//          Slide Screen Show Settings
            impress.menuItem("Slide Show->Slide Transition...").select();
            sleep(2);
            ImpressSlideTransitions.select("Uncover Up");
            sleep(2);
            SlideShowSpeed.select("Slow");
            sleep(5);
            SlideAutoAfter.check();
            sleep(2);
            ApplyToAllSlides.click();
            sleep(2);
            impress.menuItem("Slide Show->Slide Show").select();
            sleep(70);
            impress.typeKeys("<esc>");
            sleep(5);

//          Save file and close
            String saveTo = "tempSD" + i + ".odp";
            impress.menuItem("File->Save As...").select();
            FileUtil.deleteFile(saveTo);
            submitSaveDlg(saveTo);
            if(ActiveMsgBox.exists())
            {
                ActiveMsgBox.yes();
                sleep(2);
            }
            impress.menuItem("File->Close").select();

            HashMap<String, Object> perfData = SystemUtil.getProcessPerfData(pid);
            String record = i + "," + System.currentTimeMillis() + "," + perfData.get("rss") + "," + perfData.get("pcpu");
            LOG.info("Record: " + record);
            result.println(record);
            result.flush();

            sleep(3);
        }
    }

}
