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

    }

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


