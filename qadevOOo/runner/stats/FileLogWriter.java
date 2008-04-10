/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FileLogWriter.java,v $
 * $Revision: 1.5 $
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

import java.io.FileWriter;
import java.io.IOException;
import share.LogWriter;

import java.io.PrintWriter;
import java.io.OutputStreamWriter;
import java.text.DecimalFormat;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.HashMap;
import java.util.Iterator;

public class FileLogWriter extends PrintWriter implements LogWriter {


    HashMap mFileWriters = null;
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
                mFileWriters = new HashMap();
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
                    Iterator iter = mFileWriters.values().iterator();
                    while(iter.hasNext()){
                        fw = (FileWriter) iter.next();
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
        entry.UserDefinedParams.put("Watcher", (share.Watcher) watcher);
    }

}
