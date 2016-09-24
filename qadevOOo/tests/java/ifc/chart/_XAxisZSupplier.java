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


