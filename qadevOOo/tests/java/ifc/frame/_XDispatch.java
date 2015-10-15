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

package ifc.frame;


import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XDispatch;
import com.sun.star.util.URL;
import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;
import com.sun.star.frame.XNotifyingDispatch;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.DispatchResultEvent;

/**
* Testing <code>com.sun.star.frame.XDispatch</code>
* interface methods :
* <ul>
*  <li><code> dispatch()</code></li>
*  <li><code> addStatusListener()</code></li>
*  <li><code> removeStatusListener()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'XDispatch.URL'</code> (of type <code>com.sun.star.util.URL
*   </code>): URL for passing to <code>dispatch()</code> method. </li>
* <ul> <p>
* @see com.sun.star.frame.XDispatch
* @see com.sun.star.frame.XNotifyingDispatch
* @see ifc.frame._XDispatch
* @see ifc.frame._XNotifyingDispatch
*/

public class _XDispatch extends MultiMethodTest {

    public XDispatch oObj = null;

    /**
    * Listener implementation which sets flags on appropriate method calls
    */
    protected static class TestStatusListener implements
            com.sun.star.frame.XStatusListener {
        public boolean disposingCalled = false ;
        public boolean statusChangedCalled = false ;
        private final java.io.PrintWriter log;

        public TestStatusListener(java.io.PrintWriter log) {
            this.log = log ;
        }

        public void disposing(com.sun.star.lang.EventObject e) {
            disposingCalled = true ;
            log.println(" disposing was called.") ;
        }

        public void statusChanged(com.sun.star.frame.FeatureStateEvent e) {
            statusChangedCalled = true ;
            log.println(" statusChanged was called.") ;
            log.println("  FeatureURL = '" + e.FeatureURL + "'");
            log.println("  FeatureDescriptor = '" + e.FeatureDescriptor + "'");
            log.println("  IsEnabled = " + e.IsEnabled);
            log.println("  Requery = " + e.Requery);
            log.println("  State = '" + e.State.toString() +  "'");
        }

    }

    /**
    * Listener implementation which sets flags on appropriate method calls
    */
    protected static class TestNotificationListener implements
            com.sun.star.frame.XDispatchResultListener {
        public boolean disposingCalled = false ;
        public boolean finishedDispatch = false ;
        private final java.io.PrintWriter log;

        public TestNotificationListener(java.io.PrintWriter log) {
            this.log = log ;
        }

        public void disposing(com.sun.star.lang.EventObject e) {
            disposingCalled = true ;
            log.println("   disposing was called.") ;
        }

        public void dispatchFinished( DispatchResultEvent e) {
            finishedDispatch = true ;
            log.println("   dispatchFinished was called.") ;
        }

    }

    TestStatusListener listener = null ;
    TestNotificationListener notificationListener = null;
    URL url = null ;

    /**
     * Not all implementations could call the
     * <code>com.sun.star.frame.XStatusListener</code>. For this purposes the
     * <code>com.sun.star.frame.XDispatchWithNotification</code> was designed.
     * If <code>com.sun.star.frame.XStatusListener</code> was not called and
     * <code>com.sun.star.frame.XStatusListener</code> is present, it was used
     * to check listeners.
    */
    private boolean checkXDispatchWithNotification()
    {
        XNotifyingDispatch xND = UnoRuntime.queryInterface(XNotifyingDispatch.class, oObj);
        if ( xND != null) {
            log.println("   XNotifyingDispatch found:");
            PropertyValue[] arguments = (PropertyValue[])
                              tEnv.getObjRelation("XNotifyingDispatchArgument");

            notificationListener = new TestNotificationListener(log) ;
            xND.dispatchWithNotification(url, arguments, notificationListener);

            waitForEventIdle();

            log.println("   Listener called: "+ notificationListener.finishedDispatch);

            return notificationListener.finishedDispatch;
        } else {
            return false;
        }

    }
    /**
    * Retrieves object relations and creates new listeners.
    * @throws StatusException If one of relations not found.
    */
    @Override
    public void before() {
        listener = new TestStatusListener(log) ;
        url = (URL) tEnv.getObjRelation("XDispatch.URL") ;

        if (url == null) throw new StatusException
            (Status.failed("Relation not found.")) ;
    }

    /**
    * Calls the method using URL from relation. <p>
    * Has <b> OK </b> status if one listener (not removed) is called, and
    * another (removed) is not.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code>addStatusListener</code> :
    *    to check that the listener is called
    *  </li>
    * </ul>
    */
    public void _dispatch() {
        requiredMethod("addStatusListener()") ;

        boolean result = true ;

        oObj.dispatch(url, new PropertyValue[0]) ;

        waitForEventIdle();

        log.println("Listener called: "+ listener.statusChangedCalled);

        result = listener.statusChangedCalled;

        if (!result) {
            result = checkXDispatchWithNotification();
        }

        tRes.tested("dispatch()", result) ;
    }

    /**
    * Adds two listeners. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred.
    */
    public void _addStatusListener() {

        boolean result = true ;
        oObj.addStatusListener(listener, url) ;

        tRes.tested("addStatusListener()", result) ;
    }

    /**
    * Removes the listener added before. <p>
    * Has <b> OK </b> status if the dispatch call doesn't call the listener.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> dispatch() </code> : to have a listener to remove
    *  </li>
    * </ul>
    */
    public void _removeStatusListener() {
        requiredMethod("dispatch()") ;
        listener.statusChangedCalled = false;
        boolean result = true ;
        oObj.removeStatusListener(listener, url) ;

        oObj.dispatch(url, new PropertyValue[0]) ;

        waitForEventIdle();

        System.out.println("Listener called: "+ listener.statusChangedCalled);

        result = ! listener.statusChangedCalled;

        tRes.tested("removeStatusListener()", result) ;
    }
}

