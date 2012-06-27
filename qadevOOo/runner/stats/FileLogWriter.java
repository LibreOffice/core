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

import java.io.FileWriter;
import java.io.IOException;
import share.LogWriter;

import java.io.PrintWriter;
import java.text.DecimalFormat;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.HashMap;
import java.util.Iterator;

public class FileLogWriter extends PrintWriter implements LogWriter {


    HashMap<String, FileWriter> mFileWriters = null;
    boolean logging = false;
    share.DescEntry entry = null;
    share.Watcher ow = null;

    public FileLogWriter() {
        super(System.out);
        Calendar cal = new GregorianCalendar();
        DecimalFormat dfmt = new DecimalFormat("00");
        super.println("LOG> Log started " +
                    dfmt.format(cal.get(Calendar.DAY_OF_MONTH)) + "." +
                    dfmt.format(cal.get(Calendar.MONTH)) + "." +
                    dfmt.format(cal.get(Calendar.YEAR)) + " - " +
                    dfmt.format(cal.get(Calendar.HOUR_OF_DAY)) + ":" +
                    dfmt.format(cal.get(Calendar.MINUTE)) + ":" +
                    dfmt.format(cal.get(Calendar.SECOND)));
        super.flush();
    }

    public boolean initialize(share.DescEntry entry, boolean logging) {
         this.logging = logging;
         this.entry = entry;
         return true;
    }


    public void addFileLog(String filePath){
        try{
            if(mFileWriters == null)
                mFileWriters = new HashMap<String, FileWriter>();
            mFileWriters.put(filePath, new FileWriter(filePath));
        }catch(IOException e ){
            e.printStackTrace(this);
        }
    }


    public void removeFileLog(String filePath){
        if(filePath != null)
            mFileWriters.remove(filePath);
    }


    public void println(String msg) {

        this.ow = (share.Watcher) entry.UserDefinedParams.get("Watcher");

        if (ow != null) {
            ow.ping();
        }
        if (logging) {

            // logoutput to console
            super.println("LOG> "+msg);
            super.flush();

            //logoutput to file
            if(mFileWriters != null && mFileWriters.size() > 0){
                try{
                    FileWriter fw = null;
                    Iterator<FileWriter> iter = mFileWriters.values().iterator();
                    while(iter.hasNext()){
                        fw = iter.next();
                        fw.write("LOG> " + msg + "\n");
                        fw.flush();
                    }
                }catch(IOException e ){
                    e.printStackTrace(this);
                }
            }
        }
    }

    public boolean summary(share.DescEntry entry) {
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
        return this.ow;
    }

    public void setWatcher(Object watcher) {
        entry.UserDefinedParams.put("Watcher", watcher);
    }

}
