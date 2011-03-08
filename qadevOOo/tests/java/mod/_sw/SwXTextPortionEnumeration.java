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

package mod._sw;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 *
 * initial description
 * @see com.sun.star.text.TextPortion
 *
 */
public class SwXTextPortionEnumeration extends TestCase {

    XTextDocument xTextDoc;

    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
                log.println( "creating a textdocument" );
                xTextDoc = SOF.createTextDoc( null );
        } catch ( com.sun.star.uno.Exception e ) {
                // Some exception occures.FAILED
                e.printStackTrace( log );
                throw new StatusException( "Couldn't create document", e );
        }
    }

    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
     *    creating a Testenvironment for the interfaces to be tested
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *  @return    Status class
     *
     *  @see TestParameters
     *    @see PrintWriter
     */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {

        XInterface param = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        // create testobject here

        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();

        log.println( "inserting Strings" );
        log.println( "inserting ControlCharacter" );


        try{
            for (int i =0; i < 5; i++){
                oText.insertString( oCursor,"Paragraph Number: " + i, false);
                oText.insertControlCharacter(
                    oCursor, ControlCharacter.LINE_BREAK, false );
                oText.insertString( oCursor,
                    "The quick brown fox jumps over the lazy Dog: SwXParagraph\n",
                    false);
                oText.insertControlCharacter(
                    oCursor, ControlCharacter.LINE_BREAK, false );
                oText.insertString( oCursor,
                    "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG: SwXParagraph",
                    false);
                oText.insertControlCharacter(
                    oCursor, ControlCharacter.PARAGRAPH_BREAK, false );
            }
        }catch(Exception e){
            log.println("Couldn't insert Text");
            e.printStackTrace();
            throw new StatusException( "Couldn't insert Text", e );
        }

        // Enumeration
        XEnumerationAccess oEnumA = (XEnumerationAccess)
            UnoRuntime.queryInterface( XEnumerationAccess.class, oText );
        XEnumeration oEnum = oEnumA.createEnumeration();

        int n = 0;
        while ( (oEnum.hasMoreElements()) ) {
            try {
                    param = (XInterface) AnyConverter.toObject(
                        new Type(XInterface.class),oEnum.nextElement());
            } catch ( Exception e) {
                log.println("Couldn't get Paragraph");
                e.printStackTrace();
                throw new StatusException( "Couldn't get Paragraph", e );
            }
             n++;
        }

        XEnumerationAccess oEnumP = (XEnumerationAccess)
            UnoRuntime.queryInterface( XEnumerationAccess.class, param );
        XEnumeration oEnum2 = oEnumP.createEnumeration();

        log.println( "creating a new environment for TextPortionEnumeration object" );
        TestEnvironment tEnv = new TestEnvironment( oEnum2 );

        log.println("adding ObjRelation ENUM for XEnumeration");
        tEnv.addObjRelation("ENUM", oEnumP);

        return tEnv;
    } // finish method getTestEnvironment


}    // finish class SwXTextPortionEnumeration

