/*************************************************************************
 *
 *  $RCSfile: ChXDataRow.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 12:23:45 $
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
* <code>com.sun.star.chart.ChartDataRowProperties</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::drawing::FillProperties</code></li>
*  <li> <code>com::sun::star::chart::ChartStatistics</code></li>
*  <li> <code>com::sun::star::chart::ChartDataRowProperties</code></li>
*  <li> <code>com::sun::star::drawing::LineProperties</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::chart::ChartDataPointProperties</code></li>
*  <li> <code>com::sun::star::chart::Chart3DBarProperties</code></li>
*  <li> <code>com::sun::star::style::CharacterProperties</code></li>
* </ul>
* The following files used by this test :
* <ul>
*  <li><b> TransparencyChart.sxs </b> : to load predefined chart
*       document where two 'automatic' transparency styles exists :
*       'Transparency 1' and 'Transparency 2'.</li>
* </ul> <p>
* @see com.sun.star.drawing.FillProperties
* @see com.sun.star.chart.ChartStatistics
* @see com.sun.star.chart.ChartDataRowProperties
* @see com.sun.star.drawing.LineProperties
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.chart.ChartDataPointProperties
* @see com.sun.star.chart.Chart3DBarProperties
* @see com.sun.star.style.CharacterProperties
* @see ifc.drawing._FillProperties
* @see ifc.chart._ChartStatistics
* @see ifc.chart._ChartDataRowProperties
* @see ifc.drawing._LineProperties
* @see ifc.beans._XPropertySet
* @see ifc.chart._ChartDataPointProperties
* @see ifc.chart._Chart3DBarProperties
* @see ifc.style._CharacterProperties
*/
public class ChXDataRow extends TestCase {
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
    protected synchronized void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xChartDoc " );
        xChartDoc.dispose();
    }


    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves the diagram of the chart document. Obtains the properties of
    * the specified data row. The obtained properties is the instance of
    * the service <code>com.sun.star.chart.ChartDataRowProperties</code>.
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
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XPropertySet  oObj = null;
        XDiagram oDiagram = null;
        // get the ChXDataRowPoint_Row
        try {
            log.println( "getting ChXDataRowPoint_Row" );
            oDiagram = (XDiagram) xChartDoc.getDiagram();
            oObj = (XPropertySet) oDiagram.getDataRowProperties(1);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't get ChXDataRowPoint_Row", e );
        }

        log.println( "creating a new environment for chartdocument object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());

        Object line = SOF.createDiagram(xChartDoc,"XYDiagram");
        tEnv.addObjRelation("LINE",line);

        Object bar = SOF.createDiagram(xChartDoc,"BarDiagram");
        tEnv.addObjRelation("BAR",bar);

        log.println( "adding ChartDocument as mod relation to environment" );
        tEnv.addObjRelation("CHARTDOC", xChartDoc);

        return tEnv;
    } // finish method getTestEnvironment


}    // finish class ChXDataRow

