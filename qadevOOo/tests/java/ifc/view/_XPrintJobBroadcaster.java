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
package ifc.view;

import com.sun.star.beans.PropertyValue;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.view.PrintJobEvent;
import com.sun.star.view.XPrintJobBroadcaster;
import com.sun.star.view.XPrintJobListener;
import com.sun.star.view.XPrintable;
import java.io.File;
import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

/**
 * Test the XPrintJobBroadcaster interface
 */
public class _XPrintJobBroadcaster extends MultiMethodTest {
    public XPrintJobBroadcaster oObj = null;
    MyPrintJobListener listenerImpl = null;

    /**
     * Get an object implementation of the _XPrintJobListener interface from the
     * test environment.
     */
    public void before() {
        listenerImpl = (MyPrintJobListener)tEnv.getObjRelation("XPrintJobBroadcaster.XPrintJobListener");
        if (listenerImpl == null) {
            throw new StatusException(Status.failed(" No test possible. The XPrintJobListener interface has to be implemented."));
        }
    }

    /**
     * add the listener, see if it's called.
     */
    public void _addPrintJobListener() {
        oObj.addPrintJobListener(listenerImpl);
        listenerImpl.fireEvent();
        util.utils.shortWait(1000);
        tRes.tested("addPrintJobListener()", listenerImpl.actionTriggered());
    }

    /**
     * remove the listener, see if it's still caleed.
     */
    public void _removePrintJobListener() {
        requiredMethod("addPrintJobListener");
        oObj.removePrintJobListener(listenerImpl);

            util.utils.shortWait(5000);

        listenerImpl.reset();
        listenerImpl.fireEvent();
        tRes.tested("removePrintJobListener()", !listenerImpl.actionTriggered());
    }

    /**
     * Implementation for testing the XPrintJobBroadcaster interface:
     * a listener to add.
     */
    public static class MyPrintJobListener implements XPrintJobListener {
        boolean eventCalled = false;
        // object to trigger the event
        XPrintable xPrintable = null;
        PropertyValue[]printProps = null;
        String printFileName = null;

        /**
         * Constructor
         * @param printable An object that can be cast to an XPrintable.
         */
        public MyPrintJobListener(Object printable, String printFileName) {
            this.printFileName = printFileName;
            xPrintable = UnoRuntime.queryInterface(XPrintable.class, printable);
            printProps = new PropertyValue[2];
            printProps[0] = new PropertyValue();
            printProps[0].Name = "FileName";
            printProps[0].Value = printFileName;
            printProps[0].State = com.sun.star.beans.PropertyState.DEFAULT_VALUE;
            printProps[1] = new PropertyValue();
            printProps[1].Name = "Wait";
            printProps[1].Value = new Boolean(true);
        }

        /**
         * Has the action been triggered?
         * @return True if "printJobEvent" has been called.
         */
        public boolean actionTriggered() {
            return eventCalled;
        }

        /**
         * Fire the event that calls the printJobEvent
         */
        public void fireEvent() {
            try {
                xPrintable.print(printProps);
            }
            catch(com.sun.star.lang.IllegalArgumentException e) {
            }
        }

        public void reset() {
            File f = new File(printFileName);
            if (f.exists())
                f.delete();
            eventCalled = false;
        }

        /**
         * The print job event: has to be called when the action is triggered.
         */
        public void printJobEvent(PrintJobEvent printJobEvent) {
            eventCalled = true;
        }

        /**
         * Disposing event: ignore.
         */
        public void disposing(com.sun.star.lang.EventObject eventObject) {
        }
    }

}
