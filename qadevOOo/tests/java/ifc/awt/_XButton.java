/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XButton.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:55:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    * Has <b> OK </b> status if no runtime exceptions occured
    */
    public void _setLabel() {

        boolean result = true ;
        oObj.setLabel("XButton Label") ;

        tRes.tested("setLabel()", result) ;
    }

    /**
    * Just sets some command for button. <p>
    * Has <b> OK </b> status if no runtime exceptions occured
    */
    public void _setActionCommand() {

        boolean result = true ;
        oObj.setActionCommand("XButtonComand") ;

        tRes.tested("setActionCommand()", result) ;
    }

}


