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
import java.util.Comparator;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.chart.XChartDocument;
import com.sun.star.drawing.XShapeDescriptor;
import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;

/**
* Test for object which is represented by service
* <code>com.sun.star.view.OfficeDocumentView</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::view::XViewSettingsSupplier</code></li>
*  <li> <code>com::sun::star::view::XControlAccess</code></li>
*  <li> <code>com::sun::star::view::XSelectionSupplier</code></li>
* </ul>
* @see com.sun.star.view.OfficeDocumentView
* @see com.sun.star.view.XViewSettingsSupplier
* @see com.sun.star.view.XControlAccess
* @see com.sun.star.view.XSelectionSupplier
* @see ifc.view._XViewSettingsSupplier
* @see ifc.view._XControlAccess
* @see ifc.view._XSelectionSupplier
*/
public class ChXChartView extends TestCase {
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
    * Retrieves the current controller of the chart document using
    * the interface <code>XModel</code>.The retrieved controller is the instance
    * of the service  <code>com.sun.star.view.OfficeDocumentView</code>.
    * Obtains the main title and the legend of the chart document.
    * Object relations created :
    * <ul>
    *
    * </ul>
    * @see com.sun.star.frame.XModel
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XController oObj = null;
        XModel oModel = null;

        // get the ChartView
        log.println( "getting ChartView" );
        oModel = UnoRuntime.queryInterface(XModel.class, xChartDoc);
        oObj = oModel.getCurrentController();

        log.println( "creating a new environment for chartdocument object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("Selections", new Object[]
            {xChartDoc.getArea(), xChartDoc.getDiagram(), xChartDoc.getTitle(),
             xChartDoc.getLegend()} );

        tEnv.addObjRelation("Comparer", new Comparator<Object>() {
            public int compare(Object o1, Object o2) {
                XShapeDescriptor descr1 = UnoRuntime.queryInterface(XShapeDescriptor.class, o1);
                XShapeDescriptor descr2 = UnoRuntime.queryInterface(XShapeDescriptor.class, o2);
                if (descr1 == null || descr2 == null) {
                    return -1;
                }
                if (descr1.getShapeType().equals(descr2.getShapeType())) {
                    return 0;
                }
                return 1;
            }
        } );

        return tEnv;
    } // finish method getTestEnvironment


}    // finish class ChXChartView

