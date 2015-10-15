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
    protected static class TestItemListener implements com.sun.star.awt.XItemListener {
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


