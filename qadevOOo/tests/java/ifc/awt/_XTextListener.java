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

import com.sun.star.awt.TextEvent;
import com.sun.star.awt.XTextListener;

/**
* Testing <code>com.sun.star.awt.XTextListener</code>
* interface methods:
* <ul>
* <li><code> textChanged() </code></li>
* </ul><p>
*
* This test needs the following object relations :
* <ul>
*  <li> <code>'TestTextListener'</code>
*   (of type <code>ifc.awt._XTextListener.TestTextListener</code>):
*   this listener implementation must be registered for object tested for
*   checking
*   <code>textChanged()</code> method call. The listener must be registered
*   in object environment creation because it's not a fact that tested
*   component supports <code>XTextComponent</code> interface and the listener
*   can be registered in another object.</li>
* <ul> <p>
*
* @see com.sun.star.awt.XTextListener
*/
public class _XTextListener extends MultiMethodTest {
    public XTextListener oObj = null;

    /**
    * Listener implementation which sets flags on appropriate method calls
    * and stores event passed.
    */
    public static class TestTextListener implements
            com.sun.star.awt.XTextListener {
        public boolean textChangedCalled = false ;
        public TextEvent event = null ;

        public void textChanged(TextEvent e) {
            textChangedCalled = true ;
            event = e ;
        }

        public void disposing(com.sun.star.lang.EventObject e) {}

    }

    TestTextListener textListener = null;
    /**
    * Retrieves object relation.
    * @throws StatusException If the relation not found.
    */
    public void before() {
        textListener = (TestTextListener)
            tEnv.getObjRelation("TestTextListener");
        if (textListener == null) {
            throw new StatusException(Status.failed("Relation not found"));
        }
    }

    /**
    * First a <code>TextEvent</code> object created and
    * it is passed to <code>textChanged</code> method
    * call. Then a short wait follows for listener already
    * registered at the object to be called. <p>
    * Has <b> OK </b> status if the listener was called with
    * the same <code>TextEvent</code> object as was created
    * before.
    */
    public void _textChanged() {

        boolean result = true ;

        TextEvent event = new TextEvent() ;
        event.dummy1 = 2;
        oObj.textChanged(event);

        try {
            Thread.sleep(200) ;
        } catch (InterruptedException e) {}

        result = textListener.textChangedCalled &&
            textListener.event.dummy1 == 2;

        tRes.tested("textChanged()", result) ;
    }

    /**
    * Forces environment recreation.
    */
    protected void after() {
        disposeEnvironment();
    }


}

