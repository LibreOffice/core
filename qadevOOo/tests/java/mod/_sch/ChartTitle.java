/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartTitle.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2007-09-06 13:58:28 $
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

import com.sun.star.chart.XChartDocument;
import com.sun.star.drawing.XShape;
import com.sun.star.lang.XMultiServiceFactory;

/**
* Test for object which is represented by service
* <code>com.sun.star.chart.ChartTitle</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::lang::XComponent</code></li>
*  <li> <code>com::sun::star::chart::ChartTitle</code></li>
*  <li> <code>com::sun::star::drawing::XShape</code></li>
*  <li> <code>com::sun::star::drawing::XShapeDescriptor</code></li>
*  <li> <code>com::sun::star::style::CharacterProperties</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::drawing::Shape</code></li>
* </ul>
* @see com.sun.star.lang.XComponent
* @see com.sun.star.chart.ChartTitle
* @see com.sun.star.drawing.XShape
* @see com.sun.star.drawing.XShapeDescriptor
* @see com.sun.star.style.CharacterProperties
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.drawing.Shape
* @see ifc.lang._XComponent
* @see ifc.chart._ChartTitle
* @see ifc.drawing._XShape
* @see ifc.drawing._XShapeDescriptor
* @see ifc.style._CharacterProperties
* @see ifc.beans._XPropertySet
* @see ifc.drawing._Shape
*/
public class ChartTitle extends TestCase {
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
    * Retrieves the shape of the main title of the chart document.
    * The retrieved shape is the instance of the service
    * <code>com.sun.star.chart.ChartTitle</code>.
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        // get the Title
        log.println( "getting Title" );
        XShape oObj = (XShape) xChartDoc.getTitle();

        log.println( "creating a new environment for chartdocument object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );
        tEnv.addObjRelation("NoSetSize","sch.ChartTitle");
        return tEnv;
    } // finish method getTestEnvironment

}    // finish class ChXChartObject_Title

