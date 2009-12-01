/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XPrintJobBroadcaster.java,v $
 * $Revision: 1.4 $
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
         * @param An object that can be cast to an XPrintable.
         */
        public MyPrintJobListener(Object printable, String printFileName) {
            this.printFileName = printFileName;
            xPrintable = (XPrintable)UnoRuntime.queryInterface(XPrintable.class, printable);
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
