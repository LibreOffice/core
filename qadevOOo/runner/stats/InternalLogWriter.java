/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: InternalLogWriter.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:28:46 $
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

