/*************************************************************************
 *
 *  $RCSfile: _XChartDocument.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:20:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
    boolean                result = true;
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


