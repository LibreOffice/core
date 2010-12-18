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

package ifc.sheet;

import lib.MultiMethodTest;

import com.sun.star.sheet.XViewFreezable;

/**
* Testing <code>com.sun.star.sheet.XViewFreezable</code>
* interface methods :
* <ul>
*  <li><code> hasFrozenPanes()</code></li>
*  <li><code> freezeAtPosition()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XViewFreezable
*/
public class _XViewFreezable extends MultiMethodTest {

    public XViewFreezable oObj = null;

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _freezeAtPosition() {
        oObj.freezeAtPosition(2, 2);
        tRes.tested("freezeAtPosition()", true);
    }

    /**
    * Test calls the method, checks returned value, unfreezes panes, calls the
    * method and checks returned value again. <p>
    * Has <b> OK </b> status if returned value is true after firts call and
    * returned value is false after second. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> freezeAtPosition() </code> : to freeze panes with the
    *   specified number of columns and rows </li>
    * </ul>
    */
    public void _hasFrozenPanes() {
        requiredMethod("freezeAtPosition()");
        boolean result = oObj.hasFrozenPanes();
        oObj.freezeAtPosition(0,0);
        result &= !oObj.hasFrozenPanes();
        tRes.tested("hasFrozenPanes()", result);
    }


}  // finish class _XViewFreezable


