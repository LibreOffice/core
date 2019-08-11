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
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.form.XReset
*/
public class _XReset extends MultiMethodTest {

    public static XReset oObj = null;

    /**
    * Indicates if listeners must approve reset requests or not.
    */
    protected boolean approve = true;
    /**
    * Array of two elements, each of them indicates <code>reset</code>
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
    * on <code>reset</code> and
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
    * on <code>reset</code> and
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
         waitForEventIdle();
         tRes.tested("reset()", (approveReset[0] && (! resetted[0])));
         approve = true;
         oObj.reset();
         waitForEventIdle();
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
         waitForEventIdle();
         tRes.tested("removeResetListener()", !approveReset[0] && !resetted[0]);
         //removing the second listener here may avoid crashing the office
    } // finished _removeResetListener()


} // finished class _XRefresh


