/*************************************************************************
 *
 *  $RCSfile: SdXShape.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 12:27:52 $
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
import util.SOfficeFactory;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.style.XStyle;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
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
* </ul>
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
public class SdXShape extends TestCase {
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
            xDrawDoc = SOF.createDrawDoc(null);
         } catch (com.sun.star.uno.Exception e) {
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
    * Retrieves the collection of the draw pages from the drawing document using
    * the interface <code>XDrawPagesSupplier</code>. Creates a rectangle shape
    * that is the instance of the service <code>com.sun.star.drawing.Shape</code>.
    * Creates and adds several new rectangle shapes and one ellipse shape to
    * the retrieved draw page. Sets and gets some properties of the created
    * rectangle shape.
    * Object relations created :
    * <ul>
    *  <li> <code>'Style1'</code> for
    *      {@link ifc.drawing._XShapeDescriptor}, {@link ifc.drawing._XShape}
    *      (the value of the property 'Style' that was retrived from the created
    *      rectangle shape) </li>
    *  <li> <code>'Style2'</code> for
    *      {@link ifc.drawing._XShapeDescriptor}, {@link ifc.drawing._XShape}
    *      (the value of the property 'Style' that was retrived from the created
    *      ellipse shape) </li>
    * </ul>
    * @see com.sun.star.drawing.XDrawPagesSupplier
    * @see com.sun.star.drawing.Shape
    */
    protected synchronized TestEnvironment createTestEnvironment(
                                    TestParameters Param, PrintWriter log) {

        log.println( "creating a test environment" );

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                    (XMultiServiceFactory)Param.getMSF());

        // get the drawpage of drawing here
        log.println( "getting Drawpage" );
        XDrawPagesSupplier oDPS = (XDrawPagesSupplier)
            UnoRuntime.queryInterface(XDrawPagesSupplier.class, xDrawDoc);
        XDrawPages oDPn = oDPS.getDrawPages();
        XIndexAccess oDPi = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class, oDPn);
        XDrawPage oDP = null;
        try {
            oDP = (XDrawPage) AnyConverter.toObject(
                        new Type(XDrawPage.class),oDPi.getByIndex(0));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't get by index", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't get by index", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't get by index", e);
        }

        //get a Shape
        log.println( "getting Shape" );
        XShapes oShapes = (XShapes) UnoRuntime.queryInterface
            (XShapes.class, oDP);
        XInterface oObj = SOF.createShape
            (xDrawDoc, 5000, 3500, 7500, 5000, "Rectangle");
        for (int i=0; i < 10; i++) {
            oShapes.add(
                SOF.createShape(xDrawDoc,
                    5000, 3500, 7510 + 10 * i, 5010 + 10 * i, "Rectangle"));
        }
        XShape oShape = SOF.createShape
            (xDrawDoc, 3000, 4500, 15000, 1000, "Ellipse");
        oShapes.add((XShape) oObj);
        oShapes.add((XShape) oShape);

        log.println( "creating a new environment for XShape object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        log.println( "adding two style as ObjRelation for ShapeDescriptor" );
        XPropertySet oShapeProps = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class, oObj);
        XStyle aStyle = null;
        try {
            aStyle = (XStyle) AnyConverter.toObject(
                new Type(XStyle.class),oShapeProps.getPropertyValue("Style"));
            oShapeProps.setPropertyValue("ZOrder", new Integer(1));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set or get property value", e);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set or get property value", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set or get property value", e);
        } catch (com.sun.star.beans.PropertyVetoException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set or get property value", e);
        }

        tEnv.addObjRelation("Style1", aStyle);
        oShapeProps = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class, oShape);
        try {
            aStyle = (XStyle) AnyConverter.toObject(
                new Type(XStyle.class),oShapeProps.getPropertyValue("Style"));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get property value", e);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get property value", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get property value", e);
        }

        tEnv.addObjRelation("Style2", aStyle);

        return tEnv;
    } // finish method createTestEnvironment

}    // finish class SdXShape
