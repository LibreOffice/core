/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartLine.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2007-09-06 13:58:14 $
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
        log.println( "    closing xChartDoc " );
        util.DesktopTools.closeDoc(xChartDoc);
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

