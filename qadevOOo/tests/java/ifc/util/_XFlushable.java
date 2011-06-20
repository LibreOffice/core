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
    * Has OK status if no exception has occurred. <p>
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

