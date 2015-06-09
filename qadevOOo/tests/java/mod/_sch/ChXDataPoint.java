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

package mod._sch;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.beans.XPropertySet;
import com.sun.star.chart.XChartDocument;
import com.sun.star.chart.XDiagram;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;

/**
* Test for object which is represented by service
* <code>com.sun.star.chart.ChartDataPointProperties</code>. <p>
* Object implements the following interfaces
* <ul>
*  <li> <code>com::sun::star::chart::ChartDataPointProperties</code></li>
*  <li> <code>com::sun::star::drawing::FillProperties</code></li>
*  <li> <code>com::sun::star::drawing::LineProperties</code></li>
*  <li> <code>com::sun::star::style::CharacterProperties</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::chart::Chart3DBarProperties</code></li>
* </ul>
* The following files used by this test :
* <ul>
*  <li><b> TransparencyChart.sxs </b> : to load predefined chart
*       document where two 'automatic' transparency styles exists :
*       'Transparency 1' and 'Transparency 2'.</li>
* </ul> <p>
* @see com.sun.star.chart.ChartDataPointProperties
* @see com.sun.star.drawing.FillProperties
* @see com.sun.star.drawing.LineProperties
* @see com.sun.star.style.CharacterProperties
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.chart.Chart3DBarProperties
* @see ifc.chart._ChartDataPointProperties
* @see ifc.drawing._FillProperties
* @see ifc.drawing._LineProperties
* @see ifc.style._CharacterProperties
* @see ifc.beans._XPropertySet
* @see ifc.chart._Chart3DBarProperties
*/
public class ChXDataPoint extends TestCase {
    XChartDocument xChartDoc = null;

    /**
    * Creates Chart document.
    */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF());
        log.println( "creating a chartdocument" );
        XComponent xComp = SOF.loadDocument(
                         utils.getFullTestURL("TransparencyChart.sxs"));
        xChartDoc = UnoRuntime.queryInterface(XChartDocument.class,xComp);
    }

    /**
    * Disposes Chart document.
    */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        if( xChartDoc!=null ) {
            log.println( "    closing xChartDoc" );
            util.DesktopTools.closeDoc(xChartDoc);
            xChartDoc = null;
        }
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Retrieves the diagram of the chart document. Obtains the properties of
    * the specified data point. The obtained properties is the instance of
    * the service <code>com.sun.star.chart.ChartDataPointProperties</code>.
    * Creates a XY-diagram and bar-diagram also.
    * Object relations created :
    * <ul>
    *  <li> <code>'LINE'</code> for
    *      {@link ifc.chart._ChartDataPointProperties}(the created XY-diagram)</li>
    *  <li> <code>'CHARTDOC'</code> for
    *      {@link ifc.chart._ChartDataPointProperties},
    *      {@link ifc.chart._Chart3DBarProperties}(the chart document)</li>
    *  <li> <code>'BAR'</code> for
    *      {@link ifc.chart._Chart3DBarProperties}(the created bar-diagram)</li>
    * </ul>
    */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) throws Exception {

        XPropertySet  oObj = null;
        XDiagram oDiagram = null;

        SOfficeFactory SOF = SOfficeFactory.getFactory( Param.getMSF());

        // get the DataRowPoint_Point
        log.println( "getting ChXDataRowPoint_Point" );
        oDiagram = xChartDoc.getDiagram();
        oObj = oDiagram.getDataPointProperties(1,1);

        log.println( "creating a new environment for chartdocument object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        Object line = SOF.createDiagram(xChartDoc,"XYDiagram");
        tEnv.addObjRelation("LINE",line);

        Object bar = SOF.createDiagram(xChartDoc,"BarDiagram");
        tEnv.addObjRelation("BAR",bar);

        log.println( "adding ChartDocument as mod relation to environment" );
        tEnv.addObjRelation("CHARTDOC", xChartDoc);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class ChXDataPoint

