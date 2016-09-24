/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package mod._sd;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.InstCreator;
import util.SOfficeFactory;
import util.ShapeDsc;

import com.sun.star.container.XIndexAccess;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XShapes;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.drawing.GenericDrawPage</code>. <p>
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
public class SdGenericDrawPage extends TestCase {
    XComponent xDrawDoc;

    /**
    * Creates Drawing document.
    */
    @Override
    protected void initialize(TestParameters Param, PrintWriter log) throws Exception {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                Param.getMSF());

        log.println( "creating a draw document" );
        xDrawDoc = SOF.createDrawDoc(null);
    }

    /**
    * Disposes Drawing document.
    */
    @Override
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println("disposing xDrawDoc");
        util.DesktopTools.closeDoc(xDrawDoc);
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Retrieves the collection of the draw pages from the drawing document using
    * the interface <code>XDrawPagesSupplier</code> and take one of them.
    * The retrieved draw page is the instance of the service
    * <code>com.sun.star.drawing.DrawPage</code>. Inserts some shapes
    * into the draw page. Creates the rectangle shape.
    * Object relations created :
    * <ul>
    *  <li> <code>'Shape'</code> for
    *      {@link ifc.drawing._XShapes}( the created rectangle shape )</li>
    *  <li> <code>'DrawPage'</code> for
    *      {@link ifc.drawing._XShapeBinder}, {@link ifc.drawing._XShapeGrouper},
    *      {@link ifc.drawing._XShapeCombiner}( the retrieved draw page )</li>
    * </ul>
    * @see com.sun.star.drawing.DrawPage
    * @see com.sun.star.drawing.XDrawPagesSupplier
    */
    @Override
    protected TestEnvironment createTestEnvironment(
                                    TestParameters Param, PrintWriter log) throws Exception {

        XInterface oObj = null;
        XShapes oShapes = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                    Param.getMSF());

        // get the drawpage of drawing here
        log.println( "getting Drawpage" );
        XDrawPagesSupplier oDPS = UnoRuntime.queryInterface(XDrawPagesSupplier.class,xDrawDoc);
        XDrawPages oDPn = oDPS.getDrawPages();
        XIndexAccess oDPi = UnoRuntime.queryInterface
            (XIndexAccess.class,oDPn);
        oObj = (XDrawPage) AnyConverter.toObject(
                    new Type(XDrawPage.class),oDPi.getByIndex(0));

        //put something on the drawpage
        log.println( "inserting some Shapes" );
        oShapes = UnoRuntime.queryInterface(XShapes.class,oObj);
        oShapes.add(SOF.createShape
            (xDrawDoc, 2000, 1500, 1000, 1000, "Line"));
        oShapes.add(SOF.createShape
            (xDrawDoc, 3000, 4500, 15000, 1000, "Ellipse"));
        oShapes.add(SOF.createShape
            (xDrawDoc, 5000, 3500, 7500, 5000, "Rectangle"));

        log.println( "creating a new environment for drawpage object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        ShapeDsc sDsc = new ShapeDsc(5000, 3500, 7500, 10000, "Rectangle");
        log.println( "adding Shape as mod relation to environment" );
        tEnv.addObjRelation("Shape", new InstCreator( xDrawDoc, sDsc));

        // adding relation for XShapeGrouper
        tEnv.addObjRelation("DrawPage", oObj);

        return tEnv;
    } // finish method createTestEnvironment


}    // finish class SdGenericDrawPage

