/*************************************************************************
 *
 *  $RCSfile: SwXParagraphEnumeration.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 12:47:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
 * threads concurently.
 * @see com.sun.star.container.XEnumeration
 * @see ifc.container._XEnumeration
 */
public class SwXParagraphEnumeration extends TestCase {
        XTextDocument xTextDoc = null;

    /**
    * Creates text document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );
        try {
            log.println( "creating a textdocument" );
            xTextDoc = SOF.createTextDoc( null );
        } catch ( com.sun.star.uno.Exception e ) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn³t create document", e );
        }
    }

    /**
    * Disposes text document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        xTextDoc.dispose();
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested. After major
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
    public synchronized TestEnvironment createTestEnvironment(
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
        XEnumerationAccess oEnumA = (XEnumerationAccess)
            UnoRuntime.queryInterface( XEnumerationAccess.class, oText );
        XEnumeration oEnum = oEnumA.createEnumeration();

        oObj = oEnum;

        log.println("creating a new environment for ParagraphEnumeration object");
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("ENUM", oEnumA);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SwXParagraphEnumeration

