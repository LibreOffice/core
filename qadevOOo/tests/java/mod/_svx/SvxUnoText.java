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

import com.sun.star.text.XText;
import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DefaultDsc;
import util.DrawTools;
import util.InstCreator;
import util.SOfficeFactory;

import com.sun.star.drawing.XShape;
import com.sun.star.lang.XComponent;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XSimpleText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextRange;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class SvxUnoText extends TestCase {

    static XComponent xDrawDoc;

    /**
     * in general this method creates a testdocument
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *
     *  @see TestParameters
     *  *    @see PrintWriter
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
     *  *    @see PrintWriter
     *
     */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xDrawDoc " );
        util.DesktopTools.closeDoc(xDrawDoc);
    }


    /**
     *  *    creating a TestEnvironment for the interfaces to be tested
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *  @return    Status class
     * Object relations created :
     * <ul>
     *  <li> <code>'RangeForMove'</code> for
     *      {@link ifc.text._XTextRangeMover} (the range to be moved)</li>
     *  <li> <code>'XTextRange'</code> for
     *      {@link ifc.text._XTextRangeMover} (the range that includes moving
     *       range)</li>
     * </ul>
     *  @see TestParameters
     *  *    @see PrintWriter
     */
    @Override
    protected TestEnvironment createTestEnvironment
        (TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;
        // create testobject here
        XTextRange aRange = null;
        XShape oShape = null;

        try {
            SOfficeFactory SOF = SOfficeFactory.getFactory(tParam.getMSF()) ;
            oShape = SOF.createShape
                (xDrawDoc,5000,3500,7500,5000,"Text");
            DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape);

            XSimpleText text = UnoRuntime.queryInterface
                (XSimpleText.class, oShape) ;

            XTextCursor cursor = text.createTextCursor() ;
            text.insertString(cursor, "Paragraph 1", false) ;
            text.insertControlCharacter(cursor,
                ControlCharacter.PARAGRAPH_BREAK, false) ;
            cursor.setString("TextForMove");
            aRange = cursor;
            XTextCursor cursor1 = text.createTextCursorByRange(text.getEnd());
            text.insertControlCharacter(cursor1,
                ControlCharacter.PARAGRAPH_BREAK, false) ;
            text.insertString(cursor1, "Paragraph 2", false);
            text.insertControlCharacter(cursor1,
                ControlCharacter.PARAGRAPH_BREAK, false) ;
            text.insertString(cursor1, "Paragraph 3", false) ;
            text.insertControlCharacter(cursor1,
                ControlCharacter.PARAGRAPH_BREAK, false) ;
            oObj = text.getText() ;
        } catch (Exception e) {
            log.println("Can't create test object") ;
            e.printStackTrace(log) ;
        }

        // create test environment here
        TestEnvironment tEnv = new TestEnvironment( oObj );
        // adding relation for XText
        DefaultDsc tDsc = new DefaultDsc("com.sun.star.text.XTextContent",
                                    "com.sun.star.text.TextField.DateTime");
        log.println( "    adding InstCreator object" );
        tEnv.addObjRelation( "XTEXTINFO", new InstCreator( xDrawDoc, tDsc ) );

        // adding relation for XTextRangeMover
        tEnv.addObjRelation("RangeForMove", aRange);
        tEnv.addObjRelation("XTextRange", oObj);

        // adding relation for XTextRangeCompare
        tEnv.addObjRelation("TEXT", UnoRuntime.queryInterface(XText.class, oShape)) ;

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SvxUnoText

