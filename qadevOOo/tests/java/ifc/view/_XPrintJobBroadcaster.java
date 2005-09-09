/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XPrintJobBroadcaster.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:45:54 $
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
import util.utils;

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
