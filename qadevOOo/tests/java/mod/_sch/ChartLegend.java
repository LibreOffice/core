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

import com.sun.star.chart.XChartDocument;
import com.sun.star.drawing.XShape;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;

/**
* Test for object which is represented by service
* <code>com.sun.star.chart.ChartLegend</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::drawing::FillProperties</code></li>
*  <li> <code>com::sun::star::drawing::XShape</code></li>
*  <li> <code>com::sun::star::drawing::Shape</code></li>
*  <li> <code>com::sun::star::chart::ChartLegend</code></li>
*  <li> <code>com::sun::star::drawing::LineProperties</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::style::CharacterProperties</code></li>
*  <li> <code>com::sun::star::drawing::XShapeDescriptor</code></li>
*  <li> <code>com::sun::star::lang::XComponent</code></li>
* </ul>
* The following files used by this test :
* <ul>
*  <li><b> TransparencyChart.sxs </b> : to load predefined chart
*       document where two 'automatic' transparency styles exists :
*       'Transparency 1' and 'Transparency 2'.</li>
* </ul> <p>
* @see com.sun.star.drawing.FillProperties
* @see com.sun.star.drawing.XShape
* @see com.sun.star.drawing.Shape
* @see com.sun.star.chart.ChartLegend
* @see com.sun.star.drawing.LineProperties
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.style.CharacterProperties
* @see com.sun.star.drawing.XShapeDescriptor
* @see com.sun.star.lang.XComponent
* @see ifc.drawing._FillProperties
* @see ifc.drawing._XShape
* @see ifc.drawing._Shape
* @see ifc.chart._ChartLegend
* @see ifc.drawing._LineProperties
* @see ifc.beans._XPropertySet
* @see ifc.style._CharacterProperties
* @see ifc.drawing._XShapeDescriptor
* @see ifc.lang._XComponent
*/
public class ChartLegend extends TestCase {
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
    * Retrieves the diagram of the chart document. The retrieved
    * diagram is the instance of the service
    * <code>com.sun.star.chart.ChartLegend</code>.
    */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XShape oObj = null;

        // get the Legend
        log.println( "getting Legend" );
        oObj = xChartDoc.getLegend();

        log.println( "creating a new environment for chartdocument object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("NoSetSize", "sch.ChartLegend");
        return tEnv;
    } // finish method getTestEnvironment


}    // finish class ChartLegend

