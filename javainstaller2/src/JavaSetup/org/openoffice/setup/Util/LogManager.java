/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: LogManager.java,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
