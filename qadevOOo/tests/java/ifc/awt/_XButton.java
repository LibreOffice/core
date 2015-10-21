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

import com.sun.star.awt.XButton;

/**
* Testing <code>com.sun.star.awt.XButton</code>
* interface methods :
* <ul>
*  <li><code> addActionListener()</code></li>
*  <li><code> removeActionListener()</code></li>
*  <li><code> setLabel()</code></li>
*  <li><code> setActionCommand()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.awt.XButton
*/
public class _XButton extends MultiMethodTest {

    public XButton oObj = null;

    /**
    * Listener implementation which sets flags on appropriate method calls
    */
    protected static class TestActionListener implements com.sun.star.awt.XActionListener {
        public boolean disposingCalled = false ;

        public void disposing(com.sun.star.lang.EventObject e) {
            disposingCalled = true ;
        }

        public void actionPerformed(com.sun.star.awt.ActionEvent e) {}
    }

    TestActionListener listener = new TestActionListener() ;

    /**
    * !!! Can be checked only interactively !!!
    */
    public void _addActionListener() {

        boolean result = true ;
        oObj.addActionListener(listener) ;

        tRes.tested("addActionListener()", result) ;
    }

    /**
    * !!! Can be checked only interactively !!!
    */
    public void _removeActionListener() {

        boolean result = true ;
        oObj.removeActionListener(listener) ;

        tRes.tested("removeActionListener()", result) ;
    }

    /**
    * Just sets some text for label. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _setLabel() {

        boolean result = true ;
        oObj.setLabel("XButton Label") ;

        tRes.tested("setLabel()", result) ;
    }

    /**
    * Just sets some command for button. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _setActionCommand() {

        boolean result = true ;
        oObj.setActionCommand("XButtonCommand") ;

        tRes.tested("setActionCommand()", result) ;
    }

}


