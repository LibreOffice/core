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
