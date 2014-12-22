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

package complex.toolkit.accessibility;

import com.sun.star.lang.EventObject;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.PosSize;
import com.sun.star.awt.XWindow;
import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.AccessibleStateType;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleEventBroadcaster;
import com.sun.star.accessibility.XAccessibleEventListener;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;

/**
 * Testing <code>
 * com.sun.star.accessibility.XAccessibleEventBroadcaster</code>
 * interface methods :
 * <ul>
 *  <li><code> addAccessibleEventListener()</code></li>
 *  <li><code> removeAccessibleEventListener()</code></li>
 * </ul>
 *
 * <p>This test needs the following object relations :</p>
 * <ul>
 *  <li> <code>'EventProducer'</code> (of type
 *  <code>ifc.accessibility._XAccessibleEventBroadcaster.EventProducer</code>):
 *   this must be an implementation of the interface which could perform
 *   some actions for generating any kind of <code>AccessibleEvent</code></li>
 * </ul>
 *
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 */
public class _XAccessibleEventBroadcaster {

    private final XAccessibleEventBroadcaster oObj;
    private final EventProducer prod;
    private final EvListener list = new EvListener();

    /**
     * An event producer
     */
    private static class EventProducer {
        private final XWindow xWindow;
        private EventProducer(XWindow window) {
            xWindow = window;
        }

       private void fireEvent() {
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
            System.out.println("Listener, Event : " + ev.EventId);
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

    public _XAccessibleEventBroadcaster(XInterface object, XWindow window) {
        oObj = UnoRuntime.queryInterface(XAccessibleEventBroadcaster.class, object);
        prod = new EventProducer(window);
    }

    /**
     * Adds two listeners and fires event by mean of object relation. <p>
     * Has <b> OK </b> status if both listeners were called
     */
    public boolean _addEventListener() {
        System.out.println("adding two listeners");
        oObj.addAccessibleEventListener(list);
        boolean isTransient = chkTransient(oObj);
        System.out.println("fire event");
        prod.fireEvent() ;

        util.utils.pause(1500);

        boolean works = true;

        if (list.notifiedEvent == null) {
            if (!isTransient) {
                System.out.println("listener wasn't called");
                works = false;
            } else {
                System.out.println("Object is Transient, listener isn't expected to be called");
            }
            oObj.removeAccessibleEventListener(list);
        }

        return works;
    }

    /**
     * Removes one of two listeners added before and fires event
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

        System.out.println("remove first listener");
        oObj.removeAccessibleEventListener(list);

        System.out.println("fire event");
        prod.fireEvent() ;

        util.utils.pause(500);

        if (list.notifiedEvent == null) {
            System.out.println("listener wasn't called -- OK");
        }

        return list.notifiedEvent == null;

    }

    private static boolean chkTransient(Object Testcase) {
        XAccessibleContext accCon = UnoRuntime.queryInterface(XAccessibleContext.class, Testcase);
        return accCon.getAccessibleStateSet().contains(
            AccessibleStateType.TRANSIENT)
            && accCon.getAccessibleParent().getAccessibleContext()
                .getAccessibleStateSet().contains(
                    AccessibleStateType.MANAGES_DESCENDANTS);
    }

}

