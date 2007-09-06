/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChXChartView.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2007-09-06 13:56:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
        log.println( "    closing xChartDoc " );
        util.DesktopTools.closeDoc(xChartDoc);
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

