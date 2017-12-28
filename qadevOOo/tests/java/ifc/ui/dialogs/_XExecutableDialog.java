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
 * These interface methods can't be checked, thereby methods
 * are just called. <code>execute</code> method is not called
 * at all as the dialog shown can't be disposed. <p>
 *
 * Test is <b> NOT </b> multithread compliant. <p>
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
            XCancellable canc = UnoRuntime.queryInterface
                    (XCancellable.class, tEnv.getTestObject());
            waitForEventIdle();
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

        public short execRes = (short) 17;
        private final XExecutableDialog Diag;

        public ExecThread(XExecutableDialog Diag) {
            this.Diag = Diag ;
        }

        @Override
        public void run() {
            try {
                execRes = Diag.execute();
                System.out.println("HERE: "+execRes);
            } catch(Exception e) {
                log.println("Thread has been interrupted ... ");
            }
        }
    }

    @Override
    public void after() {
        if (eThread.isAlive()) {
            log.println("Thread didn't die ... cleaning up");
            disposeEnvironment();
        }
    }

    private void closeDialog() {
        XCancellable canc = UnoRuntime.queryInterface(
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
                Thread.yield();
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


