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



package org.openoffice.setup.Util;

import java.util.Vector;

public class LogManager {

    static private Vector logfile; /* collects all logging information during installation */
    static private Vector commandsLogFile;  /* collects all system commands information during installation */
    static private Vector saveLogFile;  /* contains the content of the saved log file */
    static private Vector modulesLogFile;

    private LogManager() {
    }

    static public Vector getSaveLogfile() {
        return saveLogFile;
    }

    static public void addLogfileComment(String text) {
        logfile.add(text);
    }

    static public void addCommandsLogfileComment(String text) {
        commandsLogFile.add(text);
    }

    static public void addModulesLogfileComment(String text) {
        modulesLogFile.add(text);
    }

    static public void setCommandsHeaderLine(String text) {
        String line = "*************************************";
        commandsLogFile.add("");
        commandsLogFile.add(line);
        commandsLogFile.add("<b>" + text + "</b>");
        commandsLogFile.add(line);
        commandsLogFile.add("");
    }

    static public void setModulesLogFileHeaderLine(String text) {
        String line = "*************************************";
        modulesLogFile.add(line);
        modulesLogFile.add(text);
        modulesLogFile.add(line);
    }

    static public String publishLogfileContent(String text, String separatorline) {
        for (int i = 0; i < logfile.size(); i++) {
            text = text + logfile.get(i) + "<br>";
        }

        if ( ! logfile.isEmpty() ) {
            text = text + separatorline + "<br>";
        }

        return text;
    }

    static public String publishCommandsLogfileContent(String text) {
        for (int i = 0; i < commandsLogFile.size(); i++) {
            text = text + commandsLogFile.get(i) + "<br>";
        }

        return text;
    }

    static public Vector getModulesLogFile() {
        return modulesLogFile;
    }

    static {
        logfile = new Vector();
        commandsLogFile = new Vector();
        saveLogFile = new Vector();
        modulesLogFile = new Vector();
    }

}
