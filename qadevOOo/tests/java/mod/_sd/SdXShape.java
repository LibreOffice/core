/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
        util.DesktopTools.closeDoc(xDrawDoc);;
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
