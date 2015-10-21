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
import lib.Status;

import com.sun.star.awt.XRadioButton;

/**
* Testing <code>com.sun.star.awt.XRadioButton</code>
* interface methods :
* <ul>
*  <li><code> addItemListener()</code></li>
*  <li><code> removeItemListener()</code></li>
*  <li><code> getState()</code></li>
*  <li><code> setState()</code></li>
*  <li><code> setLabel()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.awt.XRadioButton
*/
public class _XRadioButton extends MultiMethodTest {

    public XRadioButton oObj = null;
    private boolean state = false ;

    /**
    * Listener implementation which sets flags on appropriate method calls
    */
    protected static class TestItemListener implements com.sun.star.awt.XItemListener {
        private final java.io.PrintWriter log;

        public TestItemListener(java.io.PrintWriter log) {
            this.log = log ;
        }

        public void disposing(com.sun.star.lang.EventObject e) {
            log.println(" disposing was called.") ;
        }

        public void itemStateChanged(com.sun.star.awt.ItemEvent e) {
            log.println(" itemStateChanged was called.") ;
        }

    }

    TestItemListener itemListener = null ;

    /**
    * !!! Can be checked only interactively !!!
    */
    public void _addItemListener() {

        itemListener = new TestItemListener(log) ;

        oObj.addItemListener(itemListener) ;

        tRes.tested("addItemListener()", Status.skipped(true)) ;
    }

    /**
    * !!! Can be checked only interactively !!!
    */
    public void _removeItemListener() {
        requiredMethod("addItemListener()") ;

        oObj.removeItemListener(itemListener) ;

        tRes.tested("removeItemListener()", Status.skipped(true)) ;
    }

    /**
    * Gets state and stores it. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _getState() {

        boolean result = true ;
        state = oObj.getState() ;

        tRes.tested("getState()", result) ;
    }

    /**
    * Sets a new state and the gets it for checking. <p>
    * Has <b> OK </b> status if set and get states are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getState </code>  </li>
    * </ul>
    */
    public void _setState() {
        requiredMethod("getState()") ;

        boolean result = true ;
        oObj.setState(!state) ;

        waitForEventIdle();

        result = oObj.getState() == !state ;

        tRes.tested("setState()", result) ;
    }

    /**
    * Just sets a new label. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _setLabel() {

        boolean result = true ;
        oObj.setLabel("XRadioButton") ;

        tRes.tested("setLabel()", result) ;
    }
}


