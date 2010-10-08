/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

import share.*;
import lib.TestParameters;
import util.PropertyName;
import util.utils;

/**
 * This class is printing to a <CODE>LogWriter</CODE>. This could be usefull if a UNO-API
 * function runns longer the the time out. To avoid the assumption of death applikation
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
