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

package mod._sc;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DrawTools;
import util.SOfficeFactory;

import com.sun.star.drawing.XShape;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class ScShapeObj extends TestCase {

    static XComponent xSheetDoc;

    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );
        log.println( "creating a sheetdoc" );
        xSheetDoc = UnoRuntime.queryInterface(XComponent.class, SOF.createCalcDoc(null));
    }

    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        util.DesktopTools.closeDoc(xSheetDoc);
    }

    /**
     * Creating a TestEnvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.drawing.EllipseShape</code> as tested component
     * and adds it to the document.
     *     Object relations created :
     * <ul>
     *  <li> <code>'Style1', 'Style2'</code> for
     *      {@link ifc.drawing._Shape} :
     *       two values of 'Style' property. The first is taken
     *       from the shape tested, the second from another
     *       shape added to the draw page. </li>
     *  <li> <code>'XTEXTINFO'</code> for
     *      {@link ifc.text._XText} :
     *      creator which can create instances of
     *      <code>com.sun.star.text.TextField.URL</code>
     *      service. </li>
     * </ul>
     */
    @Override
    protected TestEnvironment createTestEnvironment
            (TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;
        XShape oShape = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF());
        oShape = SOF.createShape(xSheetDoc,5000, 3500, 7500, 5000,"Rectangle");
        DrawTools.getShapes(DrawTools.getDrawPage(xSheetDoc,0)).add(oShape);
        oObj = oShape ;

        for (int i=0; i < 10; i++) {
            DrawTools.getShapes(DrawTools.getDrawPage(xSheetDoc,0)).add(
                SOF.createShape(xSheetDoc,
                    5000, 3500, 7510 + 10 * i, 5010 + 10 * i, "Rectangle"));
        }

        // create test environment here
        TestEnvironment tEnv = new TestEnvironment( oShape );

        log.println("Implementation name: "+util.utils.getImplName(oObj));
        tEnv.addObjRelation("DOCUMENT",xSheetDoc);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class ScShapeObj
