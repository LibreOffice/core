/*************************************************************************
 *
 *  $RCSfile: OfficeWatcher.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2004-07-23 10:43:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

public class OfficeWatcher extends Thread implements share.Watcher {

    public boolean finish;

    TestParameters params;
    String StoredPing = "";

    /** Creates new OfficeWatcher */
    public OfficeWatcher(TestParameters param) {
        finish = false;
        this.params = param;
    }

    /**
     * pings the office watcher to check for changes
     */
    public void ping() {
        StoredPing += ".";
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
            String previous = StoredPing;
            shortWait(timeOut==0?30000:timeOut);
            // a timeout with value 0 lets watcher not react.
            if (StoredPing.equals(previous) && timeOut != 0){
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
            if (AppKillCommand != null) {
                System.out.println("User defined an application to destroy the started process.");
                System.out.println("Trying to execute: "+AppKillCommand);
                try {
                    Runtime.getRuntime ().exec (AppKillCommand);
                } catch (java.io.IOException e) {
                    e.printStackTrace ();
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
