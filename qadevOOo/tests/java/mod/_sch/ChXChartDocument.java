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

import com.sun.star.chart.XChartData;
import com.sun.star.chart.XChartDocument;
import com.sun.star.frame.XController;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.view.XSelectionSupplier;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;


/**
* Test for object which is represented by service
* <code>com.sun.star.chart.ChartDocument</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::lang::XComponent</code></li>
*  <li> <code>com::sun::star::frame::XModel</code></li>
*  <li> <code>com::sun::star::chart::XChartDocument</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::chart::ChartTableAddressSupplier</code></li>
*  <li> <code>com::sun::star::chart::ChartDocument</code></li>
* </ul>
* @see com.sun.star.lang.XComponent
* @see com.sun.star.frame.XModel
* @see com.sun.star.chart.XChartDocument
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.chart.ChartTableAddressSupplier
* @see com.sun.star.chart.ChartDocument
* @see ifc.lang._XComponent
* @see ifc.frame._XModel
* @see ifc.chart._XChartDocument
* @see ifc.beans._XPropertySet
* @see ifc.chart._ChartTableAddressSupplier
* @see ifc.chart._ChartDocument
*/
public class ChXChartDocument extends TestCase {
    XChartDocument xChartDoc = null;
    XChartDocument doc2 = null;

    /**
    * Disposes Chart documents.
    */
    @Override
    protected void cleanup(TestParameters Param, PrintWriter log) {
        if( xChartDoc!=null ) {
            log.println( "    closing xChartDoc" );
            util.DesktopTools.closeDoc(xChartDoc);
            xChartDoc = null;
        }
        if( doc2!=null ) {
            log.println( "    closing xChartDoc2" );
            util.DesktopTools.closeDoc(doc2);
            doc2 = null;
        }
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Creates two chart documents and retrieves current controllers from them
    * using the interface <code>XChartDocument</code>. The created documents
    * is the instances of the service <code>com.sun.star.chart.ChartDocument</code>.
    * Obtains the data source of the second created chart and creates
    * a pie diagram.
    * Object relations created :
    * <ul>
    *  <li> <code>'SELSUPP'</code> for
    *      {@link ifc.frame._XModel}(the controller of the first created chart
    *      document)</li>
    *  <li> <code>'TOSELECT'</code> for
    *      {@link ifc.frame._XModel}(the shape of the main title of
    *      the first created chart document)</li>
    *  <li> <code>'CONT2'</code> for
    *      {@link ifc.frame._XModel}(the second created chart document)</li>
    *  <li> <code>'DIAGRAM'</code> for
    *      {@link ifc.chart._XChartDocument}(the created pie diagram)</li>
    *  <li> <code>'CHARTDATA'</code> for
    *      {@link ifc.chart._XChartDocument}(the data source of the second
    *      created chart)</li>
    * </ul>
    * @see com.sun.star.chart.XChartData
    * @see com.sun.star.chart.ChartDocument
    */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters tParam,
                                                                 PrintWriter log) throws Exception {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                     tParam.getMSF());

        log.println("creating a chartdocument");
        xChartDoc = SOF.createChartDoc();
        log.println("Waiting before opening second document");
        doc2 = SOF.createChartDoc();


        // get the chartdocument
        log.println("getting ChartDocument");

        XInterface oObj = xChartDoc;

        XController cont1 = xChartDoc.getCurrentController();
        XController cont2 = doc2.getCurrentController();

        cont1.getFrame().setName("cont1");
        cont2.getFrame().setName("cont2");

        XSelectionSupplier sel = UnoRuntime.queryInterface(
                                         XSelectionSupplier.class, cont1);

        log.println("creating a new environment for chartdocument object");

        TestEnvironment tEnv = new TestEnvironment(oObj);

        log.println("Adding SelectionSupplier and Shape to select for XModel");
        tEnv.addObjRelation("SELSUPP", sel);
        tEnv.addObjRelation("TOSELECT", xChartDoc.getTitle());

        log.println("adding Controller as ObjRelation for XModel");
        tEnv.addObjRelation("CONT2", cont2);

        log.println("adding another Diagram as mod relation to environment");
        tEnv.addObjRelation("DIAGRAM",
                            SOF.createDiagram(xChartDoc, "PieDiagram"));

        log.println("adding another ChartData as mod relation to environment");

        XChartData ChartData = doc2.getData();
        tEnv.addObjRelation("CHARTDATA", ChartData);

        return tEnv;
    } // finish method getTestEnvironment
} // finish class ChXChartDocument
