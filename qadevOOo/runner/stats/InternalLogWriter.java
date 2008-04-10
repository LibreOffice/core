/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: InternalLogWriter.java,v $
 * $Revision: 1.6 $
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
package stats;

import java.io.PrintWriter;
import java.io.StringWriter;

/**
 * Write all logs into a java.io.PrintWriter, i.e. a StringBuffer.
 * Log is gathered there.
 */
public class InternalLogWriter extends PrintWriter
                                            implements share.LogWriter {
    /** log active **/
    boolean active;
    /** write all output to a StringBuffer **/
    static StringWriter writer = new StringWriter();

    /**
     * c'*tor
     */
    public InternalLogWriter() {
         super(new PrintWriter(writer));
         active = true;
    }

    /**
     * Initialization.
     * @param entry The description entry.
     * @param active Logging is active.
     * @return True, if initialize worked.
     */
    public boolean initialize(share.DescEntry entry, boolean active) {
        this.active = active;
        return true;
    }

    /**
     * Method to print a line that is added to the StringBuffer.
     * @param msg The message that is printed.
     */
    public void println(String msg) {
        if (active)
            super.println(msg);
    }

    /**
     * Method to print to the StringBuffer.
     * @param msg The message that is printed.
     */
    public void print(String msg) {
        if (active)
            super.print(msg);

    }

    /**
     * Is used to sum up the information.
     * The summary is also added to the StringBuffer.
     * @param entry The description entry.
     * @return True, if a summary could be created.
     */
    public boolean summary(share.DescEntry entry) {
//        linePrefix = "";
        String header = "***** State for "+entry.longName+" ******";
        println(header);
        if (entry.hasErrorMsg) {
            println(entry.ErrorMsg);
            println("Whole "+entry.EntryType+": "+entry.State);
        } else {
            println("Whole "+entry.EntryType+": "+entry.State);
        }
        for (int i=0;i<header.length();i++) {
            print("*");
        }
        println("");
        return true;
    }

    /**
     * Return all the written stuff.
     * @return All that was written to the StringBuffer with the
     * 'println()', 'print()' and 'summarize()' methods.
     * The StringBuffer is emptied afterwards.
     **/
    public String getLog() {
        String message = writer.getBuffer().toString();
        writer = new StringWriter();
        return message;
    }

    public Object getWatcher() {
        return null;
    }

    public void setWatcher(Object watcher) {
    }

}

