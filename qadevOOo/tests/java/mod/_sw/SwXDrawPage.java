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

package mod._sw;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.InstCreator;
import util.SOfficeFactory;
import util.ShapeDsc;

import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPageSupplier;
import com.sun.star.drawing.XShapes;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.drawing.DrawPage</code>. <p>
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
 *  <li> <code>com::sun::star::drawing::XMasterPageTarget</code></li>
 *  <li> <code>com::sun::star::container::XNamed</code></li>
 *  <li> <code>com::sun::star::drawing::XShapeCombiner</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurrently.
 * @see com.sun.star.drawing.GenericDrawPage
 * @see com.sun.star.drawing.XShapeBinder
 * @see com.sun.star.drawing.XShapeGrouper
 * @see com.sun.star.drawing.XShapes
 * @see com.sun.star.lang.XServiceInfo
 * @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.container.XIndexAccess
 * @see com.sun.star.container.XElementAccess
 * @see com.sun.star.drawing.XMasterPageTarget
 * @see com.sun.star.container.XNamed
 * @see com.sun.star.drawing.XShapeCombiner
 * @see ifc.drawing._GenericDrawPage
 * @see ifc.drawing._XShapeBinder
 * @see ifc.drawing._XShapeGrouper
 * @see ifc.drawing._XShapes
 * @see ifc.lang._XServiceInfo
 * @see ifc.beans._XPropertySet
 * @see ifc.container._XIndexAccess
 * @see ifc.container._XElementAccess
 * @see ifc.drawing._XMasterPageTarget
 * @see ifc.container._XNamed
 * @see ifc.drawing._XShapeCombiner
 */
public class SwXDrawPage extends TestCase {
    XTextDocument xTextDoc;

    /**
    * Creates text document.
    */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );
        log.println( "creating a textdocument" );
        xTextDoc = SOF.createTextDoc( null );
    }

    /**
    * Disposes text document.
    */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested. After
    * obtaining DrawPage object several shapes are inserted to it.
    *     Object relations created :
    * <ul>
    *  <li> <code>'Shape'</code> for
    *      {@link ifc.drawing._XShapes} : shape creator. </li>
    *  <li> <code>'DrawPage'</code> for
    *      {@link ifc.drawing._XShapeBinder},
    *      {@link ifc.drawing._XShapeCombiner},
    *      {@link ifc.drawing._XShapeGrouper} : DrawPage component we are
    *      testing.</li>
    * </ul>
    */
    @Override
    public TestEnvironment createTestEnvironment(
            TestParameters Param, PrintWriter log ) throws StatusException {
        XInterface oObj = null;
        XDrawPageSupplier XDPSupp;
        XDrawPage xDP = null;
        XShapes oShapes = null;

        log.println( "creating a test environment" );
        SOfficeFactory SOF = SOfficeFactory.getFactory( Param.getMSF() );
        XDPSupp = UnoRuntime.queryInterface
            (XDrawPageSupplier.class, xTextDoc);
        xDP = XDPSupp.getDrawPage();
        oObj = xDP;

        log.println( "inserting some Shapes" );
        oShapes = UnoRuntime.queryInterface(XShapes.class, oObj);
        oShapes.add(SOF.createShape(xTextDoc,2000,1500,1000,1000,"Line"));
        oShapes.add(SOF.createShape(xTextDoc,3000,4500,15000,1000,"Ellipse"));
        oShapes.add(SOF.createShape(xTextDoc,5000,3500,7500,5000,"Rectangle"));

        log.println( "creating a new environment for bodytext object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        ShapeDsc sDsc = new ShapeDsc(15000,13500,7500,10000,"Rectangle");
        log.println( "adding Shape as mod relation to environment" );
        tEnv.addObjRelation("Shape", new InstCreator( xTextDoc, sDsc));
        tEnv.addObjRelation("DrawPage", oObj);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SwXDrawPage

