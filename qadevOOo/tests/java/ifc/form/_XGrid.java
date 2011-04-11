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

import com.sun.star.form.XGrid;

/**
* Testing <code>com.sun.star.form.XGrid</code>
* interface methods :
* <ul>
*  <li><code> getCurrentColumnPosition()</code></li>
*  <li><code> setCurrentColumnPosition(short nPos)</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.form.XGrid
*/
public class _XGrid extends MultiMethodTest {
    public XGrid oObj = null;
    public short positionValue;

    /**
    * The method called. Then CurrentColumnPosition is obtained and
    * compared with previously changed value.
    * Has <b> OK </b> status if the method successfully returns
    * and the returned value is correct.
    */
    public void _getCurrentColumnPosition() {
        boolean result;

        requiredMethod("setCurrentColumnPosition()");
        log.println("Testing getCurrentColumnPosition()...");
        if (oObj.getCurrentColumnPosition() == positionValue) {
            result = true;
        } else {
            result = false;
        }
        tRes.tested("getCurrentColumnPosition()", result);
    }

    /**
    * The method called. Then CurrentColumnPosition is obtained, changed
    * and saved.
    * Has <b> OK </b> status if no exceptions was occurred.
    */
    public void _setCurrentColumnPosition() {
        log.println("Testing setCurrentColumnPosition()...");
        if (oObj.getCurrentColumnPosition() != 0) {
            positionValue = 0;
        } else {
            positionValue = 1;
        }
        oObj.setCurrentColumnPosition(positionValue);
        tRes.tested("setCurrentColumnPosition()", true);
    }

}
