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

import com.sun.star.awt.XCheckBox;

/**
* Testing <code>com.sun.star.awt.XCheckBox</code>
* interface methods :
* <ul>
*  <li><code> addItemListener()</code></li>
*  <li><code> removeItemListener()</code></li>
*  <li><code> getState()</code></li>
*  <li><code> setState()</code></li>
*  <li><code> setLabel()</code></li>
*  <li><code> enableTriState()</code></li>
* </ul> <p>
*
* @see com.sun.star.awt.XCheckBox
*/
public class _XCheckBox extends MultiMethodTest {

    public XCheckBox oObj = null;

    /**
    * Listener implementation which sets flags on appropriate method calls
    */
    protected class TestItemListener implements com.sun.star.awt.XItemListener {
        public boolean disposingCalled = false ;
        public boolean itemStateChangedCalled = false ;

        public void disposing(com.sun.star.lang.EventObject e) {
            disposingCalled = true ;
        }

        public void itemStateChanged(com.sun.star.awt.ItemEvent e) {
            itemStateChangedCalled = true ;
        }

    }
    TestItemListener listener = new TestItemListener() ;
    short state = -1 ;

    /**
    * !!! Can be checked only interactively !!!
    */
    public void _addItemListener() {

        boolean result = true ;
        oObj.addItemListener(listener) ;
        tRes.tested("addItemListener()", result) ;
    }

    /**
    * !!! Can be checked only interactively !!!
    */
    public void _removeItemListener() {

        boolean result = true ;
        oObj.removeItemListener(listener) ;

        tRes.tested("removeItemListener()", result) ;
    }

    /**
    * Just retrieves current state and stores it. <p>
    * Has <b>OK</b> status if no runtime exceptions occurs.
    */
    public void _getState() {

        boolean result = true ;
        state = oObj.getState() ;

        tRes.tested("getState()", result) ;
    }

    /**
    * Sets a new value and then checks get value. <p>
    * Has <b>OK</b> status if set and get values are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getState </code>  </li>
    * </ul>
    */
    public void _setState() {
        requiredMethod("getState()") ;

        boolean result = true ;
        short newState = state == 0 ? (short)1 : (short)0 ;
        oObj.setState(newState) ;
        result = newState == oObj.getState() ;

        tRes.tested("setState()", result) ;
    }

    /**
    * Just sets some text for label. <p>
    * Has <b>OK</b> status if no runtime exceptions occurs.
    */
    public void _setLabel() {

        boolean result = true ;
        oObj.setLabel("XCheckBox test") ;

        tRes.tested("setLabel()", result) ;
    }

    /**
    * Just enables tristate. <p>
    * Has <b>OK</b> status if no runtime exceptions occurs.
    */
    public void _enableTriState() {

        boolean result = true ;
        oObj.enableTriState(true) ;

        tRes.tested("enableTriState()", result) ;
    }

}


