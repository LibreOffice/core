/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SimpleLogWriter.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-01-19 14:24:12 $
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

import java.io.PrintWriter;
import java.io.OutputStreamWriter;
import java.text.DecimalFormat;
import java.util.Calendar;
import java.util.GregorianCalendar;

public class SimpleLogWriter extends PrintWriter implements LogWriter {

    boolean m_bLogging = false;
    share.DescEntry entry = null;
    share.Watcher ow = null;

    public SimpleLogWriter() {
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

    public boolean initialize(share.DescEntry _entry, boolean _bLogging) {
        m_bLogging = _bLogging;
        entry = _entry;

        return true;
    }

    public void println(String msg) {
        if (entry != null)
        {
            this.ow = (share.Watcher) entry.UserDefinedParams.get("Watcher");

            if (this.ow != null) {
                this.ow.ping();
            }
        }
        if (m_bLogging)
        {
            super.println("LOG> "+msg);
            super.flush();
        }
        // else
        // {
        //     super.println(" ++ " + msg);
        //     super.flush();
        // }
    }

    public boolean summary(share.DescEntry entry) {
        return true;
    }

    public Object getWatcher() {
        return this.ow;
    }

    public void setWatcher(Object watcher) {
        entry.UserDefinedParams.put("Watcher", (share.Watcher) watcher);
    }

}
