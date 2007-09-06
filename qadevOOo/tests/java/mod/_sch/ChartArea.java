/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartArea.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2007-09-06 13:57:30 $
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

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.beans.XPropertySet;
import com.sun.star.chart.XChartDocument;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;

/**
* Test for object which is represented by service
* <code>com.sun.star.chart.ChartArea</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::drawing::FillProperties</code></li>
*  <li> <code>com::sun::star::drawing::LineProperties</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
* </ul>
* The following files used by this test :
* <ul>
*  <li><b> TransparencyChart.sxs </b> : to load predefined chart
*       document where two 'automatic' transparency styles exists :
*       'Transparency 1' and 'Transparency 2'.</li>
* </ul> <p>
* @see com.sun.star.chart.ChartArea
* @see com.sun.star.drawing.FillProperties
* @see com.sun.star.drawing.LineProperties
* @see com.sun.star.beans.XPropertySet
* @see ifc.drawing._FillProperties
* @see ifc.drawing._LineProperties
* @see ifc.beans._XPropertySet
*/
public class ChartArea extends TestCase {
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
        log.println( "    closing xChartDoc " );
        util.DesktopTools.closeDoc(xChartDoc);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieved the instance of the service <code>com.sun.star.chart.ChartArea</code>
    * using the interface <code>XChartDocument</code>.
    * @see com.sun.star.chart.XChartDocument
    * @see com.sun.star.chart.ChartArea
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        // get the Area
        log.println( "getting Area" );
        XPropertySet oObj = (XPropertySet) xChartDoc.getArea();

        log.println( "creating a new environment for chartdocument object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class ChartArea

