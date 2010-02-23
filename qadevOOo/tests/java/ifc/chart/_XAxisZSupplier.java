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
import com.sun.star.chart.XAxisZSupplier;
import com.sun.star.drawing.XShape;

/**
* Testing <code>com.sun.star.chart.XAxisZSupplier</code>
* interface methods :
* <ul>
*  <li><code> getZMainGrid()</code></li>
*  <li><code> getZAxisTitle()</code></li>
*  <li><code> getZAxis()</code></li>
*  <li><code> getZHelpGrid()</code></li>
* </ul> <p>
* @see com.sun.star.chart.XAxisZSupplier
*/
public class _XAxisZSupplier extends MultiMethodTest {
    public XAxisZSupplier oObj = null;
    boolean            result = true;

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if the method returns
    * value that isn't null. <p>
    */
    public void _getZMainGrid() {
        result = true;

        XPropertySet MGrid = oObj.getZMainGrid();
        result = (MGrid != null);

        tRes.tested("getZMainGrid()", result);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if the method returns
    * value that isn't null. <p>
    */
    public void _getZAxisTitle() {
        result = true;

        XShape title = oObj.getZAxisTitle();
        result = (title != null);

        tRes.tested("getZAxisTitle()", result);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if the method returns
    * value that isn't null. <p>
    */
    public void _getZAxis() {
        result = true;

        XPropertySet axis = oObj.getZAxis();
        result = (axis != null);

        tRes.tested("getZAxis()", result);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if the method returns
    * value that isn't null. <p>
    */
    public void _getZHelpGrid() {
        result = true;

        XPropertySet HGrid = oObj.getZHelpGrid();
        result = (HGrid != null);

        tRes.tested("getZHelpGrid()", result);
    }

} // EOF XAxisZSupplier


