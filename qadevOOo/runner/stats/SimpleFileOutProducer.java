/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SimpleFileOutProducer.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:29:39 $
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

import share.LogWriter;
import java.io.File;
import java.io.FileWriter;

public class SimpleFileOutProducer implements LogWriter {


    /** initialization, here a filename might be given
     * or a dbUrL
     */
    public boolean initialize(share.DescEntry entry, boolean active) {
        return true;
    }

    /** Method to print
     */
    public void println(String msg) {

    }

    /** will mostly be used by outproducers to sum up
     * the information, maybe write them to a db
     */
    public boolean summary(share.DescEntry entry) {
        try {
            String outpath = (String) entry.UserDefinedParams.get("OutputPath");
            if (outpath==null) {
                System.out.println("## Parameter OutputPath isn't defined using default");
                return summary_default(entry);
            }
            String FileName = entry.longName + ".out";
            if (!entry.EntryType.equals("component")) {
                FileName = entry.longName.substring(0,
                                entry.longName.indexOf(":")) + ".out";
            }
            util.utils.make_Directories("",outpath);
            File outputFile = new File(outpath, FileName);
            FileWriter out = new FileWriter(outputFile.toString(),true);
            String ls = System.getProperty("line.separator");
            String date = new java.util.Date().toString();
            String header = "***** State for "+entry.longName+"( "+ date +" ) ******";
            out.write(header+ls);
            if (entry.hasErrorMsg) {
                out.write(entry.ErrorMsg+ls);
                out.write("Whole "+entry.EntryType+": "+entry.State+ls);
            } else {
                out.write("Whole "+entry.EntryType+": "+entry.State+ls);
            }
            String bottom="";
            for (int i=0;i<header.length();i++) {
                bottom += "*";
            }
            out.write(bottom+ls);
            out.write(""+ls);
            out.close();
        } catch (java.io.IOException e) {

        }
        return true;
    }

    public boolean summary_default(share.DescEntry entry) {
        String header = "***** State for "+entry.longName+" ******";
        System.out.println(header);
        if (entry.hasErrorMsg) {
            System.out.println(entry.ErrorMsg);
            System.out.println("Whole "+entry.EntryType+": "+entry.State);
        } else {
            System.out.println("Whole "+entry.EntryType+": "+entry.State);
        }
        for (int i=0;i<header.length();i++) {
            System.out.print("*");
        }
        System.out.println("");
        return true;
    }

    public Object getWatcher() {
        return null;
    }

    public void setWatcher(Object watcher) {
    }

}
