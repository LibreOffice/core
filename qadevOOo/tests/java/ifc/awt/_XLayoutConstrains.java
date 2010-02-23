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

import com.sun.star.awt.Size;
import com.sun.star.awt.XLayoutConstrains;

/**
* Testing <code>com.sun.star.awt.XLayoutConstrains</code>
* interface methods:
* <ul>
*   <li><code> getMinimumSize() </code></li>
*   <li><code> getPreferredSize() </code></li>
*   <li><code> calcAdjustedSize() </code></li>
* </ul><p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XLayoutConstrains
*/
public class _XLayoutConstrains extends MultiMethodTest {
    public XLayoutConstrains oObj = null;

    /**
    * Test calls the method, then check if returned value is not null.<p>
    * Has <b> OK </b> status if the method returns not null.
    */
    public void _getMinimumSize() {
        Size aSize = oObj.getMinimumSize();
        tRes.tested("getMinimumSize()", aSize != null);
    }

    /**
    * Test calls the method, then check if returned value is not null.<p>
    * Has <b> OK </b> status if the method returns not null.
    */
    public void _getPreferredSize() {
        Size aSize = oObj.getPreferredSize();
        tRes.tested("getPreferredSize()", aSize != null);
    }

    /**
    * Test calls the method with the new size as a parameter.<p>
    * Has <b> OK </b> status if the method returns not null.
    */
    public void _calcAdjustedSize() {
        Size aSize = oObj.calcAdjustedSize(new Size(50,50));
        tRes.tested("calcAdjustedSize()", aSize != null);
    }

}

