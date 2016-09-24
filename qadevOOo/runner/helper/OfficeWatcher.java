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
package helper;

import lib.TestParameters;
import util.utils;

public class OfficeWatcher extends Thread implements share.Watcher {

    public boolean finish;
    private final TestParameters params;
    private int StoredPing = 0;
    private final boolean debug;

    public OfficeWatcher(TestParameters param) {
        this.finish = false;
        this.params = param;
        this.debug = params.getBool(util.PropertyName.DEBUG_IS_ACTIVE);
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

    @Override
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
            util.utils.pause(timeOut == 0 ? 30000 : timeOut);
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
            ph.kill();
        } else {
            dbg("reached timeout but ProcessHandler is NULL");
        }
        util.utils.pause(timeOut == 0 ? 30000 : timeOut);
        dbg("finished");
    }

    private void dbg(String message) {
        if (debug) {
            System.out.println(utils.getDateTime() + "OfficeWatcher: " + message);
        }
    }
}
