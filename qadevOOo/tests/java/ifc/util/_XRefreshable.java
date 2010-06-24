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
* Test is <b> NOT </b> multithread compilant. <p>
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

