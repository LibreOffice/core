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

package ifc.form;

import lib.MultiMethodTest;

import com.sun.star.form.XBoundControl;

/**
* Testing <code>com.sun.star.form.XBoundControl</code>
* interface methods:
* <ul>
*  <li><code> getLock() </code></li>
*  <li><code> setLock() </code></li>
* </ul><p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.form.XBoundControl
*/
public class _XBoundControl extends MultiMethodTest {
    public XBoundControl oObj = null;
    public boolean defaultState = false;

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _getLock() {
        boolean res = false;
        defaultState = oObj.getLock();
        res = true;
        tRes.tested("getLock()",res);
    }

    /**
    * Test calls the method, then result is checked. <p>
    * Has <b> OK </b> status if method locks input (set and gotten values are
    * equal). <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getLock() </code>: determines whether the input is
    *  currently locked or not </li>
    * </ul>
    */
    public void _setLock() {
        boolean res = false;
        boolean toSet = !defaultState;

        requiredMethod("getLock()");
        oObj.setLock(toSet);
        res = (oObj.getLock() != defaultState);
        oObj.setLock(defaultState);
        tRes.tested("setLock()", res);
    }

} //EOF of XBoundControl

