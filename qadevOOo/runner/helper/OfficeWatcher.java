/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OfficeWatcher.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-01-19 14:23:56 $
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

package helper;

import java.lang.Thread;
import lib.TestParameters;
import share.LogWriter;

import com.sun.star.frame.XComponentLoader;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XComponent;
import com.sun.star.beans.PropertyValue;
import java.util.StringTokenizer;

public class OfficeWatcher extends Thread implements share.Watcher {

    public boolean finish;

    TestParameters params;
    int StoredPing = 0;

    /** Creates new OfficeWatcher */
    public OfficeWatcher(TestParameters param) {
        finish = false;
        this.params = param;
    }

    /**
     * pings the office watcher to check for changes
     */
    public void ping() {
        try{
            StoredPing ++;
        } catch (Exception e){
            StoredPing=0;
        }
    }

    /**
     * returns the amount of pings
     */
    public int getPing(){
        return StoredPing;
    }

    public void run() {
        boolean isDone = false;
        ProcessHandler ph = (ProcessHandler) params.get("AppProvider");
        int timeOut = params.getInt("TimeOut");
        if (ph == null) {
            isDone = true;
        }
        while (!isDone) {
            timeOut = params.getInt("TimeOut");
            int previous = StoredPing;
            shortWait(timeOut==0?30000:timeOut);
            // a timeout with value 0 lets watcher not react.
            if ((StoredPing == previous) && timeOut != 0){
                isDone = true;
            }
            // execute in case the watcher is not needed anymore
            if (finish) {
                return;
            }
        }
        if (ph !=null) {
            System.out.println("OfficeWatcher: the Office is idle for " + timeOut/1000
                        + " seconds, it probably hangs and is killed NOW.");
            String AppKillCommand = (String) params.get ("AppKillCommand");
            if (AppKillCommand != null)
            {
                StringTokenizer aKillCommandToken = new StringTokenizer( AppKillCommand,";" );
                while (aKillCommandToken.hasMoreTokens())
                {
                    String sKillCommand = aKillCommandToken.nextToken();

                    System.out.println("User defined an application to destroy the started process.");
                    System.out.println("Trying to execute: "+sKillCommand);
                    try
                    {
                        Runtime.getRuntime().exec(sKillCommand);
                        shortWait(2000);
                    }
                    catch (java.io.IOException e)
                    {
                        e.printStackTrace ();
                    }
                }
            }
            ph.kill();
        }
        shortWait(timeOut==0?30000:timeOut);
    }

    protected void shortWait(int timeOut) {
        try {
            this.sleep(timeOut);
        } catch (java.lang.InterruptedException ie) {}
    }


}
