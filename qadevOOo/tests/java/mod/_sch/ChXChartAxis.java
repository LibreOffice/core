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

import com.sun.star.beans.XPropertySet;
import com.sun.star.chart.XAxisYSupplier;
import com.sun.star.chart.XChartDocument;
import com.sun.star.drawing.XShape;
import com.sun.star.uno.UnoRuntime;

/**
* Test for object which is represented by service
* <code>com.sun.star.chart.ChartAxis</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::drawing::LineProperties</code></li>
*  <li> <code>com::sun::star::style::CharacterProperties</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::chart::ChartAxis</code></li>
* </ul>
* @see com.sun.star.drawing.LineProperties
* @see com.sun.star.style.CharacterProperties
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.chart.ChartAxis
* @see ifc.drawing._LineProperties
* @see ifc.style._CharacterProperties
* @see ifc.beans._XPropertySet
* @see ifc.chart._ChartAxis
*/
public class ChXChartAxis extends TestCase {
    XChartDocument xChartDoc = null;

    /**
    * Creates Chart document.
    */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF());
        log.println( "creating a chartdocument" );
        xChartDoc = SOF.createChartDoc();
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
    * Retrieves the diagram of the chart document. Then obtains the properties
    * of the y-axis of the diagram using the interface
    * <code>XAxisYSupplier</code>. The obatined property is the instance
    * of the service <code>com.sun.star.chart.ChartAxis</code>.
    * @see com.sun.star.chart.XAxisYSupplier
    * @see com.sun.star.chart.ChartAxis
    */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XPropertySet oObj = null;
        XShape oDiagram = null;

        // create testobject here
        // get the Diagram
        log.println( "getting Diagram" );
        oDiagram = xChartDoc.getDiagram();

        // get the Axis
        log.println( "getting ChartAxis" );
        XAxisYSupplier oAxisSup = UnoRuntime.queryInterface(XAxisYSupplier.class,oDiagram);
        oObj = oAxisSup.getYAxis();

        log.println( "creating a new environment for chartdocument object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        return tEnv;
    } // finish method getTestEnvironment



}    // finish class ChXChartAxis

