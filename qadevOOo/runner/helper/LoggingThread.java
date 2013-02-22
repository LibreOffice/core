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

import share.*;
import lib.TestParameters;
import util.PropertyName;
import util.utils;

/**
 * This class is printing to a <CODE>LogWriter</CODE>. This could be usefull if a UNO-API
 * function runns longer the time out. To avoid the assumption of death applikation
 * a simple string is logged for 100 times of time out.</br>
 * Example:</br>
 *          logger = new LoggingThread((LogWriter)log, tParam);
 *          logger.start();
 *          oObj.longRunningFunction();
 *          logger.finish();
 *
 */
public class LoggingThread extends Thread {

    TestParameters param;
    LogWriter log = null;
    boolean finished = false;
    boolean debug = false;

    /**
     *
     * @param log
     * @param tParam
     */
    public LoggingThread(LogWriter log, TestParameters tParam) {
        this.log = log;
        this.param = tParam;
        this.debug = tParam.getBool(PropertyName.DEBUG_IS_ACTIVE);
    }

    public void run() {
        final int timeOut = param.getInt(PropertyName.TIME_OUT) / 2;
        int count = 0;
        finished = false;
        if (debug) log.println("TimeOutLogger: " + utils.getDateTime() + " start");
        while (!finished && count < 200) {
            try {
                if (debug) log.println("TimeOutLogger: "+utils.getDateTime() + count);
                synchronized (this) {
                    wait(timeOut);
                }
                count++;
            } catch (InterruptedException ex) {
            }
        }
        if (debug) log.println("TimeOutLogger: " + utils.getDateTime() + " finished");
    }

    /**
     * finished the LoggingThread
     */
    public void finish() {
        try {
            finished = true;
            synchronized (this) {
                notify();
            }
            if (debug) log.println("TimeOutLogger: " + utils.getDateTime() + " try to finish ");
            sleep(1000);
        } catch (InterruptedException ex) {
        }
    }
}
