/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FileLogWriter.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:28:32 $
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
