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

package ifc.util;

import lib.MultiMethodTest;

import com.sun.star.util.XRefreshListener;
import com.sun.star.util.XRefreshable;

/**
* Testing <code>com.sun.star.util.XRefreshable</code>
* interface methods :
* <ul>
*  <li><code> refresh()</code></li>
*  <li><code> addRefreshListener()</code></li>
*  <li><code> removeRefreshListener()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.util.XRefreshable
*/
public class _XRefreshable extends MultiMethodTest {

    public XRefreshable oObj = null;

    final boolean listenerCalled[] = new boolean[1];


    /**
    * <code>XRefreshListener</code> implementation which
    * sets a flag when <code>refreshed</code> method is
    * called.
    */
    public class MyRefreshListener implements XRefreshListener {
        public void refreshed (com.sun.star.lang.EventObject e) {
            listenerCalled[0] = true;
        }

        public void disposing (com.sun.star.lang.EventObject obj) {}
    }

    XRefreshListener listener = new MyRefreshListener();

    /**
    * Just adds a listener. <p>
    * Always has <b>OK</b> status.
    */
    public void _addRefreshListener() {

         oObj.addRefreshListener(listener) ;
         tRes.tested("addRefreshListener()", true);
    }

    /**
    * Calls the method and checks if the listener was called. <p>
    * Has <b>OK</b> status if listener's flag is set after call.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addRefreshListener </code> : to have a listener added.</li>
    * </ul>
    */
    public void _refresh() {

        requiredMethod("addRefreshListener()");

        oObj.refresh();

        tRes.tested("refresh()", listenerCalled[0]);
        if (!listenerCalled[0])
            log.println("RefreshListener wasn't called after refresh");

    }

    /**
    * Removes the listener added before and calls <code>refresh</code>
    * method. Listener must not be called. <p>
    * Has <b>OK</b> status if listener's flag isn't changed.
    * <ul>
    *  <li> <code> refresh </code> : listener added must be already
    *    tested.</li>
    * </ul>
    */
    public void _removeRefreshListener() {
        requiredMethod("refresh()");
        listenerCalled[0] = false;

        oObj.removeRefreshListener(listener) ;
        oObj.refresh();

        tRes.tested("removeRefreshListener()", !listenerCalled[0]);
        if (listenerCalled[0])
            log.println("RefreshListener was called after removing");
    }
} // finish class _XRefreshable

