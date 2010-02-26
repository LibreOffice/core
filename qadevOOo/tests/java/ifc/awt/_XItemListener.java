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
import lib.StatusException;

import com.sun.star.awt.ItemEvent;
import com.sun.star.awt.XItemListener;

/**
* Testing <code>com.sun.star.awt.XItemListener</code>
* interface methods :
* <ul>
*  <li><code> itemStateChanged()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'TestItemListener'</code>
*   (of type <code>ifc.awt._XItemListener.TestItemListener</code>):
*   this <code>XItemListener</code> implementation must be
*   added to the object tested for checking
*   <code> itemStateChanged()</code> method call. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XItemListener
*/
public class _XItemListener extends MultiMethodTest {

    public XItemListener oObj = null;

    /**
    * Listener implementation which sets flags on appropriate method calls
    * and stores event passed.
    */
    public static class TestItemListener implements com.sun.star.awt.XItemListener {
        public boolean itemStateChangedCalled = false ;
        public ItemEvent event = null ;

        public void itemStateChanged(com.sun.star.awt.ItemEvent e) {
            itemStateChangedCalled = true ;
            event = e ;
        }

        public void disposing(com.sun.star.lang.EventObject e) {}

    }

    TestItemListener itemListener = null ;

    /**
    * Retrieves object relation.
    * @throws StatusException If the relation not found.
    */
    public void before() {
        itemListener = (TestItemListener) tEnv.getObjRelation("TestItemListener") ;
        if (itemListener == null)
            throw new StatusException(Status.failed("Relation not found")) ;
    }

    /**
    * First a <code>ItemEvent</code> object created and
    * it is passed to <code>itemStateChanged</code> method
    * call. Then a short wait follows for listener already
    * registered at the object to be caled. <p>
    * Has <b> OK </b> status if the listener was called with
    * the same <code>ItemEvent</code> object as was created
    * before.
    */
    public void _itemStateChanged() {

        boolean result = true ;

        ItemEvent event = new ItemEvent() ;
        event.Selected = 1 ;
        event.Highlighted = 2 ;
        oObj.itemStateChanged(event) ;

        try {
            Thread.sleep(200) ;
        } catch (InterruptedException e) {}

        result = itemListener.itemStateChangedCalled &&
            itemListener.event.Selected == 1 &&
            itemListener.event.Highlighted == 2 ;

        tRes.tested("itemStateChanged()", result) ;
    }

}


