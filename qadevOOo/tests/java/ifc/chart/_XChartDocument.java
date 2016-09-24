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
import com.sun.star.chart.XChartData;
import com.sun.star.chart.XChartDocument;
import com.sun.star.chart.XDiagram;
import com.sun.star.drawing.XShape;

/**
* Testing <code>com.sun.star.chart.XChartDocument</code>
* interface methods :
* <ul>
*  <li><code> getTitle()</code></li>
*  <li><code> getSubTitle()</code></li>
*  <li><code> getLegend()</code></li>
*  <li><code> getArea()</code></li>
*  <li><code> getDiagram()</code></li>
*  <li><code> setDiagram()</code></li>
*  <li><code> getData()</code></li>
*  <li><code> attachData()</code></li>
* </ul> <p>
*
* This test needs the following object relations :
* <ul>
*  <li> <code>'DIAGRAM'</code> (of type <code>XDiagram</code>):
*  is used as parameter for method <code>setDiagram</code> </li>
*  <li> <code>'CHARTDATA'</code> (of type <code>XChartData</code>):
*  is used as parameter for method <code>attachData</code> </li>
* </ul>
*
* @see com.sun.star.chart.XChartDocument
*/
public class _XChartDocument extends MultiMethodTest {

    public XChartDocument    oObj = null;
    XDiagram diagram = null;
    XChartData ChartData = null;

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getTitle() {
        XShape title = oObj.getTitle();
        tRes.tested("getTitle()", title != null);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getSubTitle() {
        XShape subtitle = oObj.getSubTitle();
        tRes.tested("getSubTitle()", subtitle != null);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getLegend() {
        XShape legend = oObj.getLegend();
        tRes.tested("getLegend()", legend != null);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getArea() {
        XPropertySet area = oObj.getArea();
        tRes.tested("getArea()", area != null);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getDiagram() {
        diagram = oObj.getDiagram();
        tRes.tested("getDiagram()", diagram != null);
    }

    /**
    * Test compares type of diagram before method call and after.<p>
    * Has <b> OK </b> status if diagram types are not equal. <p>
    *
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getDiagram </code> : to have diagram before method call</li>
    * </ul>
    *
    * @see com.sun.star.chart.XDiagram
    */
    public void _setDiagram() {
        requiredMethod("getDiagram()");
        String oldType = diagram.getDiagramType();
        XDiagram diag = (XDiagram)tEnv.getObjRelation("DIAGRAM");
        oObj.setDiagram(diag);
        String newType = oObj.getDiagram().getDiagramType();
        tRes.tested("setDiagram()", !(oldType.equals(newType)));
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getData() {
        ChartData = oObj.getData();
        tRes.tested("getData()", ChartData != null);
    }

    /**
    * Test compares data before method call and after. <p>
    * Has <b> OK </b> status if the data before method call and
    * after are not equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getData </code> : to have data before method call </li>
    * </ul>
    * @see com.sun.star.chart.XChartData
    */
    public void _attachData() {
        requiredMethod("getData()");
        XChartData data = (XChartData)tEnv.getObjRelation("CHARTDATA");
        oObj.attachData(data);
        XChartData newdata = oObj.getData();

        tRes.tested("attachData()", !(newdata.equals(ChartData)));
    }

}


