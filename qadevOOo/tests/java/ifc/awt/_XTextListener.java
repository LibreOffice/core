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
    @Override
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

        util.utils.pause(200);

        result = textListener.textChangedCalled &&
            textListener.event.dummy1 == 2;

        tRes.tested("textChanged()", result) ;
    }

    /**
    * Forces environment recreation.
    */
    @Override
    protected void after() {
        disposeEnvironment();
    }


}

