/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LoggingThread.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 13:16:04 $
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

import share.*;
import lib.TestParameters;
import util.PropertyName;

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

    /**
     *
     * @param log
     * @param tParam
     */
    public LoggingThread(LogWriter log, TestParameters tParam) {
        this.log = log;
        this.param = tParam;
    }

    public void run() {
        int timeOut = param.getInt(PropertyName.TIME_OUT) / 2;
        int count = 0;
        finished = false;
        log.println("TimeOutLogger: start");
        while (!finished && count < 200) {
            try {
                log.println("TimeOutLogger: " + count);
                synchronized (this) {
                    wait(timeOut);
                }
                count++;
            } catch (InterruptedException ex) {
            }
        }
        log.println("TimeOutLogger: finished");
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
            log.println("TimeOutLogger: try to finish ");
            sleep(1000);
        } catch (InterruptedException ex) {
        }
    }
}
