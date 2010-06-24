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

package mod._sch;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.beans.XPropertySet;
import com.sun.star.chart.XAxisYSupplier;
import com.sun.star.chart.XChartDocument;
import com.sun.star.chart.XDiagram;
import com.sun.star.drawing.XShape;
import com.sun.star.lang.XMultiServiceFactory;
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
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());

        try {
            log.println( "creating a chartdocument" );
            xChartDoc = SOF.createChartDoc(null);;
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
    * Disposes Chart document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        if( xChartDoc!=null ) {
            log.println( "    closing xChartDoc" );
            util.DesktopTools.closeDoc(xChartDoc);
            xChartDoc = null;
        }
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves the diagram of the chart document. Then obtains the properties
    * of the y-axis of the diagram using the interface
    * <code>XAxisYSupplier</code>. The obatined property is the instance
    * of the service <code>com.sun.star.chart.ChartAxis</code>.
    * @see com.sun.star.chart.XAxisYSupplier
    * @see com.sun.star.chart.ChartAxis
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XPropertySet oObj = null;
        XShape oDiagram = null;

        // create testobject here
        // get the Diagram
        log.println( "getting Diagram" );
        oDiagram = (XDiagram) xChartDoc.getDiagram();

        // get the Axis
        log.println( "getting ChartAxis" );
        XAxisYSupplier oAxisSup = (XAxisYSupplier)
            UnoRuntime.queryInterface(XAxisYSupplier.class,oDiagram);
        oObj = (XPropertySet) oAxisSup.getYAxis();

        log.println( "creating a new environment for chartdocument object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        return tEnv;
    } // finish method getTestEnvironment



}    // finish class ChXChartAxis

