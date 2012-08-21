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
            xChartDoc = SOF.createChartDoc(null);
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

