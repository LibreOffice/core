/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwXShape.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:51:06 $
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

package mod._sw;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPageSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.drawing.Shape</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::lang::XComponent</code></li>
 *  <li> <code>com::sun::star::drawing::XShape</code></li>
 *  <li> <code>com::sun::star::drawing::XShapeDescriptor</code></li>
 *  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 *  <li> <code>com::sun::star::drawing::Shape</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.drawing.XShape
 * @see com.sun.star.drawing.XShapeDescriptor
 * @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.drawing.Shape
 * @see ifc.lang._XComponent
 * @see ifc.drawing._XShape
 * @see ifc.drawing._XShapeDescriptor
 * @see ifc.beans._XPropertySet
 * @see ifc.drawing._Shape
 */
public class SwXShape extends TestCase {
    XTextDocument xTextDoc;
    SOfficeFactory SOF;

    /**
    * Creates text document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );
        try {
            log.println( "creating a textdocument" );
            xTextDoc = SOF.createTextDoc( null );
        } catch ( com.sun.star.uno.Exception e ) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
    * Disposes text document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested. At first,
    * DrawPage is gotten from text document using <code>XDrawPageSupplier</code>
    * interface. Then shape (rectangle) is created and added to DrawPage
    * obtained before, then returned as a test component.
    */
    public synchronized TestEnvironment createTestEnvironment(
            TestParameters tParam, PrintWriter log ) throws StatusException {
        XInterface oObj = null;
        XDrawPage oDP = null;
        XShapes oShapes = null;

        log.println( "creating a test environment" );
        log.println( "getting Drawpage" );
        XDrawPageSupplier oDPS = (XDrawPageSupplier)
            UnoRuntime.queryInterface(XDrawPageSupplier.class, xTextDoc);
        oDP = oDPS.getDrawPage();

        log.println( "getting Shape" );
        oShapes = (XShapes) UnoRuntime.queryInterface(XShapes.class, oDP);
        oObj = SOF.createShape(xTextDoc,5000,3500,7500,5000,"Rectangle");
        oShapes.add((XShape) oObj);

        for ( int i = 0; i < 9; i++){
            XInterface oShape = SOF.createShape(xTextDoc,
                5000 + 100*i,3500,7500,5000,"Rectangle");
            oShapes.add((XShape) oShape);
        }

        log.println( "creating a new environment for XShape object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SwXShape

