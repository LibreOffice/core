/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XAccessibleEventBroadcaster.java,v $
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

package complex.toolkit.interface_tests;

import com.sun.star.lang.EventObject;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.PosSize;
import com.sun.star.awt.XWindow;
import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleEventBroadcaster;
import com.sun.star.accessibility.XAccessibleEventListener;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import share.LogWriter;

/**
 * Testing <code>
 * com.sun.star.accessibility.XAccessibleEventBroadcaster</code>
 * interface methods :
 * <ul>
 *  <li><code> addEventListener()</code></li>
 *  <li><code> removeEventListener()</code></li>
 * </ul> <p>
 *
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'EventProducer'</code> (of type
 *  <code>ifc.accessibility._XAccessibleEventBroadcaster.EventProducer</code>):
 *   this must be an implementation of the interface which could perform
 *   some actions for generating any kind of <code>AccessibleEvent</code></li>
 * <ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 */
public class _XAccessibleEventBroadcaster {

    private LogWriter log;
    private static final String className =
            "com.sun.star.accessibility.XAccessibleEventBroadcaster" ;

    public XAccessibleEventBroadcaster oObj = null;
    public String EventMsg = "";
    EventProducer prod = null ;
    EvListener list = new EvListener();

    /**
     * An event producer
     */
    public static class EventProducer {
        XWindow xWindow;
        public EventProducer(XWindow window) {
            xWindow = window;
        }

       public void fireEvent() {
            Rectangle newPosSize = xWindow.getPosSize();
            newPosSize.Width = newPosSize.Width - 20;
            newPosSize.Height = newPosSize.Height - 20;
            newPosSize.X = newPosSize.X + 20;
            newPosSize.Y = newPosSize.Y + 20;
            xWindow.setPosSize(newPosSize.X, newPosSize.Y, newPosSize.Width,
                                    newPosSize.Height, PosSize.POSSIZE);
        }
    }

    /**
     * Listener implementation which registers listener calls.
     */
    private class EvListener implements XAccessibleEventListener {
        public AccessibleEventObject notifiedEvent = null ;
        public void notifyEvent(AccessibleEventObject ev) {
            log.println("Listener, Event : " + ev.EventId);
            System.out.println("EventID: " + ev.EventId);
            Object old=ev.OldValue;
            if (old instanceof com.sun.star.accessibility.XAccessible) {
                System.out.println("Old: "+((XAccessible)old).getAccessibleContext().getAccessibleName());
            }

            Object nev=ev.NewValue;
            if (nev instanceof com.sun.star.accessibility.XAccessible) {
                System.out.println("New: "+((XAccessible)nev).getAccessibleContext().getAccessibleName());
            }
            notifiedEvent = ev;
        }

        public void disposing(EventObject ev) {}
    }

    /**
     * c'tor
     */
    public _XAccessibleEventBroadcaster(XInterface object, LogWriter log, String eventMessage, XWindow window) {
        oObj = (XAccessibleEventBroadcaster)UnoRuntime.queryInterface(
                                    XAccessibleEventBroadcaster.class, object);
        this.log = log;
        prod = new EventProducer(window);
        EventMsg = eventMessage;
    }

    /**
     * Adds two listeners and fires event by mean of object relation. <p>
     * Has <b> OK </b> status if both listeners were called
     */
    public boolean _addEventListener() {
        log.println("adding two listeners");
        oObj.addEventListener(list);
        boolean isTransient = chkTransient(oObj);
        log.println("fire event");
        prod.fireEvent() ;

        try {
            Thread.sleep(1500);
        }
        catch (InterruptedException ex) {
        }

        boolean works = true;

        if (list.notifiedEvent == null) {
            if (!isTransient) {
                log.println("listener wasn't called");
                works = false;
            } else {
                log.println("Object is Transient, listener isn't expected to be called");
            }
            oObj.removeEventListener(list);
        }

//        log.println(EventMsg);
        return works;
    }

    /**
     * Removes one of two listeners added before and and fires event
     * by mean of object relation. <p>
     *
     * Has <b> OK </b> status if the removed listener wasn't called. <p>
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code>addEventListener()</code> : to have added listeners </li>
     * </ul>
     */
    public boolean _removeEventListener() {

        list.notifiedEvent = null;

        log.println("remove first listener");
        oObj.removeEventListener(list);

        log.println("fire event");
        prod.fireEvent() ;

        try {
            Thread.sleep(500);
        }
        catch (InterruptedException ex) {
        }

        if (list.notifiedEvent == null) {
            log.println("listener wasn't called -- OK");
        }

        return list.notifiedEvent == null;

    }

    protected static boolean chkTransient(Object Testcase) {
        XAccessibleContext accCon = (XAccessibleContext)
                    UnoRuntime.queryInterface(XAccessibleContext.class,Testcase);
        if (accCon.getAccessibleStateSet().contains(
            com.sun.star.accessibility.AccessibleStateType.TRANSIENT)){
            if (!accCon.getAccessibleParent().getAccessibleContext().getAccessibleStateSet().contains(
                        com.sun.star.accessibility.AccessibleStateType.MANAGES_DESCENDANTS)) {
                return false;
            }
            return true;
        }
        return false;
    }

}

