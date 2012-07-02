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

