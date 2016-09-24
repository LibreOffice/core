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

import lib.MultiPropertyTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.XPropertySet;
import com.sun.star.chart.XChartDocument;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.chart.ChartStatistics</code>
* service properties :
* <ul>
*  <li><code> RegressionCurves</code></li>
*  <li><code> ConstantErrorLow</code></li>
*  <li><code> ConstantErrorHigh</code></li>
*  <li><code> MeanValue</code></li>
*  <li><code> ErrorCategory</code></li>
*  <li><code> PercentageError</code></li>
*  <li><code> ErrorMargin</code></li>
*  <li><code> ErrorIndicator</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'CHARTDOC'</code> (of type <code>XChartDocument</code>):
*  to have reference to chart document </li>
* </ul> <p>
* @see com.sun.star.chart.ChartStatistics
*/
public class _ChartStatistics extends MultiPropertyTest {

    /**
    * Retrieves object relations and prepares a chart document.
    * @throws StatusException if one of relations not found.
    */
    @Override
    protected void before() {
        log.println("Setting Diagram type to LineDiagram");
        XChartDocument doc = (XChartDocument) tEnv.getObjRelation("CHARTDOC");
        if (doc == null) throw new StatusException(Status.failed
            ("Relation 'CHARTDOC' not found"));

        oObj = UnoRuntime.queryInterface( XPropertySet.class, doc.getDiagram() );
    }
}

