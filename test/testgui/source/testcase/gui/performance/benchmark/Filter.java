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

package testcase.gui.performance.benchmark;

import static org.openoffice.test.vcl.Tester.*;
import static testlib.gui.AppUtil.*;
import static testlib.gui.UIMap.*;

import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Timer;
import java.util.TimerTask;

import org.junit.Before;
import org.junit.Test;
import org.openoffice.test.OpenOffice;
import org.openoffice.test.common.SystemUtil;
import org.openoffice.test.common.Testspace;

public class Filter {
    // @Rule

    public String counterOutput = null;
    private String pid = null;
    private Timer timer = new Timer();

    @Before
    public void setUp() throws FileNotFoundException {

        OpenOffice.killAll();
        app.start();
        String processSoffice = null;
        if (SystemUtil.isWindows()) {
            processSoffice = ".*soffice\\.exe.*";
        } else {
            processSoffice = ".*soffice\\.bin.*";
        }
        HashMap<String, Object> proccessInfo = SystemUtil.findProcess(processSoffice);
        pid = (String) proccessInfo.get("pid");
    }

    @Test
    public void pvtFilter() {
        String counterOutput = Testspace.getPath("output/output_perfmon");
        String pvt_result_path = Testspace.getPath("output/pvt_filter_results.txt");
        Testspace.prepareData("pvt_benchmark/output_start.ods", "output/output_start.ods");

        try {
            if (SystemUtil.isWindows()) {
                String counterLists = Testspace.getPath("data/pvt_benchmark/perfmon/counterlist.txt");
                String createCounters = Testspace.getPath("data/pvt_benchmark/perfmon/createCounters.bat");

                Testspace.prepareData("pvt_benchmark/perfmon/counterlist.txt");
                Testspace.prepareData("pvt_benchmark/perfmon/createCounters.bat");

                String[] cmdargs_start = { "cmd.exe", "/C", "start", createCounters, counterOutput, counterLists };
                java.lang.Runtime.getRuntime().exec(cmdargs_start);
                sleep(5);
            } else {
                final FileWriter counterOut = new FileWriter(counterOutput);
                counterOut.write("Time,Memory(KB),CPU(%)");

                timer.schedule(new TimerTask() {
                    @Override
                    public void run() {
                        HashMap<String, Object> perfData = SystemUtil.getProcessPerfData(pid);
                        String record = System.currentTimeMillis() + "," + perfData.get("rss") + "," + perfData.get("pcpu");
                        try {
                            counterOut.write(record + System.getProperty("line.separator"));
                            counterOut.flush();
                        } catch (IOException e) {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                        }
                    }
                }, 0, 1000);
            }

            FileWriter out = new FileWriter(pvt_result_path);
            out.write("Test Start: " + System.getProperty("line.separator"));

            HashMap<String, Object> perfData = SystemUtil.getProcessPerfData(pid);

            for (int i = 0; i < 8; i++) {
                System.out.println("This is the " + i + " time");
                // out.write("New Document Result: " + perfNew("Text Document")
                // + System.getProperty("line.separator"));
                // out.flush();
                // out.write("New Presentation Result: " +
                // perfNew("Presentation") +
                // System.getProperty("line.separator"));
                // out.flush();
                // out.write("New Spreadsheet Result: " + perfNew("Spreadsheet")
                // + System.getProperty("line.separator"));
                // out.flush();

                out.write(getTime() + "," + "Plain ODT Load Show Result: " + perfLoadShow("pvt_benchmark/sw_plain_120p_odf1.2.odt") + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Plain DOC Load Show Result: " + perfLoadShow("pvt_benchmark/sw_plain_120p.doc") + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Complex ODT Load Show Result: " + perfLoadShow("pvt_benchmark/sw_complex_100p_odf1.2.odt") + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Complex DOC Load Show Result: " + perfLoadShow("pvt_benchmark/sw_complex_100p.doc") + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Plain ODP Load Show Result: " + perfLoadShow("pvt_benchmark/sd_plain_50p_odf1.2.odp") + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Plain PPT Load Show Result: " + perfLoadShow("pvt_benchmark/sd_plain_50p.ppt") + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Complex ODP Load Show Result: " + perfLoadShow("pvt_benchmark/sd_complex_51p_odf1.2.odp") + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Complex PPT Load Show Result: " + perfLoadShow("pvt_benchmark/sd_complex_51p.ppt") + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Plain ODS Load Show Result: " + perfLoadShow("pvt_benchmark/sc_plain_4sh_5kcell_new_odf1.2.ods")
                        + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Plain XLS Load Show Result: " + perfLoadShow("pvt_benchmark/sc_plain_4sh_5kcell.xls") + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Complex ODS Load Show Result: " + perfLoadShow("pvt_benchmark/sc_complex_13sh_4kcell_new_odf1.2.ods")
                        + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Complex XLS Load Show Result: " + perfLoadShow("pvt_benchmark/sc_complex_13sh_4kcell.xls") + System.getProperty("line.separator"));
                out.flush();

                out.write(getTime() + "," + "Plain ODT Load Finish Result: " + perfLoadFinish("pvt_benchmark/sw_plain_120p_odf1.2.odt", 100) + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Plain DOC Load Finish Result: " + perfLoadFinish("pvt_benchmark/sw_plain_120p.doc", 100) + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Complex ODT Load Finish Result: " + perfLoadFinish("pvt_benchmark/sw_complex_100p_odf1.2.odt", 100)
                        + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Complex DOC Load Finish Result: " + perfLoadFinish("pvt_benchmark/sw_complex_100p.doc", 95) + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Plain ODP Load Finish Result: " + perfLoadFinish("pvt_benchmark/sd_plain_50p_odf1.2.odp", 50) + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Plain PPT Load Finish Result: " + perfLoadFinish("pvt_benchmark/sd_plain_50p.ppt", 50) + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Complex ODP Load Finish Result: " + perfLoadFinish("pvt_benchmark/sd_complex_51p_odf1.2.odp", 51)
                        + System.getProperty("line.separator"));
                out.flush();
                sleep(5);
                out.write(getTime() + "," + "Complex PPT Load Finish Result: " + perfLoadFinish("pvt_benchmark/sd_complex_51p.ppt", 51) + System.getProperty("line.separator"));
                out.flush();

                out.write(getTime() + "," + "Plain ODT Save Result: " + perfSave("pvt_benchmark/sw_plain_120p_odf1.2.odt") + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Plain DOC Save Result: " + perfSave("pvt_benchmark/sw_plain_120p.doc") + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Complex ODT Save Result: " + perfSave("pvt_benchmark/sw_complex_100p_odf1.2.odt") + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Complex DOC Save Result: " + perfSave("pvt_benchmark/sw_complex_100p.doc") + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Plain ODP Save Result: " + perfSave("pvt_benchmark/sd_plain_50p_odf1.2.odp") + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Plain PPT Save Result: " + perfSave("pvt_benchmark/sd_plain_50p.ppt") + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Complex ODP Save Result: " + perfSave("pvt_benchmark/sd_complex_51p_odf1.2.odp") + System.getProperty("line.separator"));
                out.flush();
                sleep(5);
                out.write(getTime() + "," + "Complex PPT Save Result: " + perfSave("pvt_benchmark/sd_complex_51p.ppt") + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Plain ODS Save Result: " + perfSave("pvt_benchmark/sc_plain_4sh_5kcell_new_odf1.2.ods") + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Plain XLS Save Result: " + perfSave("pvt_benchmark/sc_plain_4sh_5kcell.xls") + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Complex ODS Save Result: " + perfSave("pvt_benchmark/sc_complex_13sh_4kcell_new_odf1.2.ods") + System.getProperty("line.separator"));
                out.flush();
                out.write(getTime() + "," + "Complex XLS Save Result: " + perfSave("pvt_benchmark/sc_complex_13sh_4kcell.xls") + System.getProperty("line.separator"));
                out.flush();
            }
            out.close();
            if (SystemUtil.isWindows()) {
                String stopCounters = Testspace.getPath("data/pvt_benchmark/perfmon/stopCounters.bat");
                Testspace.prepareData("pvt_benchmark/perfmon/stopCounters.bat");
                String[] cmdargs_end = { "cmd.exe", "/C", "start", stopCounters };
                java.lang.Runtime.getRuntime().exec(cmdargs_end);
            } else {
                timer.cancel();
            }

        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        GenerateReports genReport = new GenerateReports();
        GenerateReports.computeResults(pvt_result_path);

    }

    public String getTime() {

        Date d = new Date();
        SimpleDateFormat sdf = new SimpleDateFormat("HH:mm:ss");
        String date = sdf.format(d);
        return date;
    }

    public long perfNew(String fileType) {
        System.out.println("New " + fileType);
        startcenter.menuItem("File->New->" + fileType).select();
        // startcenter.menuItem("File->New->Spreadsheet").select();
        long tr = System.currentTimeMillis();
        // System.out.println("1: " + tr);
        while (true) {
            if (fileType.equals("Text Document")) {
                if (writer.exists()) {
                    break;
                }
            }

            // else{
            if (fileType.equals("Presentation")) {
                // System.out.println("2: " + System.currentTimeMillis());
                if (impress.exists()) {
                    // System.out.println("3: " + System.currentTimeMillis());
                    break;
                }
            }
            // else
            // {
            if (calc.exists()) {
                break;
            }
            // }
            // }

        }
        // System.out.println("4: " + System.currentTimeMillis());
        tr = System.currentTimeMillis() - tr;

        if (fileType.equals("Text Document")) {
            writer.menuItem("File->Close").select();
        }

        if (fileType.equals("Presentation")) {
            impress.menuItem("File->Close").select();
        }

        if (fileType.equals("Spreadsheet")) {
            calc.menuItem("File->Close").select();
        }

        return tr;
    }

    public long perfLoadShow(String fileName) {
        System.out.println(fileName + " Load Show");
        long tr = 0;
        String file = Testspace.prepareData(fileName);
        startcenter.menuItem("File->Open...").select();
        FilePicker_Path.setText(file);
        FilePicker_Open.click();
        // submitOpenDlg(file);
        tr = System.currentTimeMillis();
        while (true) {
            if (fileName.contains("odt") || fileName.contains("doc") || fileName.contains("docx")) {
                if (writer.exists()) {
                    // writer.menuItem("File->Close").select();
                    break;
                }
            }

            if (fileName.contains("odp") || fileName.contains("ppt") || fileName.contains("pptx")) {
                if (impress.exists()) {
                    // impress.menuItem("File->Close").select();
                    break;
                }
            }

            if (fileName.contains("ods") || fileName.contains("xls") || fileName.contains("xlsx")) {
                if (calc.exists()) {
                    // calc.menuItem("File->Close").select();
                    break;
                }
            }
        }
        tr = System.currentTimeMillis() - tr;
        sleep(2);

        if (fileName.contains("odt") || fileName.contains("doc") || fileName.contains("docx")) {
            writer.menuItem("File->Close").select();
        }

        if (fileName.contains("odp") || fileName.contains("ppt") || fileName.contains("pptx")) {
            impress.menuItem("File->Close").select();
        }

        if (fileName.contains("ods") || fileName.contains("xls") || fileName.contains("xlsx")) {
            calc.menuItem("File->Close").select();
        }

        return tr;
    }

    public long perfLoadFinish(String fileName, int destPage) {
        System.out.println(fileName + " Load Finish");
        long tr = 0;
        String file = Testspace.prepareData(fileName);
        startcenter.menuItem("File->Open...").select();
        // submitOpenDlg(file);
        FilePicker_Path.setText(file);
        FilePicker_Open.click();
        tr = System.currentTimeMillis();
        while (true) {
            while (!statusbar("FWK_HID_STATUSBAR").exists()) {
                ;
            }
            if (getLoadedPage(fileName) >= destPage) {
                break;
            }
        }
        tr = System.currentTimeMillis() - tr;
        sleep(5);
        // System.out.println("Load Finish Time: " + tr);

        if (fileName.contains("odt") || fileName.contains("doc") || fileName.contains("docx")) {
            writer.menuItem("File->Close").select();
        }

        if (fileName.contains("odp") || fileName.contains("ppt") || fileName.contains("pptx")) {
            impress.menuItem("File->Close").select();
        }

        if (fileName.contains("ods") || fileName.contains("xls") || fileName.contains("xlsx")) {
            calc.menuItem("File->Close").select();
        }
        return tr;
    }

    public static int getLoadedPage(String docName) {
        String tmp = "";
        if (docName.endsWith("odt") || docName.endsWith("doc") || docName.endsWith("docx")) {
            tmp = statusbar("FWK_HID_STATUSBAR").getItemText(0);
        }
        if (docName.endsWith("odp") || docName.endsWith("ppt") || docName.endsWith("pptx")) {
            tmp = statusbar("FWK_HID_STATUSBAR").getItemText(4);
        }
        // System.out.println(tmp);
        String s[] = tmp.split("/");
        // System.out.println(Integer.parseInt(s[1].trim()));
        return Integer.parseInt(s[1].trim());
    }

    public long perfSave(String fileName_i) {
        System.out.println(fileName_i + " Save");
        String fileName = fileName_i.substring(14);
        // System.out.println(fileName);
        long tr = 0;
        sleep(2);
        String file = Testspace.prepareData(fileName_i);
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(file);
        sleep(8);
        if (fileName.startsWith("sw")) {
            if (fileName.startsWith("sw_complex")) {
                typeKeys("<ctrl Home>");
                if (fileName.endsWith("odt")) {
                    // writer.click(180,300);
                    typeKeys("<PageDown>");
                    sleep(2);
                    typeKeys("<PageDown>");
                    sleep(2);
                    typeKeys("<PageDown>");
                    sleep(2);
                    typeKeys("<Enter>");
                    sleep(2);
                    typeKeys("<delete>");
                } else {
                    // writer.click(180,300);
                    // org.vclauto.Tester.typeKeys("<PGDN pgdn pgdn pgdn pgdn>");
                    // org.vclauto.Tester.typeKeys("<DOWN down>");
                    typeKeys("<Enter>");
                    sleep(2);
                    // System.out.println("Enter");
                    typeKeys("<backspace>");
                    // System.out.println("backspace");
                }
            } else {
                // writer.click(180,300);
                // writer.click();
                typeKeys("<enter>");
                typeKeys("<backspace>");
            }
        } else if (fileName.startsWith("sd")) {
            // writer.click(436, 326);
            // presenter.click();
            // sleep(2);
            // org.vclauto.Tester.typeKeys("<Enter>");
            // sleep(1);
            // org.vclauto.Tester.typeKeys("<BACKSPACE>");
            // sleep(1);
            // org.vclauto.Tester.typeKeys("<ESC>");
            // sleep(1);
            // org.vclauto.Tester.typeKeys("<ESC>");
            // System.out.println("SD");
            impress.menuItem("Insert->Slide").select();
            // System.out.println("Insert slide");
            sleep(5);
            impress.menuItem("Edit->Delete Slide").select();
            // System.out.println("Delete slide");
            sleep(5);

        } else {
            // writer.click(100, 220);
            typeKeys("1");
            typeKeys("<BACKSPACE>");
            typeKeys("<Enter>");
            typeKeys("<ESC>");
        }
        sleep(5);

        // typeKeys("<ctrl s>");
        app.dispatch(".uno:Save");
        tr = System.currentTimeMillis();

        int index = 0;
        if (fileName.endsWith("odt") || fileName.endsWith("doc") || fileName.endsWith("docx")) {
            index = 5;
        }
        if (fileName.endsWith("odp") || fileName.endsWith("ppt") || fileName.endsWith("pptx")) {
            index = 2;
        }
        if (fileName.endsWith("ods") || fileName.endsWith("xls") || fileName.endsWith("xlsx")) {
            index = 4;
        }
        while (true) {
            // System.out.println(statusbar("FWK_HID_STATUSBAR").getItemText(index));
            if (statusbar("FWK_HID_STATUSBAR").getItemText(index).equals(" ")) {
                break;
            }
        }
        tr = System.currentTimeMillis() - tr;

        sleep(2);

        if (fileName.contains("odt") || fileName.contains("doc") || fileName.contains("docx")) {
            writer.menuItem("File->Close").select();
        }

        if (fileName.contains("odp") || fileName.contains("ppt") || fileName.contains("pptx")) {
            impress.menuItem("File->Close").select();
        }

        if (fileName.contains("ods") || fileName.contains("xls") || fileName.contains("xlsx")) {
            calc.menuItem("File->Close").select();
        }
        return tr;
    }

}
