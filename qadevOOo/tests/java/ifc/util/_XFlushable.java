/*************************************************************************
 *
 *  $RCSfile: _XFlushable.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:29:45 $
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

package ifc.util;

import lib.MultiMethodTest;

import com.sun.star.util.XFlushListener;
import com.sun.star.util.XFlushable;

/**
 * Testing <code>com.sun.star.util.XFlushable</code>
 * interface methods :
 * <ul>
 *  <li><code> flush()</code></li>
 *  <li><code> addFlushListener()</code></li>
 *  <li><code> removeFlushListener()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.util.XFlushable
 */
public class _XFlushable extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XFlushable oObj = null ;

    /**
     * Simple <code>XFlushListener</code> implementation which
     * just registers if any calls to its methods were made.
     */
    private class MyFlushListener implements XFlushListener{
        boolean called = false ;
        public void flushed(com.sun.star.lang.EventObject e) {
            called = true ;
        }
        public void disposing(com.sun.star.lang.EventObject e) {}
        public void reset() { called = false; }
        public boolean wasFlushed() { return called; }
    }

    private MyFlushListener listener1 = new MyFlushListener(),
                            listener2 = new MyFlushListener() ;

    /**
    * Test call method <code>flush</code> and checks if added listener
    * was called and removed one wasn't. <p>
    * Has OK status if no exception has occured. <p>
    *     Methods to be executed before :
    * {@link #_addFlushListener},
    * {@link #_removeFlushListener}
    */
    public void _flush() {
        executeMethod("addFlushListener()") ;
        executeMethod("removeFlushListener()") ;

        oObj.flush() ;

        tRes.tested("flush()", true) ;
        tRes.tested("addFlushListener()", listener2.wasFlushed()) ;
        tRes.tested("removeFlushListener()", !listener1.wasFlushed()) ;
    }

    /**
    * Test adds two listeners, one of which will be removed then.<p>
    * Has OK status if the listener was called on <code>flush()</code>
    * method call.
    */
    public void _addFlushListener() {
        oObj.addFlushListener(listener1) ;
        oObj.addFlushListener(listener2) ;
    }

    /**
    * Test removes one of two listeners added before. <p>
    * Has OK status if the listener removed wasn't called on
    * <code>flush()</code> method call.
    *     Methods to be executed before :
    * {@link #_addFlushListener},
    */
    public void _removeFlushListener() {
        executeMethod("addFlushListener()") ;

        oObj.removeFlushListener(listener1) ;
    }

}  // finish class _XFlushable

