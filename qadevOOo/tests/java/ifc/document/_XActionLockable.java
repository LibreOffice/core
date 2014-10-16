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

package ifc.document;

import lib.MultiMethodTest;

import com.sun.star.document.XActionLockable;

/**
* Testing <code>com.sun.star.document.XActionLockable</code>
* interface methods :
* <ul>
*  <li><code> isActionLocked()</code></li>
*  <li><code> addActionLock()</code></li>
*  <li><code> removeActionLock()</code></li>
*  <li><code> setActionLocks()</code></li>
*  <li><code> resetActionLocks()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.document.XActionLockable
*/
public class _XActionLockable extends MultiMethodTest {
    public XActionLockable oObj = null;

    /**
     * Method addActionLock called first and then
     * checked value returned by isActionLocked().<p>
     * Has <b> OK </b> status if no runtime exceptions occurred
     * and returned value is true.
     */
    public void _isActionLocked() {
        //determines if at least one lock exists
        oObj.addActionLock();
        boolean result = oObj.isActionLocked();
        tRes.tested("isActionLocked()", result);
    }

    /**
    * Just calls the method. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _addActionLock() {
        requiredMethod("resetActionLocks()");
        oObj.addActionLock();
        tRes.tested("addActionLock()", true);
    }

    /**
    * Calls the method and check value returned by isActionLocked(). <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    * and returned value is false.
    */
    public void _removeActionLock() {
        requiredMethod("addActionLock()");
        oObj.removeActionLock();
        boolean result = ! oObj.isActionLocked();
        tRes.tested("removeActionLock()", result);
    }

    /**
    * Calls the method with specific value. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _setActionLocks() {
        oObj.setActionLocks( nLock );
        tRes.tested("setActionLocks()", true);
    }

    static final short nLock = 8;

    /**
    * Calls the method and checks returned value.<p>
    * Has <b> OK </b> status if the component is not currently
    * locked and returned value is the same as locks number
    * set by <code>setActionLocks</code> method test.
    */
    public void _resetActionLocks() {
        requiredMethod("setActionLocks()");
        short nLocksBeforeReset = oObj.resetActionLocks();
        boolean result = !oObj.isActionLocked() && nLocksBeforeReset == nLock;
        tRes.tested("resetActionLocks()", result);
    }
}// finish class _XActionLockable

