/*************************************************************************
 *
 *  $RCSfile: ChXChartDocument.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 12:23:17 $
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

import com.sun.star.chart.XChartData;
import com.sun.star.chart.XChartDocument;
import com.sun.star.frame.XController;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.view.XSelectionSupplier;

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
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println( "    disposing xChartDoc " );
        xChartDoc.dispose();
        doc2.dispose();
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
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
    protected synchronized TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());

        try {
            log.println( "creating a chartdocument" );
            xChartDoc = SOF.createChartDoc(null);
            log.println("Waiting before opening second document");
            doc2 = SOF.createChartDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }

        // get the chartdocument
        log.println( "getting ChartDocument" );
        XInterface oObj = (XChartDocument) xChartDoc;

        XController cont1 = xChartDoc.getCurrentController();
        XController cont2 = doc2.getCurrentController();

        XSelectionSupplier sel = (XSelectionSupplier)
                    UnoRuntime.queryInterface(XSelectionSupplier.class, cont1);

        log.println( "creating a new environment for chartdocument object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        log.println( "Adding SelectionSupplier and Shape to select for XModel");
        tEnv.addObjRelation("SELSUPP",sel);
        tEnv.addObjRelation("TOSELECT",xChartDoc.getTitle());

        log.println( "adding Controller as ObjRelation for XModel");
        tEnv.addObjRelation("CONT2",cont2);

        log.println( "adding another Diagram as mod relation to environment" );
        tEnv.addObjRelation("DIAGRAM", SOF.createDiagram(xChartDoc, "PieDiagram"));

        log.println( "adding another ChartData as mod relation to environment" );
        XChartData ChartData = doc2.getData();
        tEnv.addObjRelation("CHARTDATA", ChartData);

        return tEnv;
    } // finish method getTestEnvironment


}    // finish class ChXChartDocument

