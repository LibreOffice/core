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

import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.text.ParagraphEnumeration</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::container::XEnumeration</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurrently.
 * @see com.sun.star.container.XEnumeration
 * @see ifc.container._XEnumeration
 */
public class SwXParagraphEnumeration extends TestCase {
        XTextDocument xTextDoc = null;

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
    * Creating a TestEnvironment for the interfaces to be tested. After major
    * text is gotten from a text document, three paragraphs (each of them
    * filled by 5 strings) are inserted to major text. Finally, paragraph
    * enumeration is created using <code>XEnumeration</code> interface.
    *     Object relations created :
    * <ul>
    *  <li> <code>'ENUM'</code> for
    *      {@link ifc.container._XEnumeration} : major text of text document
    *  with several paragraphs inserted, queried to
    *  <code>XEnumerationAccess</code> interface.</li>
    * </ul>
    */
    @Override
    public TestEnvironment createTestEnvironment(
            TestParameters tParam, PrintWriter log ) throws StatusException {
        XInterface oObj = null;

        log.println( "creating a test environment" );
        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();

        for (int i=0; i<3; i++) {
            try {
                oText.insertString( oCursor, "Paragraph Number: " + i, false);
                oText.insertControlCharacter( oCursor,
                    ControlCharacter.LINE_BREAK, false );
            } catch ( com.sun.star.lang.IllegalArgumentException e ){
                log.println( "EXCEPTION: " + e);
            }

            for (int j=0; j<5; j++){
                try {
                    oText.insertString( oCursor,"The quick brown fox jumps"+
                        " over the lazy Dog: SwXParagraph", false);
                    oText.insertControlCharacter( oCursor,
                        ControlCharacter.LINE_BREAK, false );
                    oText.insertString( oCursor, "THE QUICK BROWN FOX JUMPS"+
                        " OVER THE LAZY DOG: SwXParagraph", false);
                    oText.insertControlCharacter( oCursor,
                        ControlCharacter.LINE_BREAK, false );
                } catch ( com.sun.star.lang.IllegalArgumentException e ){
                    log.println( "EXCEPTION: " + e);
                }
            }

            try {
                oText.insertControlCharacter( oCursor,
                    ControlCharacter.PARAGRAPH_BREAK, false );
            } catch ( com.sun.star.lang.IllegalArgumentException e ){
                log.println( "EXCEPTION: " + e);
            }
        }

        // Enumeration
        XEnumerationAccess oEnumA = UnoRuntime.queryInterface( XEnumerationAccess.class, oText );
        XEnumeration oEnum = oEnumA.createEnumeration();

        oObj = oEnum;

        log.println("creating a new environment for ParagraphEnumeration object");
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("ENUM", oEnumA);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SwXParagraphEnumeration

