/*************************************************************************
 *
 *  $RCSfile: SdMasterPage.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 12:26:25 $
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

package mod._sd;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.InstCreator;
import util.SOfficeFactory;
import util.ShapeDsc;

import com.sun.star.container.XIndexAccess;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XMasterPagesSupplier;
import com.sun.star.drawing.XShapes;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.drawing.MasterPage</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::drawing::GenericDrawPage</code></li>
*  <li> <code>com::sun::star::drawing::XShapeBinder</code></li>
*  <li> <code>com::sun::star::drawing::XShapeGrouper</code></li>
*  <li> <code>com::sun::star::drawing::XShapes</code></li>
*  <li> <code>com::sun::star::lang::XServiceInfo</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::container::XIndexAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::drawing::XShapeCombiner</code></li>
*  <li> <code>com::sun::star::container::XNamed</code></li>
* </ul>
* @see com.sun.star.drawing.MasterPage
* @see com.sun.star.drawing.GenericDrawPage
* @see com.sun.star.drawing.XShapeBinder
* @see com.sun.star.drawing.XShapeGrouper
* @see com.sun.star.drawing.XShapes
* @see com.sun.star.lang.XServiceInfo
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.drawing.XShapeCombiner
* @see com.sun.star.container.XNamed
* @see ifc.drawing._GenericDrawPage
* @see ifc.drawing._XShapeBinder
* @see ifc.drawing._XShapeGrouper
* @see ifc.drawing._XShapes
* @see ifc.lang._XServiceInfo
* @see ifc.beans._XPropertySet
* @see ifc.container._XIndexAccess
* @see ifc.container._XElementAccess
* @see ifc.drawing._XShapeCombiner
* @see ifc.container._XNamed
*/
public class SdMasterPage extends TestCase {
    XComponent xDrawDoc;

    /**
    * Creates Drawing document.
    */
    protected void initialize(TestParameters Param, PrintWriter log) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                    (XMultiServiceFactory)Param.getMSF());

        try {
            log.println( "creating a draw document" );
            xDrawDoc = SOF.createDrawDoc(null);;
         } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException("Couldn't create document", e);
         }
    }

    /**
    * Disposes Drawing document.
    */
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println("disposing xDrawDoc");
        xDrawDoc.dispose();
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves the collection of the master pages from the document using the
    * interface <code>XMasterPagesSupplier</code> and takes
    * one of them. Inserts some shapes into the document. Creates a rectangle
    * shape. The retrieved master page is the instance of the service
    * <code>com.sun.star.drawing.MasterPage</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'DrawPage'</code> for
    *      {@link ifc.drawing._XShapeBinder}, {@link ifc.drawing._XShapeGrouper},
    *      {@link ifc.drawing._XShapeCombiner}( the draw page of the retrieved
    *      master page)</li>
    *  <li> <code>'Shape'</code> for
    *      {@link ifc.drawing._XShapes}(the created rectangle shape)</li>
    * </ul>
    * @see com.sun.star.drawing.XMasterPagesSupplier
    */
    protected TestEnvironment createTestEnvironment(
                                    TestParameters Param, PrintWriter log) {

        log.println( "creating a test environment" );

        // get the MasterPages here
        log.println( "getting MasterPages" );
        XMasterPagesSupplier oMPS = (XMasterPagesSupplier)
            UnoRuntime.queryInterface(XMasterPagesSupplier.class, xDrawDoc);
        XDrawPages oMPn = oMPS.getMasterPages();
        XIndexAccess oMPi = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class, oMPn);

        log.println( "getting MasterPage" );
        XInterface oObj = null;
        try {
            oObj = (XDrawPage) AnyConverter.toObject(
                        new Type(XDrawPage.class),oMPi.getByIndex(0));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't get MasterPage by index", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't get MasterPage by index", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't get MasterPage by index", e);
        }

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                    (XMultiServiceFactory)Param.getMSF());

        //put something on the masterpage
        log.println( "inserting some Shapes" );
        XShapes oShapes = (XShapes)
            UnoRuntime.queryInterface(XShapes.class, oObj);
        oShapes.add(SOF.createShape(xDrawDoc, 2000, 1500, 1000, 1000, "Line"));
        oShapes.add(SOF.createShape
            (xDrawDoc, 3000, 4500, 15000, 1000, "Ellipse"));
        oShapes.add(SOF.createShape
            (xDrawDoc, 5000, 3500, 7500, 5000, "Rectangle"));

        log.println( "creating a new environment for MasterPage object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        log.println( "adding DrawPage as mod relation to environment" );
        tEnv.addObjRelation("DrawPage", oObj);

        ShapeDsc sDsc = new ShapeDsc(5000, 3500, 7500, 10000, "Rectangle");
        log.println( "adding Shape as mod relation to environment" );
        tEnv.addObjRelation("Shape", new InstCreator( xDrawDoc, sDsc));

        return tEnv;
    } // finish method createTestEnvironment

}    // finish class SdMasterPage

