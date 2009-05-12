/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OfficeWatcher.java,v $
 * $Revision: 1.9.2.1 $
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
package helper;

import lib.TestParameters;
import java.util.StringTokenizer;
import util.utils;

public class OfficeWatcher extends Thread implements share.Watcher {

    public boolean finish;
    private TestParameters params;
    private int StoredPing = 0;
    private boolean debug = false;

    /** Creates new OfficeWatcher
     * @param param
     */
    public OfficeWatcher(TestParameters param) {
        finish = false;
        this.params = param;
        debug = params.getBool(util.PropertyName.DEBUG_IS_ACTIVE);
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
        dbg("started");
        boolean isDone = false;
        final ProcessHandler ph = (ProcessHandler) params.get("AppProvider");
        int timeOut = params.getInt("TimeOut");
        if (ph == null) {
            isDone = true;
        }
        while (!isDone) {
            timeOut = params.getInt("TimeOut");
            final int previous = StoredPing;
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
            dbg("the Office is idle for " + timeOut / 1000 +
                " seconds, it probably hangs and is killed NOW.");
            final String AppKillCommand = (String) params.get(util.PropertyName.APP_KILL_COMMAND);
            if (AppKillCommand != null) {
                final StringTokenizer aKillCommandToken = new StringTokenizer(AppKillCommand, ";");
                while (aKillCommandToken.hasMoreTokens()) {
                    final String sKillCommand = aKillCommandToken.nextToken();

                    dbg("User defined an application to destroy the started process.");
                    dbg("Trying to execute: " + sKillCommand);

                    final ProcessHandler pHdl = new ProcessHandler(sKillCommand);
                    pHdl.executeSynchronously();
//                    dbg("---> Output of killoffice:");
//                    dbg(pHdl.getOutputText());
//                    dbg("<--- Output of killoffice");
//                    dbg("---> Error output of killoffice:");
//                    dbg(pHdl.getErrorText());
//                    dbg("<--- Error output of killoffice");

                }
            }
            ph.kill();
        } else {
            dbg("reaeched timeout but ProcessHandler is NULL");
        }
        shortWait(timeOut == 0 ? 30000 : timeOut);
        dbg("finished");
    }

    protected void shortWait(int timeOut) {
        try {
            OfficeWatcher.sleep(timeOut);
        } catch (java.lang.InterruptedException ie) {
        }
    }

    protected void dbg(String message) {
        if (debug) {
            System.out.println(utils.getDateTime() + "OfficeWatcher: " + message);
        }
    }
}
