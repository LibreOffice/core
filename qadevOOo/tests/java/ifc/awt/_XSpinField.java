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

package ifc.awt;

import lib.MultiMethodTest;

import com.sun.star.awt.SpinEvent;
import com.sun.star.awt.XSpinField;
import com.sun.star.awt.XSpinListener;
import com.sun.star.lang.EventObject;

/**
* Testing <code>com.sun.star.awt.XSpinField</code>
* interface methods :
* <ul>
*  <li><code> addSpinListener()</code></li>
*  <li><code> removeSpinListener()</code></li>
*  <li><code> up()</code></li>
*  <li><code> down()</code></li>
*  <li><code> first()</code></li>
*  <li><code> last()</code></li>
*  <li><code> enableRepeat()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.awt.XSpinField
*/
public class _XSpinField extends MultiMethodTest {

    public XSpinField oObj = null;

    /**
    * Listener implementation which set flags on appropriate
    * listener methods calls.
    */
    protected static class TestListener implements XSpinListener {
        public boolean upFl = false ;
        public boolean downFl = false ;
        public boolean firstFl = false ;
        public boolean lastFl = false ;

        public void up(SpinEvent e) {
            upFl = true ;
        }
        public void down(SpinEvent e) {
            downFl = true ;
        }
        public void first(SpinEvent e) {
            firstFl = true ;
        }
        public void last(SpinEvent e) {
            lastFl = true ;
        }
        public void disposing(EventObject e) {}
    }

    private final TestListener listener = new TestListener() ;

    /**
    * Just adds a listener. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _addSpinListener() {
        oObj.addSpinListener(listener) ;

        tRes.tested("addSpinListener()", true) ;
    }

    /**
    * Calls the method. <p>
    * Has <b>OK</b> status if the appropriate listener method
    * was called. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addSpinListener </code> </li>
    * </ul>
    */
    public void _up() {
        requiredMethod("addSpinListener()") ;

        oObj.up() ;
        waitForEventIdle();

        tRes.tested("up()", listener.upFl) ;
    }

    /**
    * Calls the method. <p>
    * Has <b>OK</b> status if the appropriate listener method
    * was called. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addSpinListener </code> </li>
    * </ul>
    */
    public void _down() throws Exception {
        requiredMethod("addSpinListener()") ;

        oObj.down() ;
        waitForEventIdle();

        tRes.tested("down()", listener.downFl) ;
    }

    /**
    * Calls the method. <p>
    * Has <b>OK</b> status if the appropriate listener method
    * was called.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addSpinListener </code> </li>
    * </ul>
    */
    public void _first() throws Exception {
        requiredMethod("addSpinListener()") ;

        oObj.first();
        waitForEventIdle();

        tRes.tested("first()", listener.firstFl) ;
    }

    /**
    * Calls the method. <p>
    * Has <b>OK</b> status if the appropriate listener method
    * was called.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addSpinListener </code> </li>
    * </ul>
    */
    public void _last() throws Exception {
        requiredMethod("addSpinListener()") ;

        oObj.last();
        waitForEventIdle();

        tRes.tested("last()", listener.lastFl) ;
    }

    /**
    * Removes the listener, then calls <code>up</code> method and
    * checks if the listener wasn't called. <p>
    * Has <b>OK</b> status if listener wasn't called. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addSpinListener </code> </li>
    *  <li> <code> up </code> </li>
    *  <li> <code> down </code> </li>
    *  <li> <code> first </code> </li>
    *  <li> <code> last </code> </li>
    * </ul>
    */
    public void _removeSpinListener() {
        requiredMethod("addSpinListener()") ;
        executeMethod("up()") ;
        executeMethod("down()") ;
        executeMethod("first()") ;
        executeMethod("last()") ;

        listener.upFl = false ;

        oObj.removeSpinListener(listener) ;

        oObj.up() ;

        tRes.tested("removeSpinListener()", !listener.upFl) ;
    }

    /**
    * Enables then disables repeating. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _enableRepeat() {
        oObj.enableRepeat(true) ;
        oObj.enableRepeat(false) ;

        tRes.tested("enableRepeat()", true) ;
    }
}
