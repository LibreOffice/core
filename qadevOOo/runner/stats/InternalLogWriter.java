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

