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

package ifc.ui.dialogs;

import lib.MultiMethodTest;

import com.sun.star.ui.dialogs.XExecutableDialog;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCancellable;

/**
 * Testing <code>com.sun.star.ui.dialogs.XExecutableDialog</code>
 * interface methods :
 * <ul>
 *  <li><code> setTitle()</code></li>
 *  <li><code> execute()</code></li>
 * </ul> <p>
 *
 * This interface methods cann't be checked, thereby methods
 * are just called. <code>execute</code> method is not called
 * at all as the dialog shown cann't be disposed. <p>
 *
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.ui.dialogs.XExecutableDialog
 */
public class _XExecutableDialog extends MultiMethodTest {

    public XExecutableDialog oObj = null;
    private ExecThread eThread = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method successfully returns
     * and no exceptions were thrown. <p>
     */
    public void _setTitle() {
        oObj.setTitle("The Title");
        tRes.tested("setTitle()",true);
    }

    /**
     * This method is excluded from automated test since
     * we can't close the dialog. <p>
     * Always has <b>OK</b> status.
     */
    public void _execute() {
        String aName = tEnv.getTestCase().getObjectName();
        boolean result = false;
        if (aName.startsWith("OData") || aName.startsWith("OSQL")) {
            log.println("dbaccess dialogs can't be closed via API");
            log.println("therefore they aren't executed");
            log.println("and the result is set to true");
            result = true;
        } else {
            eThread = new ExecThread(oObj);
            log.println("Starting Dialog");
            eThread.start();
            XCancellable canc = (XCancellable)UnoRuntime.queryInterface
                    (XCancellable.class, tEnv.getTestObject());
            shortWait();
            if (canc != null) {
                closeDialog();
                short res = eThread.execRes;
                log.println("result: "+res);
                result = (res == 0);
            } else {
                this.disposeEnvironment();
                result=true;
                log.println("XCancellable isn't supported and the "+
                        "environment is killed hard");
            }


        }
        tRes.tested("execute()",result);
    }

    /**
     * Calls <code>execute()</code> method in a separate thread.
     * Necessary to check if this method works
     */
    protected class ExecThread extends Thread {

        public short execRes = (short) 17 ;
        private XExecutableDialog Diag = null ;

        public ExecThread(XExecutableDialog Diag) {
            this.Diag = Diag ;
        }

        public void run() {
            try {
                execRes = Diag.execute();
                System.out.println("HERE: "+execRes);
            } catch(Exception e) {
                log.println("Thread has been interrupted ... ");
            }
        }
    }

    /**
     * Sleeps for 5 sec. to allow StarOffice to react on <code>
     * reset</code> call.
     */
    private void shortWait() {
        try {
            Thread.sleep(2000) ;
        } catch (InterruptedException e) {
            log.println("While waiting :" + e) ;
        }
    }

    public void after() {
        if (eThread.isAlive()) {
            log.println("Thread didn't die ... cleaning up");
            disposeEnvironment();
        }
    }

    private void closeDialog() {
        XCancellable canc = (XCancellable) UnoRuntime.queryInterface(
                XCancellable.class, tEnv.getTestObject());
        if (canc != null) {
            log.println("Cancelling Dialog");
            canc.cancel();
        } else {
            this.disposeEnvironment();
        }

        long st = System.currentTimeMillis();
        boolean toLong = false;

        log.println("waiting for dialog to close");

        while (eThread.isAlive() && !toLong) {
            //wait for dialog to close
            toLong = (System.currentTimeMillis()-st > 10000);
        }

        log.println("done");

        try {
            if (eThread.isAlive()) {
                log.println("Interrupting Thread");
                eThread.interrupt();
                eThread.yield();
            }
        } catch (Exception e) {
            // who cares ;-)
        }

        st = System.currentTimeMillis();
        toLong = false;

        log.println("waiting for interruption to work");

        while (eThread.isAlive() && !toLong) {
            //wait for dialog to close
            toLong = (System.currentTimeMillis()-st > 10000);
        }

        log.println("DialogThread alive: "+eThread.isAlive());

        log.println("done");

    }

}


