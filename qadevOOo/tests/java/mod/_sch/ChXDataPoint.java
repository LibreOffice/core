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
import util.utils;

import com.sun.star.beans.XPropertySet;
import com.sun.star.chart.XChartDocument;
import com.sun.star.chart.XDiagram;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
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
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());

        try {
            log.println( "creating a chartdocument" );
            XComponent xComp = SOF.loadDocument(
                             utils.getFullTestURL("TransparencyChart.sxs"));
            xChartDoc = (XChartDocument)
                UnoRuntime.queryInterface(XChartDocument.class,xComp);
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
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XPropertySet  oObj = null;
        XDiagram oDiagram = null;

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());

        // get the DataRowPoint_Point
        try {
            log.println( "getting ChXDataRowPoint_Point" );
            oDiagram = (XDiagram) xChartDoc.getDiagram();
            oObj = (XPropertySet) oDiagram.getDataPointProperties(1,1);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't get ChXDataRowPoint_Point", e );
        }

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

