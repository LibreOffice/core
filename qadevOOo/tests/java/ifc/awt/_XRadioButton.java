/*************************************************************************
 *
 *  $RCSfile: _XRadioButton.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:12:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    * Has <b> OK </b> status if no runtime exceptions occured
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
    * Has <b> OK </b> status if no runtime exceptions occured
    */
    public void _setLabel() {

        boolean result = true ;
        oObj.setLabel("XRadioButton") ;

        tRes.tested("setLabel()", result) ;
    }
}


