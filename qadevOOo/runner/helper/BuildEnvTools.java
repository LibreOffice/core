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
package helper;

import java.io.File;
import java.io.PrintWriter;
import lib.TestParameters;
import share.LogWriter;
import util.*;

/**
 * This class support you to execute some shell commands in a build environment. At every call of commands
 * a build environment was created and the commands will be executed.
 *
 */
public class BuildEnvTools {

    private final TestParameters param;
    private final LogWriter log;
    private final boolean mDebug;
    private final String mPlatform;
    private final String mShell;
    private final boolean mCygwin;

    /**
     * This constructor creates an instance of BuildEncTools. It is verifying for all neccesarry
     * parameters in <CODE>TestParameters</CODE> This must be:
     * <ul>
     * <li>OperatingSystem: Fill this parameter with an operating system like unxsols, unxsoli, unxlngi or wntmsci.
     * </li>
     * <li> Shell: Fill this parameter with a shell f.e '/bin/tcsh'
     *      or 'c:\\myShell\\myShell.exe'
     * </li>
     */
    public BuildEnvTools(TestParameters param, LogWriter log) throws ParameterNotFoundException {
        this.param = param;
        this.log = log;
        mDebug = param.getBool(PropertyName.DEBUG_IS_ACTIVE);

        boolean error = false;

        String msg = "\nERROR: the following parameter must be set before executing the test:\n\n";

        mPlatform = (String) param.get(PropertyName.OPERATING_SYSTEM);
        if (mDebug) {
            log.println("### " + mPlatform);
        }
        if (mPlatform == null){
            msg += PropertyName.OPERATING_SYSTEM + "\nFill this parameter with an operating system like unxsols," +
                " unxsoli, unxlngi, unxmacxi or wntmsci.  \n\n";
        }
        if (mPlatform != null &&
            (!mPlatform.equalsIgnoreCase(PropertyName.UNXSOLS)) &&
            (!mPlatform.equalsIgnoreCase(PropertyName.UNXSOLI)) &&
            (!mPlatform.equalsIgnoreCase(PropertyName.UNXLNGI)) &&
            (!mPlatform.equalsIgnoreCase(PropertyName.UNXMACXI))&&
            (!mPlatform.equalsIgnoreCase(PropertyName.WNTMSCI)) ){

            msg += PropertyName.OPERATING_SYSTEM + ":" + mPlatform + "\nFill this parameter with an operating system like unxsols," +
                " unxsoli, unxlngi, unxmacxi or wntmsci.  \n\n";
            error = true;
        }

        mShell = (String) param.get(PropertyName.SHELL);
        if (mShell == null) {
            msg += PropertyName.SHELL + "\nFill this parameter with a shell" +
                "\n\t/bin/tcsh c:\\myShell\\myShell.exe\n\n";
            error = true;
        }

        mCygwin = (param.getBool(PropertyName.CYGWIN));

        if (error) {
            throw new ParameterNotFoundException(msg);
        }
    }

    /**
     * Executes the given commands in OOo-Environment shell.
     * @return the processHandler of the commands
     * @see helper.ProcessHandler
     */
    public ProcessHandler runCommandsInEnvironmentShell(String[] commands, File workDir, int shortWait) {

        final String[] cmdLines = getCmdLinesWithCommand(commands);
        final ProcessHandler pHdl = new ProcessHandler(cmdLines, (PrintWriter) log, workDir, shortWait, param);
        pHdl.runCommand();
        return pHdl;
    }

    public String getSrcRoot() {

        String sSrcRoot = (String) param.get(PropertyName.SRC_ROOT);

        if (sSrcRoot == null) {
            String[] cmdLines = null;
            if (mPlatform.equals(PropertyName.WNTMSCI) && ! mCygwin) {
                cmdLines = new String[]{mShell, "/C", "echo SRC_ROOT=%SRC_ROOT"};
            } else {
                cmdLines = new String[]{mShell, "--login ", "-c ", "echo \"SRC_ROOT=$SRC_ROOT\""};
            }

            final ProcessHandler procHdl = new ProcessHandler(cmdLines, (PrintWriter) log, null, 5000, param);
            procHdl.runCommand();

            if (mDebug) {
                log.println("---> Output of command:");
                log.println(procHdl.getOutputText());
                log.println("<--- Output of command:");
                log.println("---> Error output of command");
                log.println(procHdl.getErrorText());
                log.println("<--- Error output of command");
            }
            final String output = procHdl.getOutputText();
            final String[] outs = output.split("\n");

            for (int i = 0; i < outs.length; i++) {
                final String line = outs[i];
                if (line.startsWith("SRC_ROOT")) {
                    sSrcRoot = getEnvValue(line);
                }
            }
        }
        return sSrcRoot;
    }

    private String[] getCmdLinesWithCommand(String[] commands) {
        String[] cmdLines = null;
        log.println("prepare command for platform " + mPlatform);

        String separator = "";
        if (mPlatform.equals(PropertyName.WNTMSCI)) {
            separator = mCygwin ? ";" : "^";
        } else {
            separator = ";";
        }

        String command = "";
        for (int i = 0; i < commands.length; i++) {
            if (i != 0) {
                command += separator;
            }
            command += commands[i];
        }

        if (mPlatform.equals(PropertyName.WNTMSCI)){
            if (mCygwin){
                String srcRoot = (String) param.get(PropertyName.SRC_ROOT);
                String envSet = "export cyg_src_root=`cygpath '" + srcRoot.replaceAll("\\\\", "\\\\\\\\")+ "'`; source $cyg_src_root/winenv.set.sh;";
                command = envSet + command;
                cmdLines = new String[]{mShell, "--login", "-c", "\"" + command + "\""};
            } else {
            cmdLines = new String[]{mShell, "/C", "\"" + command + "\""};
            }
        } else {
            cmdLines = new String[]{mShell, "-c", command};
        }
        return cmdLines;
    }

    private String getEnvValue(String line) {
        final String[] split = line.split("=");
        return split[1];
    }
}
