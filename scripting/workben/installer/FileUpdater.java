/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package installer;

import java.io.*;
import javax.swing.JLabel;

public class FileUpdater {




    public static boolean updateScriptXLC(String installPath, JLabel statusLabel) {

        File in_file = null;
        File out_file = null;
        FileWriter out = null;
        int count = 0;

        try {
            in_file = new File(installPath + File.separator + "user" + File.separator +
                               "basic" + File.separator + "script.xlc");

            String[] xmlArray = new String[50];

            try {
                BufferedReader reader = new BufferedReader(new FileReader(in_file));
                count = -1;

                for (String s = reader.readLine(); s != null;
                     s = reader.readLine()) { //</oor:node>
                    count = count + 1;
                    xmlArray[count] = s;
                }

                reader.close();
            } catch (IOException ioe) {
                String message = "Error reading script.xlc, please view SFrameworkInstall.log.";
                System.out.println(message);
                ioe.printStackTrace();
                statusLabel.setText(message);
                return false;
            }

            in_file.delete();

            out_file = new File(installPath + File.separator + "user" + File.separator +
                                "basic" + File.separator + "script.xlc");
            out_file.createNewFile();
            out = new FileWriter(out_file);

            //split the string into a string array with one line of xml in each element
            for (int i = 0; i < count + 1; i++) {
                out.write(xmlArray[i] + "\n");

                if ((xmlArray[i].indexOf("<library:libraries xmlns:library") != -1)
                    && (xmlArray[i + 1].indexOf("ScriptBindingLibrary") == -1)) {
                    String opSys = System.getProperty("os.name");

                    if (opSys.indexOf("Windows") != -1) {
                        out.write(" <library:library library:name=\"ScriptBindingLibrary\" library:link=\"true\"/>\n");
                    } else {
                        out.write(" <library:library library:name=\"ScriptBindingLibrary\" xlink:href=\"file://"
                                  + installPath +
                                  "/share/basic/ScriptBindingLibrary/script.xlb/\" xlink:type=\"simple\" library:link=\"true\"/>\n");
                    }
                }
            }
        } catch (Exception e) {
            String message =
                "\nError updating script.xlc, please view SFrameworkInstall.log.";
            System.out.println(message);
            e.printStackTrace();
            statusLabel.setText(message);
            return false;
        } finally {
            try {
                out.close();
            } catch (Exception e) {
                System.out.println("Update Script.xlc Failed, please view SFrameworkInstall.log.");
                e.printStackTrace();
                System.err.println(e);
            }
        }

        return true;
    }// updateScriptXLC


    public static boolean updateDialogXLC(String installPath, JLabel statusLabel) {
        File in_file = null;
        File out_file = null;
        FileWriter out = null;
        int count = 0;

        try {
            in_file = new File(installPath + File.separator + "user" + File.separator +
                               "basic" + File.separator + "dialog.xlc");

            String[] xmlArray = new String[50];

            try {
                BufferedReader reader = new BufferedReader(new FileReader(in_file));
                count = -1;

                for (String s = reader.readLine(); s != null; s = reader.readLine()) {
                    count = count + 1;
                    xmlArray[count] = s;
                }

                reader.close();
            } catch (IOException ioe) {

                String message =
                    "\nError reading dialog.xlc, please view SFrameworkInstall.log.";
                System.out.println(message);
                statusLabel.setText(message);
                return false;
            }

            in_file.delete();

            out_file = new File(installPath + File.separator + "user" + File.separator +
                                "basic" + File.separator + "dialog.xlc");
            out_file.createNewFile();

            out = new FileWriter(out_file);

            //split the string into a string array with one line of xml in each element
            for (int i = 0; i < count + 1; i++) {
                out.write(xmlArray[i] + "\n");

                if ((xmlArray[i].indexOf("<library:libraries xmlns:library") != -1)
                    && (xmlArray[i + 1].indexOf("ScriptBindingLibrary") == -1)) {
                    String opSys = System.getProperty("os.name");

                    if (opSys.indexOf("Windows") != -1) {
                        out.write(" <library:library library:name=\"ScriptBindingLibrary\" library:link=\"true\"/>\n");
                    } else {
                        out.write(" <library:library library:name=\"ScriptBindingLibrary\" xlink:href=\"file://"
                                  + installPath +
                                  "/share/basic/ScriptBindingLibrary/dialog.xlb/\" xlink:type=\"simple\" library:link=\"true\"/>\n");
                    }
                }
            }
        } catch (Exception e) {
            String message =
                "\nError updating dialog.xlc, please view SFrameworkInstall.log.";
            System.out.println(message);
            e.printStackTrace();
            statusLabel.setText(message);
            return false;
        } finally {
            try {
                out.close();
            } catch (Exception e) {
                System.out.println("Update dialog.xlc Failed, please view SFrameworkInstall.log.");
                e.printStackTrace();
                System.err.println(e);
            }
        }

        return true;
    }// updateScriptXLC


}
