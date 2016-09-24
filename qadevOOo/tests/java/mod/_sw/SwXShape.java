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
import util.SOfficeFactory;

import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPageSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
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
 * threads concurrently.
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
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOF = SOfficeFactory.getFactory( tParam.getMSF() );
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
    * Creating a TestEnvironment for the interfaces to be tested. At first,
    * DrawPage is gotten from text document using <code>XDrawPageSupplier</code>
    * interface. Then shape (rectangle) is created and added to DrawPage
    * obtained before, then returned as a test component.
    */
    @Override
    public TestEnvironment createTestEnvironment(
            TestParameters tParam, PrintWriter log ) throws StatusException {
        XInterface oObj = null;
        XDrawPage oDP = null;
        XShapes oShapes = null;

        log.println( "creating a test environment" );
        log.println( "getting Drawpage" );
        XDrawPageSupplier oDPS = UnoRuntime.queryInterface(XDrawPageSupplier.class, xTextDoc);
        oDP = oDPS.getDrawPage();

        log.println( "getting Shape" );
        oShapes = UnoRuntime.queryInterface(XShapes.class, oDP);
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

