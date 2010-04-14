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

package ifc.form;

import lib.MultiMethodTest;

import com.sun.star.form.XReset;
import com.sun.star.form.XResetListener;
import com.sun.star.lang.EventObject;

/**
* Testing <code>com.sun.star.form.XReset</code>
* interface methods :
* <ul>
*  <li><code> reset()</code></li>
*  <li><code> addResetListener()</code></li>
*  <li><code> removeResetListener()</code></li>
* </ul>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.form.XReset
*/
public class _XReset extends MultiMethodTest {

    public static XReset oObj = null;

    /**
    * Indicates if listeners must approve restes requests or not.
    */
    protected boolean approve = true;
    /**
    * Array of two elements, each of them indicates <code>resetted</code>
    * call of appropriate listener.
    */
      protected boolean resetted[] = new boolean[2];
    /**
    * Array of two elements, each of them indicates
    * <code>approveReset</code> call of appropriate listener.
    */
    protected boolean approveReset[] = new boolean[2];

    /**
    * The listener which sets flags (in array elements with index 0)
    * on <code>resetted</code> and
    * <code>approveReset</code> events. It approves reset request
    * depending on <code>approve</code> field.
    */
    protected class MyResetListener implements XResetListener {
           public void disposing ( EventObject oEvent ) {}
               public boolean approveReset ( EventObject oEvent ) {
                   approveReset[0] = true;
                   //cancel the reset action
                   return approve;
                   }
               public void resetted ( EventObject oEvent ) {
                   resetted[0] = true;
                   }
      }


    /**
    * The listener which sets flags (in array elements with index 1)
    * on <code>resetted</code> and
    * <code>approveReset</code> events. It approves reset request
    * depending on <code>approve</code> field.
    */
    protected class MyResetListener2 implements XResetListener {
           public void disposing ( EventObject oEvent ) {}
               public boolean approveReset ( EventObject oEvent ) {
                   approveReset[1] = true;
                   //don't cancel the reset action
                   return true;
                   }
               public void resetted ( EventObject oEvent ) {
                   resetted[1] = true;
                   }
    }

    /**
    * Listener which is added in test
    */
    protected XResetListener listener1 = new MyResetListener();
    /**
    * Listener which is added in test
    */
    protected XResetListener listener2 = new MyResetListener2();

    /**
    * Just adds two reset listeners. <p>
    * Status for it is set later in <code>reset</code> method test.
    */
    public void _addResetListener() {

         log.println("Testing addResetListener ...");
         oObj.addResetListener( listener2 );
         oObj.addResetListener( listener1 );

    } // finished _addResetListener()

    /**
    * First calls <code>reset</code> method without approving
    * the request, in this case only <code>approveReset</code>
    * event must be called. Second calls <code>reset</code> with
    * approving the request. In this case both listener's events
    * must be called. <p>
    * Has <b>OK</b> status for <code>reset</code> method if in
    * the first case only <code>approveReset</code> method was
    * called. <p>
    * Has <b>OK</b> status for <code>addResetListener</code> method
    * if in the second case both listener's methods were called.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addResetListener </code> : to have listeners added.</li>
    * </ul>
    */
    public void _reset() {

         executeMethod("addResetListener()");
         log.println("Testing reset() ...");
         approve = false;
         oObj.reset();
         shortWait();
         tRes.tested("reset()", (approveReset[0] && (! resetted[0])));
         approve = true;
         oObj.reset();
         shortWait();
         tRes.tested("addResetListener()", (approveReset[1] && resetted[1]));

    } // finished _reset

    /**
    * Removes the first listener, clears it's call flags, and
    * calls <code>reset</code> method.<p>
    * Has <b> OK </b> status if no methods of the listener removed
    * were called. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> reset </code> : to test this method last. </li>
    * </ul>
    */
    public void _removeResetListener() {
         requiredMethod("reset()");
         log.println("Testing removeResetListener ...");
         approveReset[0] = resetted[0] = false;
         oObj.removeResetListener(listener1);
         oObj.reset();
         shortWait();
         tRes.tested("removeResetListener()", !approveReset[0] && !resetted[0]);
         //removing the second listener here may avoid crashing the office

         return;

    } // finished _removeResetListener()

    /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(500) ;
        } catch (InterruptedException e) {
            log.println("While waiting :" + e) ;
        }
    }


} // finished class _XRefresh


