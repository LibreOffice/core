/*************************************************************************
 *
 *  $RCSfile: ChXChartView.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 12:23:27 $
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
        log.println( "    disposing xChartDoc " );
        xChartDoc.dispose();
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
        oModel = (XModel)UnoRuntime.queryInterface(XModel.class, xChartDoc);
        oObj = (XController)oModel.getCurrentController();

        log.println( "creating a new environment for chartdocument object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("Selections", new Object[]
            {xChartDoc.getArea(), xChartDoc.getDiagram(), xChartDoc.getTitle(),
             xChartDoc.getLegend()} );

        tEnv.addObjRelation("Comparer", new Comparator() {
            public int compare(Object o1, Object o2) {
                XShapeDescriptor descr1 = (XShapeDescriptor)
                    UnoRuntime.queryInterface(XShapeDescriptor.class, o1);
                XShapeDescriptor descr2 = (XShapeDescriptor)
                    UnoRuntime.queryInterface(XShapeDescriptor.class, o2);
                if (descr1 == null || descr2 == null) {
                    return -1;
                }
                if (descr1.getShapeType().equals(descr2.getShapeType())) {
                    return 0;
                }
                return 1;
            }
            public boolean equals(Object obj) {
                return compare(this, obj) == 0;
            }
        } );

        return tEnv;
    } // finish method getTestEnvironment


}    // finish class ChXChartView

