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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import org.openoffice.test.common.FileUtil;

import testlib.gui.CalcUtil;

public class GenerateReports {
    static public void computeResults(String resultDir) {
        try {
            Map<String, ArrayList<String>> map = new HashMap<String, ArrayList<String>>();
            BufferedReader in = new BufferedReader(new FileReader(resultDir));
            String line = in.readLine();
            String testScenario;
            String testResult;

            while ((line = in.readLine()) != null) {
                String temp = line.substring(9);
                testScenario = temp.substring(0, temp.indexOf(":"));
                testResult = temp.substring(temp.indexOf(":") + 2);
                System.out.println(testScenario + " " + testResult);
                if (map.containsKey(testScenario)) {
                    map.get(testScenario).add(testResult);
                } else {
                    ArrayList<String> array = new ArrayList<String>();
                    array.add(testResult);
                    map.put(testScenario, array);
                }
                // line = in.readLine();
            }

            Iterator iter = map.keySet().iterator();
            while (iter.hasNext()) {
                String key = (String) iter.next();
                ArrayList<String> val = map.get(key);

                if (val.size() < 8) {
                    continue;
                }

                // Remove the first round result;
                val.remove(0);
                val.trimToSize();

                // Find the min and max value;
                Iterator iter1 = val.listIterator();
                int min_index = 0, max_index = 0, curr_index = 0;
                long min, max, curr_value;
                min = max = Long.parseLong((String) iter1.next());
                while (iter1.hasNext()) {
                    curr_index++;
                    curr_value = Long.parseLong((String) iter1.next());
                    if (curr_value <= min) {
                        min = curr_value;
                        min_index = curr_index;
                    }

                    if (curr_value > max) {
                        max = curr_value;
                        max_index = curr_index;
                    }
                }

                if (min_index > max_index) {
                    val.remove(min_index);
                    val.remove(max_index);
                } else {
                    val.remove(max_index);
                    val.remove(min_index);
                }

                System.out.println("Map removed value: ");
                System.out.println("Key: " + key + " ;Value: " + map.get(key));

                // Compute the average and standard deviation value of the 5
                // left round result
                Iterator iter2 = val.listIterator();
                double average = 0;
                long sum = 0;
                while (iter2.hasNext()) {
                    // System.out.println("Result: " + (String)iter2.next());
                    sum += Long.parseLong((String) iter2.next());
                }

                average = sum / 5;

                Iterator iter3 = val.listIterator();
                double stdev = 0;
                sum = 0;
                while (iter3.hasNext()) {
                    long curr_result = Long.parseLong((String) iter3.next());
                    sum += (curr_result - average) * (curr_result - average);
                }

                average = average / 1000;
                stdev = Math.sqrt(sum / 4) / 1000;

                val.add(Double.toString(average));
                val.add(Double.toString(stdev));

                System.out.println(val.size());
            }

            String resultOdsFile = new File(System.getProperty("testspace", "../testspace")).getAbsolutePath() + "/output/output_start.ods";

            writeResultToFile(map, resultOdsFile);

        } catch (FileNotFoundException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

    }

    static void writeResultToFile(Map<String, ArrayList<String>> m, String report_dir) {
        // initApp();
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(report_dir);
        sleep(10);

        CalcUtil.selectRange("Spread.F1");
        calc.menuItem("Insert->Columns").select();
        // SC_InsertColumnsRowsdialog.ok();
        sleep(1);

        // fillReport(m, "Spread.F13", "New Document Result");
        // fillReport(m, "Spread.F15", "New Presentation Result");
        // fillReport(m, "Spread.F17", "New Spreadsheet Result");
        fillReport(m, "Spread.F3", "Plain ODT Load Show Result");
        fillReport(m, "Spread.F4", "Complex ODT Load Show Result");
        fillReport(m, "Spread.F5", "Plain ODT Load Finish Result");
        fillReport(m, "Spread.F6", "Complex ODT Load Finish Result");
        fillReport(m, "Spread.F7", "Plain DOC Load Show Result");
        fillReport(m, "Spread.F8", "Complex DOC Load Show Result");
        fillReport(m, "Spread.F9", "Plain DOC Load Finish Result");
        fillReport(m, "Spread.F10", "Complex DOC Load Finish Result");
        fillReport(m, "Spread.F11", "Plain ODP Load Show Result");
        fillReport(m, "Spread.F12", "Complex ODP Load Show Result");
        fillReport(m, "Spread.F13", "Plain ODP Load Finish Result");
        fillReport(m, "Spread.F14", "Complex ODP Load Finish Result");
        fillReport(m, "Spread.F15", "Plain PPT Load Show Result");
        fillReport(m, "Spread.F16", "Complex PPT Load Show Result");
        fillReport(m, "Spread.F17", "Plain PPT Load Finish Result");
        fillReport(m, "Spread.F18", "Complex PPT Load Finish Result");
        fillReport(m, "Spread.F19", "Plain ODS Load Show Result");
        fillReport(m, "Spread.F20", "Complex ODS Load Show Result");
        fillReport(m, "Spread.F21", "Plain XLS Load Show Result");
        fillReport(m, "Spread.F22", "Complex XLS Load Show Result");
        fillReport(m, "Spread.F23", "Plain ODT Save Result");
        fillReport(m, "Spread.F24", "Complex ODT Save Result");
        fillReport(m, "Spread.F25", "Plain DOC Save Result");
        fillReport(m, "Spread.F26", "Complex DOC Save Result");
        fillReport(m, "Spread.F27", "Plain ODP Save Result");
        fillReport(m, "Spread.F28", "Complex ODP Save Result");
        fillReport(m, "Spread.F29", "Plain PPT Save Result");
        fillReport(m, "Spread.F30", "Complex PPT Save Result");
        fillReport(m, "Spread.F31", "Plain ODS Save Result");
        fillReport(m, "Spread.F32", "Complex ODS Save Result");
        fillReport(m, "Spread.F33", "Plain XLS Save Result");
        fillReport(m, "Spread.F34", "Complex XLS Save Result");
        // fillReport(m, "Spread.F51",
        // "SD-SShow Complex odp Slider Show Result");

        // Save the text document
        calc.menuItem("File->Save As...").select();
        String saveTo = new File(System.getProperty("testspace", "../testspace")).getAbsolutePath() + "/output/output_start_1.ods";
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        // if (dialog("33388").exists(3))
        // dialog("33388").ok();
        sleep(2);
        calc.menuItem("File->Exit").select();

    }

    static void fillReport(Map<String, ArrayList<String>> m, String cell, String scenario) {
        ArrayList<String> raw_data = m.get(scenario);

        // Iterator iter1 = raw_data.listIterator();
        // while(iter1.hasNext())
        // {
        // System.out.println(iter1.next());
        // }

        // NumberFormat nbf = NumberFormat.getInstance();
        // nbf.setMinimumFractionDigits(2);
        // nbf.setMaximumFractionDigits(2);
        String result;
        if (raw_data.size() == 1) {
            System.out.println("alsdjf");
            result = raw_data.get(0);
        } else {
            // result = " " + nbf.format(raw_data.get(raw_data.size() - 2)) +
            // "/ " + nbf.format(raw_data.get(raw_data.size() - 1));
            result = new java.text.DecimalFormat("0.00").format(Double.parseDouble(raw_data.get(raw_data.size() - 2))).toString() + "/ "
                    + new java.text.DecimalFormat("0.00").format(Double.parseDouble(raw_data.get(raw_data.size() - 1))).toString();
            // result = raw_data.get(raw_data.size() - 2) + "/ " +
            // raw_data.get(raw_data.size() - 1);
        }
        CalcUtil.selectRange(cell);
        typeKeys(result + "<enter>");
    }

    static void printMap(Map<String, ArrayList<String>> m) {
        Iterator iter = m.keySet().iterator();
        while (iter.hasNext()) {
            String key = (String) iter.next();
            System.out.println("Key: " + key + "; Value: " + m.get(key));
        }
    }
}
