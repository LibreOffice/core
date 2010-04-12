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

package ifc.linguistic2;

import lib.MultiMethodTest;

import com.sun.star.lang.EventObject;
import com.sun.star.linguistic2.LinguServiceEvent;
import com.sun.star.linguistic2.XLinguServiceEventBroadcaster;
import com.sun.star.linguistic2.XLinguServiceEventListener;

/**
* Testing <code>com.sun.star.linguistic2.XLinguServiceEventBroadcaster</code>
* interface methods:
* <ul>
*   <li><code>addLinguServiceEventListener()</code></li>
*   <li><code>removeLinguServiceEventListener()</code></li>
* </ul><p>
* @see com.sun.star.linguistic2.XLinguServiceEventBroadcaster
*/
public class _XLinguServiceEventBroadcaster extends MultiMethodTest {

    public XLinguServiceEventBroadcaster oObj = null;

    /**
    * Class implements interface <code>XLinguServiceEventListener</code>
    * for test method <code>addLinguServiceEventListener</code>.
    * @see com.sun.star.linguistic2.XLinguServiceEventListener
    */
    public class MyLinguServiceEventListener implements
            XLinguServiceEventListener {
        public void disposing ( EventObject oEvent ) {
            log.println("Listener has been disposed");
        }
        public void processLinguServiceEvent(LinguServiceEvent aServiceEvent) {
            log.println("Listener called");
        }

    };

    XLinguServiceEventListener listener = new MyLinguServiceEventListener();

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value is true. <p>
    */
    public void _addLinguServiceEventListener() {
        boolean res = true;

        res = oObj.addLinguServiceEventListener(listener);

        tRes.tested("addLinguServiceEventListener()",res);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value is true. <p>
    */
    public void _removeLinguServiceEventListener() {
        boolean res = true;

        res = oObj.removeLinguServiceEventListener(listener);

        tRes.tested("removeLinguServiceEventListener()",res);
    }

}  // finish class XLinguServiceEventBroadcaster


