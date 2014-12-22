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
* Test is <b> NOT </b> multithread compliant. <p>
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
    @Override
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

        util.utils.pause(200);

        result = itemListener.itemStateChangedCalled &&
            itemListener.event.Selected == 1 &&
            itemListener.event.Highlighted == 2 ;

        tRes.tested("itemStateChanged()", result) ;
    }

}


