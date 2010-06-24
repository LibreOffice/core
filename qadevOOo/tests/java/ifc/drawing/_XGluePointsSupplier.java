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

package ifc.drawing;

import lib.MultiMethodTest;

import com.sun.star.container.XIndexContainer;
import com.sun.star.drawing.XGluePointsSupplier;

/**
* Testing <code>com.sun.star.drawing.XGluePointsSupplier</code>
* interface methods :
* <ul>
*  <li><code> getGluePoints()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.drawing.XGluePointsSupplier
*/
public class _XGluePointsSupplier extends MultiMethodTest {
    public XGluePointsSupplier oObj = null;

    /**
    * Gets glue points collection. <p>
    * Has <b> OK </b> status if the value returned is not null. <p>
    */
    public void _getGluePoints(){
        boolean result = true;
        XIndexContainer points = null;

        log.println("testing getDrawPages() ... ");

        points = oObj.getGluePoints();
        result = points != null;

        tRes.tested("getGluePoints()", result);
    }

}

