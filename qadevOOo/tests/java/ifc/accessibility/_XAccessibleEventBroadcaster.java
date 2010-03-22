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

package ifc.accessibility;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleEventBroadcaster;
import com.sun.star.accessibility.XAccessibleEventListener;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;

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
public class _XAccessibleEventBroadcaster extends MultiMethodTest {

    public static interface EventProducer {
        void fireEvent();
    }

    public XAccessibleEventBroadcaster oObj = null;
    public String EventMsg = "";
    public boolean destroy = false;


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
     * Retrieves relation.
     * @throws StatusException If the relation is not found.
     */
    public void before() {
        prod = (EventProducer) tEnv.getObjRelation("EventProducer") ;
        if (prod == null) {
            throw new StatusException(Status.failed("Relation missed."));
        }
        EventMsg = (String) tEnv.getObjRelation("EventMsg");
        Object dp = tEnv.getObjRelation("Destroy");
        if (dp != null) {
            destroy=true;
        }
    }

    EventProducer prod = null ;
    EvListener list = new EvListener();

    /**
     * Adds two listeners and fires event by mean of object relation. <p>
     * Has <b> OK </b> status if both listeners were called
     */
    public void _addEventListener() {
        log.println("adding listener");
        oObj.addEventListener(list);
        boolean isTransient = chkTransient(tEnv.getTestObject());
        log.println("fire event");
        prod.fireEvent() ;

        try {
            Thread.sleep(3000);
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

        if (EventMsg != null) {
            log.println(EventMsg);
            tRes.tested("addEventListener()", Status.skipped(true) );
            return;
        }

        tRes.tested("addEventListener()", works );
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
    public void _removeEventListener() {
        requiredMethod("addEventListener()");

        list.notifiedEvent = null;

        log.println("remove listener");
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

        tRes.tested("removeEventListener()", list.notifiedEvent == null);

    }

    protected static boolean chkTransient(Object Testcase) {
        boolean ret = false;
        XAccessibleContext accCon = (XAccessibleContext)
                    UnoRuntime.queryInterface(XAccessibleContext.class,Testcase);
        if (accCon.getAccessibleStateSet().contains(
            com.sun.star.accessibility.AccessibleStateType.TRANSIENT)){
            if (!accCon.getAccessibleParent().getAccessibleContext().getAccessibleStateSet().contains(
                com.sun.star.accessibility.AccessibleStateType.MANAGES_DESCENDANTS)) {
                throw new lib.StatusException(lib.Status.failed("Parent doesn't manage descendents"));
            }
            ret=true;
        }
        return ret;
    }

    /**
    * Forces environment recreation.
    */
    protected void after() {
        if (destroy) disposeEnvironment();
    }


}

