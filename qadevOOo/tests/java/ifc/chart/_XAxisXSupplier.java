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


