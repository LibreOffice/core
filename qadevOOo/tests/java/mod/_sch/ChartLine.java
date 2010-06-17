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
import com.sun.star.chart.XChartDocument;
import com.sun.star.chart.XDiagram;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;

/**
* Test for object which is represented by service
* <code>com.sun.star.chart.ChartLine</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::drawing::LineProperties</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
* </ul>
* @see com.sun.star.drawing.LineProperties
* @see com.sun.star.beans.XPropertySet
* @see ifc.drawing._LineProperties
* @see ifc.beans._XPropertySet
*/
public class ChartLine extends TestCase {
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
    * Creates a bar diagram and sets the created diagram for the chart document.
    * Retrieves the property <code>'DataMeanValueProperties'</code> of
    * the specified data row. The retrieved property is the instance of
    * the service <code>com.sun.star.chart.ChartLine</code>.
    * @see com.sun.star.chart.ChartLine
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XPropertySet oObj = null;
        XDiagram oDiagram = null;
        SOfficeFactory SOF = null;

        //get LineDiagram
        SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());
        oDiagram = SOF.createDiagram(xChartDoc, "LineDiagram");

        log.println( "getting Line-Diagram" );
        xChartDoc.setDiagram(oDiagram);

        // get the Line
        try {
            log.println( "getting Line" );
            XPropertySet RowProps = oDiagram.getDataRowProperties(1);
            RowProps.setPropertyValue("MeanValue", new Boolean( true ));
            oObj = (XPropertySet) AnyConverter.toObject(
                new Type(XPropertySet.class),
                    RowProps.getPropertyValue("DataMeanValueProperties"));
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't get Line", e );
        } catch (com.sun.star.lang.WrappedTargetException e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't get Line", e );
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't get Line", e );
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't get Line", e );
        }
        catch(com.sun.star.beans.PropertyVetoException e) {
             // Some exception occures.FAILED
             e.printStackTrace( log );
             throw new StatusException( "Couldn't get Line", e );
         }

        log.println( "creating a new environment for chartdocument object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );


        return tEnv;
    } // finish method getTestEnvironment


}    // finish class ChartLine

