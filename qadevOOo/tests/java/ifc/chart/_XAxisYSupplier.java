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


