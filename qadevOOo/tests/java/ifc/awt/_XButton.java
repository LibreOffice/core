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
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XButton
*/
public class _XButton extends MultiMethodTest {

    public XButton oObj = null;

    /**
    * Listener implementation which sets flags on appropriate method calls
    */
    protected class TestActionListener implements com.sun.star.awt.XActionListener {
        public boolean disposingCalled = false ;
        public boolean actionPerformedCalled = false ;

        public void disposing(com.sun.star.lang.EventObject e) {
            disposingCalled = true ;
        }

        public void actionPerformed(com.sun.star.awt.ActionEvent e) {
            actionPerformedCalled = true ;
        }

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
        oObj.setActionCommand("XButtonComand") ;

        tRes.tested("setActionCommand()", result) ;
    }

}


