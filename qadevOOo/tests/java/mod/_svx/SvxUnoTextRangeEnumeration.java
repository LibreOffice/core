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

import com.sun.star.container.XEnumerationAccess;
import com.sun.star.drawing.XShape;
import com.sun.star.lang.XComponent;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XSimpleText;
import com.sun.star.text.XTextCursor;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class SvxUnoTextRangeEnumeration extends TestCase {

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
        xDrawDoc = DrawTools.createDrawDoc(tParam.getMSF());
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
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;
        // create testobject here

        XEnumerationAccess xEA = null ;
        try {
            SOfficeFactory SOF = SOfficeFactory.getFactory(tParam.getMSF()) ;
            XShape oShape = SOF.createShape
                (xDrawDoc,5000,3500,7500,5000,"Text");
            DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape) ;

            XSimpleText text = UnoRuntime.queryInterface
                (XSimpleText.class, oShape) ;

            XTextCursor cursor = text.createTextCursor() ;

            text.insertString(cursor, "Paragraph 1", false) ;
            text.insertControlCharacter(cursor,
                ControlCharacter.PARAGRAPH_BREAK, false) ;
            text.insertString(cursor, "Paragraph 2", false) ;
            text.insertControlCharacter(cursor,
                ControlCharacter.PARAGRAPH_BREAK, false) ;
            text.insertString(cursor, "Paragraph 3", false) ;
            text.insertControlCharacter(cursor,
                ControlCharacter.PARAGRAPH_BREAK, false) ;

            xEA = UnoRuntime.queryInterface
                (XEnumerationAccess.class, text) ;

            XInterface oTextContent = (XInterface) AnyConverter.toObject(
                new Type(XInterface.class),xEA.createEnumeration().nextElement());

            xEA = UnoRuntime.queryInterface
                (XEnumerationAccess.class, oTextContent) ;

            oObj = xEA.createEnumeration();

        } catch (Exception e) {
            log.println("Can't create test object") ;
            e.printStackTrace(log) ;
        }

        // create test environment here
         TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("ENUM", xEA);

        return tEnv;
    } // finish method getTestEnvironment

}

