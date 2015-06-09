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

package mod._svx;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DrawTools;
import util.SOfficeFactory;

import com.sun.star.drawing.XShape;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 *
 * initial description
 * @see ifc._XComponent
 * @see ifc._TextContent
 * @see ifc._XTextContent
 * @see ifc._XTextField
 *
 */
public class SvxUnoTextField extends TestCase {

    static XComponent xDrawDoc;

    /**
     * in general this method creates a testdocument
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *
     *  @see TestParameters
     *    @see PrintWriter
     *
     */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        log.println( "creating a drawdoc" );
        xDrawDoc = DrawTools.createDrawDoc( tParam.getMSF());
    }

    /**
     * in general this method disposes the testenvironment and document
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *
     *  @see TestParameters
     *    @see PrintWriter
     *
     */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xDrawDoc " );
        util.DesktopTools.closeDoc(xDrawDoc);
    }
    /**
     *    creating a TestEnvironment for the interfaces to be tested
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *  @return    Status class
     *
     *  @see TestParameters
     *    @see PrintWriter
     */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) throws Exception {

        XInterface oObj = null;
        XShape oShape = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF());
          oShape = SOF.createShape(xDrawDoc,5000,3500,7500,5000,"Rectangle");
        DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape);

        XTextCursor the_Cursor = null;

        // create testobject here
        XText the_Text = UnoRuntime.queryInterface(XText.class,oShape);
        XMultiServiceFactory oDocMSF = UnoRuntime.queryInterface( XMultiServiceFactory.class, xDrawDoc );
        the_Cursor = the_Text.createTextCursor();
        oObj = (XInterface)
            oDocMSF.createInstance( "com.sun.star.text.TextField.DateTime" );
        XTextContent the_Field = UnoRuntime.queryInterface(XTextContent.class,oObj);


        the_Text.insertTextContent(the_Cursor,the_Field,false);

        log.println( "creating a new environment for FieldMaster object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );
        tEnv.addObjRelation("RANGE", the_Cursor);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SvxUnoTextField

