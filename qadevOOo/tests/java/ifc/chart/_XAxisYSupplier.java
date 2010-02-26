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
import com.sun.star.chart.XAxisYSupplier;
import com.sun.star.drawing.XShape;

/**
* Testing <code>com.sun.star.chart.XAxisYSupplier</code>
* interface methods :
* <ul>
*  <li><code> getYMainGrid()</code></li>
*  <li><code> getYAxisTitle()</code></li>
*  <li><code> getYAxis()</code></li>
*  <li><code> getYHelpGrid()</code></li>
* </ul> <p>
* @see com.sun.star.chart.XAxisYSupplier
*/
public class _XAxisYSupplier extends MultiMethodTest {

    public XAxisYSupplier oObj = null;
    boolean                result = true;

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if the method returns
    * value that isn't null. <p>
    */
    public void _getYMainGrid() {
        result = true;

        XPropertySet MGrid = oObj.getYMainGrid();
        result = (MGrid != null);

        tRes.tested("getYMainGrid()", result);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if the method returns
    * value that isn't null. <p>
    */
    public void _getYAxisTitle() {
        result = true;

        XShape title = oObj.getYAxisTitle();
        result = (title != null);

        tRes.tested("getYAxisTitle()", result);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if the method returns
    * value that isn't null. <p>
    */
    public void _getYAxis() {
        result = true;

        XPropertySet axis = oObj.getYAxis();
        result = (axis != null);

        tRes.tested("getYAxis()", result);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if the method returns
    * value that isn't null. <p>
    */
    public void _getYHelpGrid() {
        result = true;

        XPropertySet HGrid = oObj.getYHelpGrid();
        result = (HGrid != null);

        tRes.tested("getYHelpGrid()", result);
    }
} // EOF XAxisYSupplier


