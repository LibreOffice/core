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
 *  <li><code> addAccessibleEventListener()</code></li>
 *  <li><code> removeAccessibleEventListener()</code></li>
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

    public interface EventProducer {
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
    @Override
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
        oObj.addAccessibleEventListener(list);
        boolean isTransient = chkTransient(tEnv.getTestObject());
        log.println("fire event");
        prod.fireEvent() ;

        util.utils.pause(3000);

        boolean works = true;

        if (list.notifiedEvent == null) {
            if (!isTransient) {
                log.println("listener wasn't called");
                works = false;
            } else {
                log.println("Object is Transient, listener isn't expected to be called");
            }
            oObj.removeAccessibleEventListener(list);
        }

        if (EventMsg != null) {
            log.println(EventMsg);
            tRes.tested("addEventListener()", Status.skipped(true) );
            return;
        }

        tRes.tested("addEventListener()", works );
    }

    /**
     * Removes one of two listeners added before and fires event
     * by mean of object relation.<p>
     *
     * Has <b> OK </b> status if the removed listener wasn't called.<p>
     *
     * The following method tests are to be completed successfully before:
     * <ul>
     *  <li> <code>addEventListener()</code> : to have added listeners </li>
     * </ul>
     */
    public void _removeEventListener() {
        requiredMethod("addEventListener()");

        list.notifiedEvent = null;

        log.println("remove listener");
        oObj.removeAccessibleEventListener(list);

        log.println("fire event");
        prod.fireEvent() ;

        util.utils.shortWait();

        if (list.notifiedEvent == null) {
            log.println("listener wasn't called -- OK");
        }

        tRes.tested("removeEventListener()", list.notifiedEvent == null);

    }

    protected static boolean chkTransient(Object Testcase) {
        boolean ret = false;
        XAccessibleContext accCon = UnoRuntime.queryInterface(XAccessibleContext.class,Testcase);
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
    @Override
    protected void after() {
        if (destroy) disposeEnvironment();
    }


}

