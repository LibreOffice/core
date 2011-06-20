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
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XRadioButton
*/
public class _XRadioButton extends MultiMethodTest {

    public XRadioButton oObj = null;
    private boolean state = false ;

    /**
    * Listener implementation which sets flags on appropriate method calls
    */
    protected class TestItemListener implements com.sun.star.awt.XItemListener {
        public boolean disposingCalled = false ;
        public boolean itemStateChangedCalled = false ;
        private java.io.PrintWriter log = null ;

        public TestItemListener(java.io.PrintWriter log) {
            this.log = log ;
        }

        public void disposing(com.sun.star.lang.EventObject e) {
            disposingCalled = true ;
            log.println(" disposing was called.") ;
        }

        public void itemStateChanged(com.sun.star.awt.ItemEvent e) {
            itemStateChangedCalled = true ;
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

        try {
            Thread.sleep(200) ;
        } catch (InterruptedException e) {}

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


