/*************************************************************************
 *
 *  $RCSfile: SwXTextColumns.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 12:50:03 $
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

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.style.XStyle;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextColumns;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.text.TextColumns</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::text::XTextColumns</code></li>
 *  <li> <code>com::sun::star::text::TextColumns</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 * @see com.sun.star.text.XTextColumns
 * @see com.sun.star.text.TextColumns
 * @see ifc.text._XTextColumns
 * @see ifc.text._TextColumns
 */
public class SwXTextColumns extends TestCase {
    XTextDocument xTextDoc;

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
    * Creating a Testenvironment for the interfaces to be tested. After style
    * families are gotten from text document using
    * <code>XStyleFamiliesSupplier</code> interface, style family indexed '2'
    * is obtained using <code>XIndexAccess</code> interface. Then style
    * named 'Standard' is gotten from previously obtained style family using
    * <code>XNameAccess</code> interface, and 'TextColumns' property value
    * of this style is returned as a test component. Finally, several
    * paragraphs within the text are inserted to a text document.
    */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {
        XInterface oObj = null;
        TestEnvironment tEnv = null;
        XStyle oStyle = null;

        log.println( "creating a test environment" );
        log.println("getting PageStyle");
        XStyleFamiliesSupplier oSFS = (XStyleFamiliesSupplier)
            UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, xTextDoc);
        XNameAccess oSF = oSFS.getStyleFamilies();
        XIndexAccess oSFIA = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class, oSF);

        try {
            XNameAccess oSFNA = (XNameAccess) AnyConverter.toObject(
                        new Type(XNameAccess.class),oSFIA.getByIndex(2));
            oStyle = (XStyle) AnyConverter.toObject(
                    new Type(XStyle.class),oSFNA.getByName("Standard"));
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            log.println("Error, exception occured while getting style.");
            e.printStackTrace(log);
        } catch ( com.sun.star.lang.IndexOutOfBoundsException e ) {
            log.println("Error, exception occured while getting style.");
            e.printStackTrace(log);
        } catch ( com.sun.star.container.NoSuchElementException e ) {
            log.println("Error, exception occured while getting style.");
            e.printStackTrace(log);
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            log.println("Error, exception occured while getting style.");
            e.printStackTrace(log);
        }

        try {
            log.println("Getting property ('TextColumns') value of style "
                + oStyle.getName());
            XPropertySet xProps = (XPropertySet)
                UnoRuntime.queryInterface(XPropertySet.class,oStyle);
            oObj = (XTextColumns) AnyConverter.toObject(
                new Type(XTextColumns.class),xProps.getPropertyValue("TextColumns"));
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            log.println("Exception occured while getting style property");
            e.printStackTrace(log);
        } catch ( com.sun.star.beans.UnknownPropertyException e ) {
            log.println("Exception occured while getting style property");
            e.printStackTrace(log);
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            log.println("Exception occured while getting style property");
            e.printStackTrace(log);
        }

        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();

        log.println( "inserting some text to text document..." );
        try {
            for (int i =0; i < 5; i++){
                oText.insertString( oCursor,"Paragraph Number: " + i, false);
                oText.insertString( oCursor,
                    "The quick brown fox jumps over the lazy Dog: SwXParagraph",
                    false);
                oText.insertControlCharacter( oCursor,
                    ControlCharacter.PARAGRAPH_BREAK, false );
                oText.insertString( oCursor,
                    "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG: SwXParagraph",
                    false);
                oText.insertControlCharacter( oCursor,
                    ControlCharacter.PARAGRAPH_BREAK, false );
                oText.insertControlCharacter( oCursor,
                    ControlCharacter.LINE_BREAK, false );
            }
        } catch ( com.sun.star.lang.IllegalArgumentException e ){
            log.println("Exception occured while inserting Text");
            e.printStackTrace(log);
        }

        log.println("creating a new environment for object");
        tEnv = new TestEnvironment(oObj);
        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SwXTextColumns
