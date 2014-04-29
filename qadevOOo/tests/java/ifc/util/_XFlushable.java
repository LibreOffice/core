/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

