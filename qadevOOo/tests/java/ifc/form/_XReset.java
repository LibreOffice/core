/*************************************************************************
 *
 *  $RCSfile: _XReset.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:35:37 $
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


