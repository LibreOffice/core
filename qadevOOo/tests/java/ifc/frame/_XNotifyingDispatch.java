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
import com.sun.star.util.URL;
import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;
import com.sun.star.frame.XNotifyingDispatch;
import com.sun.star.frame.DispatchResultEvent;

/**
* Testing <code>com.sun.star.frame.XNotifyingDispatch</code>
* interface methods :
* <ul>
*  <li><code> dispatchWithNotification()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'XDispatch.URL'</code> (of type <code>com.sun.star.util.URL
*   </code>): URL for passing to <code>dispatch()</code> method. </li>
*  <li> <code>[OPTIONAL] 'XNotifyingDispatchArgument'</code>
*    (of type  sequence<code>com::sun::star::beans::PropertyValue
*   </code>): argumets for <code>dispatchWithNotification()</code> method. </li>
* <ul> <p>
* @see com.sun.star.frame.XDispatch
* @see com.sun.star.frame.XNotifyingDispatch
* @see ifc.frame._XDispatch
*/
public class _XNotifyingDispatch extends MultiMethodTest {

    public XNotifyingDispatch oObj = null;


    /**
    * Listener implementation which sets flags on appropriate method calls
    */
    protected class TestNotificationListener implements
            com.sun.star.frame.XDispatchResultListener {
        public boolean disposingCalled = false ;
        public boolean finishedDispatch = false ;
        private java.io.PrintWriter log = null ;

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

    TestNotificationListener notificationListener = null;
    PropertyValue[] arguments = null;
    URL url = null ;

    /**
    * Retrieves object relations and creates new listeners.
    * @throws StatusException If one of relations not found.
    */
    public void before() {
        notificationListener = new TestNotificationListener(log) ;
        url = (URL) tEnv.getObjRelation("XDispatch.URL") ;

        if (url == null) throw new StatusException
            (Status.failed("Relation not found.")) ;

        arguments = (PropertyValue[])
                              tEnv.getObjRelation("XNotifyingDispatchArgument");
    }

    /**
    * Calls the method using URL and arguments from relation. <p>
    * Has <b> OK </b> status if listener is called.
    * The following method tests are to be completed successfully before :
    */
    public void _dispatchWithNotification() {

        boolean result = true ;

        oObj.dispatchWithNotification(url, arguments, notificationListener);

        try {
            Thread.sleep(200);
        }
        catch(java.lang.InterruptedException e) {}

        log.println("Listener called: "+ notificationListener.finishedDispatch);

        result = notificationListener.finishedDispatch;


        tRes.tested("dispatchWithNotification()", result) ;
    }

}

