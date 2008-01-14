/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OfficeWatcher.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 13:21:04 $
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

import lib.TestParameters;
import java.util.StringTokenizer;

public class OfficeWatcher extends Thread implements share.Watcher {

    public boolean finish;
    TestParameters params;
    int StoredPing = 0;

    /** Creates new OfficeWatcher
     * @param param
     */
    public OfficeWatcher(TestParameters param) {
        finish = false;
        this.params = param;
    }

    /**
     * pings the office watcher to check for changes
     */
    public void ping() {
        try {
            StoredPing++;
        } catch (Exception e) {
            StoredPing = 0;
        }
    }

    /**
     * returns the amount of pings
     * @return returns the amount of pings
     */
    public int getPing() {
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
            shortWait(timeOut == 0 ? 30000 : timeOut);
            // a timeout with value 0 lets watcher not react.
            if ((StoredPing == previous) && timeOut != 0) {
                isDone = true;
            }
            // execute in case the watcher is not needed anymore
            if (finish) {
                return;
            }
        }
        if (ph != null) {
            System.out.println("OfficeWatcher: the Office is idle for " + timeOut / 1000 + " seconds, it probably hangs and is killed NOW.");
            String AppKillCommand = (String) params.get ("AppKillCommand");
            if (AppKillCommand != null) {
                StringTokenizer aKillCommandToken = new StringTokenizer(AppKillCommand, ";");
                while (aKillCommandToken.hasMoreTokens()) {
                    String sKillCommand = aKillCommandToken.nextToken();

                    System.out.println("User defined an application to destroy the started process.");
                    System.out.println("Trying to execute: " + sKillCommand);
                    try {
                        Process myprc = Runtime.getRuntime().exec(sKillCommand);
                        myprc.waitFor();
                    } catch (InterruptedException ex) {
                        ex.printStackTrace();
                    } catch (java.io.IOException e) {
                        e.printStackTrace();
                    }
                }
            }
            ph.kill();
        }
        shortWait(timeOut == 0 ? 30000 : timeOut);
    }

    protected void shortWait(int timeOut) {
        try {
            OfficeWatcher.sleep(timeOut);
        } catch (java.lang.InterruptedException ie) {
        }
    }
}