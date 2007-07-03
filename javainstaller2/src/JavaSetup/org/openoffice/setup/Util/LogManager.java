/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LogManager.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-03 12:02:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
