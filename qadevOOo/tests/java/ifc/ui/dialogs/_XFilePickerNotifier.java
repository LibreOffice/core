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
import lib.Status;
import lib.StatusException;

import com.sun.star.lang.EventObject;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ui.dialogs.FilePickerEvent;
import com.sun.star.ui.dialogs.XExecutableDialog;
import com.sun.star.ui.dialogs.XFilePicker;
import com.sun.star.ui.dialogs.XFilePickerListener;
import com.sun.star.ui.dialogs.XFilePickerNotifier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCancellable;


/**
 * Testing <code>com.sun.star.ui.XFilePickerNotifier</code>
 * interface methods :
 * <ul>
 *  <li><code> addFilePickerListener()</code></li>
 *  <li><code> removeFilePickerListener()</code></li>
 * </ul> <p>
 * The object must implement <code>XFilePicker</code>
 * interface to check if a listener was called. <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.ui.XFilePickerNotifier
 */
public class _XFilePickerNotifier extends MultiMethodTest {

    public XFilePickerNotifier oObj = null;
    private XFilePicker fps = null ;
    private String dir1 = null,
            dir2 = null ;
    ExecThread eThread = null;


    /**
     * Listener implementation which sets a flag if some of its
     * methods was called.
     */
    protected class TestListener implements XFilePickerListener {
        public boolean called = false ;

        public void dialogSizeChanged() {
            called = true;
        }

        public void fileSelectionChanged(FilePickerEvent e) {
            called = true;
        }

        public void directoryChanged(FilePickerEvent e) {
            log.println("***** Directory Changed *****");
            called = true;
        }

        public String helpRequested(FilePickerEvent e) {
            called = true;
            return "help";
        }

        public void controlStateChanged(FilePickerEvent e) {
            called = true;
        }

        public void disposing(EventObject e) {}
    }

    TestListener listener = new TestListener() ;

    /**
     * Tries to query object for <code>XFilePicker</code> interface, and
     * initializes two different URLs for changing file picker directory. <p>
     * @throw StatusException If object doesn't support <code>XFilePicker</code>
     * interface.
     */
    public void before() {
        fps = UnoRuntime.queryInterface
                (XFilePicker.class, oObj) ;

        if (fps == null) {
            log.println("The object doesnt implement XFilePicker") ;
            throw new StatusException(Status.failed
                    ("The object doesnt implement XFilePicker"));
        }

        XExecutableDialog exD = UnoRuntime.queryInterface(
                XExecutableDialog.class, tEnv.getTestObject());

        dir1 = util.utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF());
        dir2 = util.utils.getFullTestURL("");
        eThread = new ExecThread(exD);
    }

    /**
     * Adds a listener, then tries to change display directory and
     * checks if the listener was called. <p>
     * Has <b>OK</b> status if a listener method was called.
     */
    public void _addFilePickerListener() {
        oObj.addFilePickerListener(listener) ;

        try {
            log.println("***** Setting DisplayDirectory to " + dir1);
            fps.setDisplayDirectory(dir1) ;
            log.println("***** Getting: " + fps.getDisplayDirectory());
            openDialog();
            log.println("***** Setting DisplayDirectory to " + dir2);
            fps.setDisplayDirectory(dir2) ;
            log.println("***** Getting: " + fps.getDisplayDirectory());

        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("!!! Exception changing dir !!!") ;
            e.printStackTrace(log) ;
        }

        shortWait();

        if (!listener.called) {
            log.println("Listener wasn't called :-(");
        }

        closeDialog();

        tRes.tested("addFilePickerListener()", listener.called) ;
    }

    /**
     * Removes the listener and changes display directory. <p>
     * Has <b>OK</b> status if the listener wasn't called. <p>
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> addFilePickerListener </code> </li>
     * </ul>
     */
    public void _removeFilePickerListener() {
        requiredMethod("addFilePickerListener()") ;

        oObj.removeFilePickerListener(listener) ;

        listener.called = false ;

        try {
            fps.setDisplayDirectory(dir1) ;
            openDialog();
            fps.setDisplayDirectory(dir2) ;
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("!!! Exception changing dir !!!") ;
            e.printStackTrace(log) ;
        }

        shortWait();

        closeDialog();

        tRes.tested("removeFilePickerListener()", !listener.called) ;
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
            } catch (Exception e) {
                log.println("Thread has been interrupted ...");
            }
        }
    }

    /**
     * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
     * reset</code> call.
     */
    private void shortWait() {
        try {
            Thread.sleep(2000) ;
        } catch (InterruptedException e) {
            log.println("While waiting :" + e) ;
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

    private void openDialog() {
        log.println("Starting Dialog");
        if (eThread.isAlive()) {
            log.println("second interrupt");
            eThread.interrupt();
            Thread.yield();
        }

        XExecutableDialog exD = UnoRuntime.queryInterface(
                XExecutableDialog.class, tEnv.getTestObject());

        dir1 = util.utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF());
        dir2 = util.utils.getFullTestURL("");
        eThread = new ExecThread(exD);

        eThread.start();
    }
}


