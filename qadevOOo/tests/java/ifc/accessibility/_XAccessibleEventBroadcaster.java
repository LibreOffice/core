/*************************************************************************
 *
 *  $RCSfile: _XAccessibleEventBroadcaster.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change:$Date: 2003-09-08 10:05:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

