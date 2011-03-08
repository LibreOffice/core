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

package ifc.chart;

import lib.MultiMethodTest;

import com.sun.star.beans.XPropertySet;
import com.sun.star.chart.XAxisXSupplier;
import com.sun.star.drawing.XShape;

/**
* Testing <code>com.sun.star.chart.XAxisXSupplier</code>
* interface methods :
* <ul>
*  <li><code> getXMainGrid()</code></li>
*  <li><code> getXAxisTitle()</code></li>
*  <li><code> getXAxis()</code></li>
*  <li><code> getXHelpGrid()</code></li>
* </ul> <p>
* @see com.sun.star.chart.XAxisXSupplier
*/
public class _XAxisXSupplier extends MultiMethodTest {

    public XAxisXSupplier oObj = null;
    boolean    result = true;

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value that isn't null. <p>
    */
    public void _getXMainGrid() {
        result = true;

        XPropertySet MGrid = oObj.getXMainGrid();
        result = (MGrid != null);

        tRes.tested("getXMainGrid()", result);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value that isn't null. <p>
    */
    public void _getXAxisTitle() {
        result = true;

        XShape title = oObj.getXAxisTitle();
        result = (title != null);

        tRes.tested("getXAxisTitle()", result);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value that isn't null. <p>
    */
    public void _getXAxis() {
        result = true;

        XPropertySet axis = oObj.getXAxis();
        result = (axis != null);

        tRes.tested("getXAxis()", result);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value that isn't null. <p>
    */
    public void _getXHelpGrid() {
        result = true;

        XPropertySet HGrid = oObj.getXHelpGrid();
        result = (HGrid != null);

        tRes.tested("getXHelpGrid()", result);
    }

}


