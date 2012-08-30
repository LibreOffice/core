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
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.Logger;
import org.openoffice.test.common.SystemUtil;
import org.openoffice.test.common.Testspace;

public class OperationOnNewSW {
    @Rule
    public Logger log = Logger.getLogger(this);

    private PrintStream result = null;

    private String pid = null;

    /**
     * @throws java.lang.Exception
     */
    @Before
    public void setUp() throws Exception {
        app.start(true);
        result = new PrintStream(new FileOutputStream(Testspace.getFile("output/svt_sw_new.csv")));
        HashMap<String, Object> proccessInfo = SystemUtil.findProcess(".*(soffice\\.bin|soffice.*-env).*");
        pid = (String) proccessInfo.get("pid");
        result.println("Iterator,Time,Memory(KB),CPU(%)");
        log.info("Result will be saved to " + Testspace.getPath("output/longrun.csv"));
    }

    @After
    public void tearDown() throws Exception {
        app.close();
        result.close();
    }

    @Test
    public void operationOnNewSW() throws Exception {
        for (int i = 0; i < 1000; i++) {
            startcenter.menuItem("File->New->Text Document").select();

            // Input words (Duplicate Formatting) and set numbering and bullet.
            writer.typeKeys("this is a wonderful world");
            writer.typeKeys("<enter>");
            sleep(3);
            writer.typeKeys("this is a beautiful world");
            app.dispatch(".uno:SelectAll");
            writer.menuItem("Format->Bullets and Numbering...").select();
            sleep(3);
            Bullet.ok();
            writer.typeKeys("<ctrl z");
            sleep(1);
            writer.typeKeys("<ctrl y>");
            sleep(1);
            writer.typeKeys("<ctrl end>");
            writer.typeKeys("<enter>");
            sleep(2);

            // Word Spell Check
            writer.typeKeys("goood");
            sleep(2);
            writer.menuItem("Tools->Spelling and Grammar...").select();
            sleep(2);
            SuggestionList.select(0);
            Change.click();
            sleep(2);
            ActiveMsgBox.no();
            sleep(2);

            // Create Header and Footer
            writer.menuItem("Insert->Header->Default").select();
            sleep(2);
            writer.typeKeys("Header");
            sleep(2);
            writer.menuItem("Insert->Footer->Default").select();
            sleep(2);
            writer.typeKeys("Footer");
            sleep(2);
            writer.typeKeys("<ctrl end>");
            sleep(2);
            writer.typeKeys("<enter>");
            sleep(2);

            // Insert Frame and change anchor
            writer.menuItem("Insert->Frame...").select();
            sleep(2);
            writer_FrameDlg.ok();
            writer.menuItem("Format->Anchor->To Page").select();
            sleep(2);
            writer.menuItem("Format->Anchor->To Paragraph").select();
            sleep(2);
            writer.menuItem("Format->Anchor->To Character").select();
            sleep(2);
            writer.menuItem("Format->Anchor->As Character").select();
            sleep(2);
            writer.typeKeys("<esc>");
            sleep(2);
            writer.typeKeys("<enter>");
            sleep(2);

            // Insert TOC
            for (int j = 0; j < 6; j++) {
                writer.typeKeys(String.valueOf(j + 1));
                writer.typeKeys("<enter>");
            }
            writer.typeKeys("<ctrl home>");
            for (int k = 0; k < 2; k++) {
                writer.typeKeys("<shift down>");
            }
            if (!StyleAndFormattingDlg.exists()) {
                app.dispatch(".uno:DesignerDialog");
                sleep(2);
            }

            StyleAndFormattingList.doubleClick(0.5, 0.25);
            sleep(2);
            writer.typeKeys("<down>");
            writer.typeKeys("<right>");
            writer.typeKeys("<left>");
            for (int k = 0; k < 2; k++) {
                writer.typeKeys("<shift down>");
            }
            StyleAndFormattingList.doubleClick(0.5, 0.3);
            sleep(2);
            writer.typeKeys("<ctrl home>");
            sleep(2);
            writer.menuItem("Insert->Indexes and Tables->Indexes and Tables...").select();
            sleep(2);
            InsertIndexDlg.ok();
            sleep(2);
            writer.typeKeys("<ctrl end>");

            // Save File and close
            String saveTo = "temp" + i + ".odt";
            writer.menuItem("File->Save As...").select();
            FileUtil.deleteFile(saveTo);
            submitSaveDlg(saveTo);
            if (ActiveMsgBox.exists()) {
                ActiveMsgBox.yes();
                sleep(2);
            }

            writer.menuItem("File->Close").select();

            HashMap<String, Object> perfData = SystemUtil.getProcessPerfData(pid);
            String record = i + "," + System.currentTimeMillis() + "," + perfData.get("rss") + "," + perfData.get("pcpu");
            log.info("Record: " + record);
            result.println(record);
            result.flush();

            sleep(3);
        }

    }
}
