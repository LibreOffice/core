/*************************************************************************
 *
 *  $RCSfile: _XDispatch.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-10-06 13:30:04 $
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
    protected class TestStatusListener implements
            com.sun.star.frame.XStatusListener {
        public boolean disposingCalled = false ;
        public boolean statusChangedCalled = false ;
        private java.io.PrintWriter log = null ;

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
        XNotifyingDispatch xND = (XNotifyingDispatch)
                      UnoRuntime.queryInterface(XNotifyingDispatch.class, oObj);
        if ( xND != null) {
            log.println("   XNotifyingDispatch found:");
            PropertyValue[] arguments = (PropertyValue[])
                              tEnv.getObjRelation("XNotifyingDispatchArgument");

            notificationListener = new TestNotificationListener(log) ;
            xND.dispatchWithNotification(url, arguments, notificationListener);

            try {
                Thread.sleep(200);
            }
            catch(java.lang.InterruptedException e) {}

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

        try {
            Thread.sleep(200);
        }
        catch(java.lang.InterruptedException e) {}

        log.println("Listener called: "+ listener.statusChangedCalled);

        result = listener.statusChangedCalled;

        if (result == false) {
            result = checkXDispatchWithNotification();
        }

        tRes.tested("dispatch()", result) ;
    }

    /**
    * Adds two listeners. <p>
    * Has <b> OK </b> status if no runtime exceptions occured.
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

        try {
            Thread.sleep(200);
        }
        catch(java.lang.InterruptedException e) {}

        System.out.println("Listener called: "+ listener.statusChangedCalled);

        result = ! listener.statusChangedCalled;

        tRes.tested("removeStatusListener()", result) ;
    }
}

