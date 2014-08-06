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
import com.sun.star.chart.XTwoAxisYSupplier;

/**
* Testing <code>com.sun.star.chart.XTwoAxisYSupplier</code>
* interface methods :
* <ul>
*  <li><code> getSecondaryYAxis()</code></li>
* </ul> <p>
* After test completion object environment has to be recreated.
* @see com.sun.star.chart.XTwoAxisYSupplier
*/
public class _XTwoAxisYSupplier extends MultiMethodTest {

    public XTwoAxisYSupplier oObj = null;
    boolean            result = true;

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getSecondaryYAxis() {
        result = true;

        XPropertySet SecYAxis = oObj.getSecondaryYAxis();
        result = (SecYAxis != null);

        tRes.tested("getSecondaryYAxis()", result);
    }

    /**
    * Forces object environment recreation.
    */
    @Override
    protected void after() {
        disposeEnvironment();
    }

} // EOF XTwoAxisYSupplier


