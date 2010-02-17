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
import util.InstCreator;
import util.ParagraphDsc;
import util.SOfficeFactory;
import util.TableDsc;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.text.Text</code>.<p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::text::XTextRangeMover</code></li>
 *  <li> <code>com::sun::star::text::XSimpleText</code></li>
 *  <li> <code>com::sun::star::text::XTextRange</code></li>
 *  <li> <code>com::sun::star::text::XRelativeTextContentInsert</code></li>
 *  <li> <code>com::sun::star::text::XTextRangeCompare</code></li>
 *  <li> <code>com::sun::star::container::XElementAccess</code></li>
 *  <li> <code>com::sun::star::container::XEnumerationAccess</code></li>
 *  <li> <code>com::sun::star::text::XText</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 * @see com.sun.star.text.XTextRangeMover
 * @see com.sun.star.text.XSimpleText
 * @see com.sun.star.text.XTextRange
 * @see com.sun.star.text.XRelativeTextContentInsert
 * @see com.sun.star.text.XTextRangeCompare
 * @see com.sun.star.container.XElementAccess
 * @see com.sun.star.container.XEnumerationAccess
 * @see com.sun.star.text.XText
 * @see ifc.text._XTextRangeMover
 * @see ifc.text._XSimpleText
 * @see ifc.text._XTextRange
 * @see ifc.text._XRelativeTextContentInsert
 * @see ifc.text._XTextRangeCompare
 * @see ifc.container._XElementAccess
 * @see ifc.container._XEnumerationAccess
 * @see ifc.text._XText
 */
public class SwXBodyText extends TestCase {
    XTextDocument xTextDoc = null;;
    SOfficeFactory SOF = null;

    /**
    * Creates text document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );
        try {
            log.println( "creating a textdocument" );
            xTextDoc = SOF.createTextDoc( null );
        } catch ( com.sun.star.uno.Exception e ) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
    * Disposes text document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Obtains body text from text document.
    * Object relations created :
    * <ul>
    *  <li> <code>'TEXT'</code> for
    *    {@link ifc.text._XTextRangeCompare} : text</li>
    *  <li> <code>'XTEXTINFO'</code> for
    *    {@link ifc.text._XRelativeTextContentInsert},
    *    {@link ifc.text._XText} : creates 6x4 tables</li>
    *  <li> <code>'PARA'</code> for
    *    {@link ifc.text._XRelativeTextContentInsert} : paragraph creator</li>
    * </ul>
    */
    public synchronized TestEnvironment createTestEnvironment(
            TestParameters Param, PrintWriter log ) throws StatusException {
        XInterface oObj = null;

        log.println( "creating a test environment" );

        // get the bodytext of textdocument here
        log.println( "    getting the bodytext of textdocument with getText()" );
        oObj = xTextDoc.getText();

        log.println( "    creating a new environment for bodytext object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        log.println(" adding Text as ObjRelation");
        tEnv.addObjRelation("TEXT", oObj );

        log.println( "    adding InstDescriptor object" );
        TableDsc tDsc = new TableDsc( 6, 4 );

        log.println( "    adding InstCreator object" );
        tEnv.addObjRelation( "XTEXTINFO", new InstCreator( xTextDoc, tDsc ) );

        log.println( "    adding Paragraph" );
        ParagraphDsc pDsc = new ParagraphDsc();
        tEnv.addObjRelation( "PARA", new InstCreator( xTextDoc, pDsc ) );

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SwXBodyText

