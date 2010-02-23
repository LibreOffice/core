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

import lib.MultiPropertyTest;

/**
* Testing <code>com.sun.star.chart.ChartDataRowProperties</code>
* service properties :
* <ul>
*  <li><code> Axis</code></li>
*  <li><code> DataErrorProperties</code></li>
*  <li><code> DataMeanValueProperties</code></li>
*  <li><code> DataRegressionProperties</code></li>
* </ul> <p>
* @see com.sun.star.chart.ChartDataRowProperties
*/
public class _ChartDataRowProperties extends MultiPropertyTest {

    public void _Axis() {
        try {
            Integer axis = (Integer) oObj.getPropertyValue("Axis");
            int newValue;
            if (axis.intValue()==2) {
                newValue = 4;
            } else {
                newValue = 2;
            }
            oObj.setPropertyValue("Axis", new Integer(newValue));
            axis = (Integer) oObj.getPropertyValue("Axis");
            tRes.tested("Axis",axis.intValue()==newValue);
        } catch (Exception e) {
            e.printStackTrace(log);
            tRes.tested("Axis",false);
        }
    }
}

