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
import javax.swing.*;
public class Register {


    public static boolean register(String path, JLabel statusLabel) {
        String[] packages = {"ooscriptframe.zip", "bshruntime.zip", "jsruntime.zip"};

        try {
            boolean goodResult = false;
            String env[] = new String[1];
            ExecCmd command = new ExecCmd();
            boolean isWindows =
                (System.getProperty("os.name").indexOf("Windows") != -1);

            String progpath = path.concat("program" + File.separator);

            statusLabel.setText("Registering Scripting Framework...");

            // pkgchk Scripting Framework Components
            statusLabel.setText("Registering Scripting Framework Components...");
            System.out.println("Registering Scripting Framework Components...");

            for (int i = 0; i < packages.length; i++) {
                String cmd = "";

                if (!isWindows) {
                    env[0] = "LD_LIBRARY_PATH=" + progpath;

                    goodResult = command.exec("chmod a+x " + progpath + "pkgchk", null);

                    if (goodResult) {
                        cmd = progpath + "pkgchk -s -f " + progpath + packages[i];

                        System.err.println(cmd);
                        goodResult = command.exec(cmd, env);
                    }
                } else {
                    cmd = "\"" + progpath + "pkgchk.exe\" -s -f \"" + progpath +
                          packages[i] + "\"";

                    System.err.println(cmd);
                    goodResult = command.exec(cmd, null);

                }

                if (!goodResult) {
                    System.err.println("\nPkgChk Failed");

                    if (!isWindows)
                        System.err.println("Command: " + cmd + "\n" + env[0]);
                    else
                        System.err.println("Command: \"" + cmd + "\"");

                    statusLabel.setText(
                        "PkgChk Failed, please view SFrameworkInstall.log");

                    return false;
                }
            }

            // updating StarBasic libraries
            statusLabel.setText("Updating StarBasic libraries...");

            if (!FileUpdater.updateScriptXLC(path, statusLabel)) {
                statusLabel.setText("Updating user/basic/script.xlc failed, please view SFrameworkInstall.log");
                return false;
            }

            if (!FileUpdater.updateDialogXLC(path, statusLabel)) {
                statusLabel.setText("Updating user/basic/dialog.xlc failed, please view SFrameworkInstall.log");
                return false;
            }

        } catch (Exception e) {
            String message =
                "\nError installing scripting package, please view SFrameworkInstall.log.";
            System.out.println(message);
            e.printStackTrace();
            statusLabel.setText(message);
            return false;
        }

        return true;
    }// register

}//Register
