/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XNotifyingDispatch.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:04:09 $
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
* @see ifc.frmae._XDispatch
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

