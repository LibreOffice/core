/*************************************************************************
 *
 *  $RCSfile: SwXParagraph.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 12:47:27 $
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
import com.sun.star.util.XCloseable;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.text.Paragraph</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::text::XTextContent</code></li>
 *  <li> <code>com::sun::star::text::TextContent</code></li>
 *  <li> <code>com::sun::star::style::CharacterPropertiesComplex</code></li>
 *  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 *  <li> <code>com::sun::star::container::XElementAccess</code></li>
 *  <li> <code>com::sun::star::container::XEnumerationAccess</code></li>
 *  <li> <code>com::sun::star::beans::XPropertyState</code></li>
 *  <li> <code>com::sun::star::style::CharacterProperties</code></li>
 *  <li> <code>com::sun::star::style::ParagraphProperties</code></li>
 *  <li> <code>com::sun::star::lang::XComponent</code></li>
 *  <li> <code>com::sun::star::style::CharacterPropertiesAsian</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 * @see com.sun.star.text.XTextContent
 * @see com.sun.star.text.TextContent
 * @see com.sun.star.style.CharacterPropertiesComplex
 * @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.container.XElementAccess
 * @see com.sun.star.container.XEnumerationAccess
 * @see com.sun.star.beans.XPropertyState
 * @see com.sun.star.style.CharacterProperties
 * @see com.sun.star.style.ParagraphProperties
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.style.CharacterPropertiesAsian
 * @see com.sun.star.text.Paragraph
 * @see ifc.text._XTextContent
 * @see ifc.text._TextContent
 * @see ifc.style._CharacterPropertiesComplex
 * @see ifc.beans._XPropertySet
 * @see ifc.container._XElementAccess
 * @see ifc.container._XEnumerationAccess
 * @see ifc.beans._XPropertyState
 * @see ifc.style._CharacterProperties
 * @see ifc.style._ParagraphProperties
 * @see ifc.lang._XComponent
 * @see ifc.style._CharacterPropertiesAsian
 */
public class SwXParagraph extends TestCase {
    XTextDocument xTextDoc;

    /**
     * Creates text document.
     */
    protected void initialize( TestParameters tParam, PrintWriter log ) {

    }

    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");

        try {
            XCloseable closer = (XCloseable) UnoRuntime.queryInterface(
            XCloseable.class, xTextDoc);
            closer.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            log.println("couldn't close document");
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("couldn't close document");
        }
    }


    /**
     * Creating a Testenvironment for the interfaces to be tested. To obtain
     * test component, at first several paragraphs are inserted to a major text
     * of text document. Then enumeration of text paragraphs is created, and
     * some paragraph is gotten using <code>XEnumeration</code> interface.<p>
     *     Object relations created :
     * <ul>
     *  <li> <code>'PARA'</code> for
     *      {@link ifc.style._CharacterProperties} : paragraph </li>
     *  <li> <code>'PORTION'</code> for
     *      {@link ifc.style._CharacterProperties} : some text portion of
     *  paragraph. To obtain text portion, enumeration of paragraph text portions is
     *  created, and some text portion is gotten using <code>XEnumeration</code>
     *  interface. </li>
     *  <li> <code>'NRULES'</code> for
     *      {@link ifc.style._ParagraphProperties} : value of property
     *  'NumberingRules' of paragraph. Method changes property
     *  'NumberingStyleName' of previously obtained paragraph, and gets value of
     *  a property 'NumberingRules'. </li>
     * </ul>
     */
    protected synchronized TestEnvironment createTestEnvironment
    (TestParameters tParam, PrintWriter log) {

        if (xTextDoc != null) {
            log.println("    disposing xTextDoc ");

            try {
                XCloseable closer = (XCloseable) UnoRuntime.queryInterface(
                XCloseable.class, xTextDoc);
                closer.close(true);
            } catch (com.sun.star.util.CloseVetoException e) {
                log.println("couldn't close document");
            } catch (com.sun.star.lang.DisposedException e) {
                log.println("couldn't close document");
            }
        }
        SOfficeFactory SOF = SOfficeFactory.getFactory(  (XMultiServiceFactory) tParam.getMSF() );
        try {
            log.println( "creating a textdocument" );
            xTextDoc = SOF.createTextDoc( null );
        } catch ( com.sun.star.uno.Exception e ) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn³t create document", e );
        }

        XInterface oObj = null;
        XPropertySet paraP = null;
        XPropertySet portP = null;
        Object nRules = null;
        XInterface port = null;
        XInterface para = null;

        log.println( "creating a test environment" );


        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();

        log.println( "inserting some lines" );
        try {
            for (int i=0; i<5; i++){
                oText.insertString( oCursor,"Paragraph Number: " + i, false);
                oText.insertString( oCursor,
                " The quick brown fox jumps over the lazy Dog: SwXParagraph",
                false);
                oText.insertControlCharacter(
                oCursor, ControlCharacter.PARAGRAPH_BREAK, false );
                oText.insertString( oCursor,
                "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG: SwXParagraph",
                false);
                oText.insertControlCharacter(oCursor,
                ControlCharacter.PARAGRAPH_BREAK, false );
                oText.insertControlCharacter(
                oCursor, ControlCharacter.LINE_BREAK, false );
            }
        } catch ( com.sun.star.lang.IllegalArgumentException e ){
            e.printStackTrace(log);
            throw new StatusException( "Couldn't insert lines", e );
        }

        // Enumeration
        XEnumerationAccess oEnumA = (XEnumerationAccess)
        UnoRuntime.queryInterface(XEnumerationAccess.class, oText );
        XEnumeration oEnum = oEnumA.createEnumeration();

        try {
            para = (XInterface) AnyConverter.toObject(
            new Type(XInterface.class),oEnum.nextElement());
            XEnumerationAccess oEnumB = (XEnumerationAccess)
            UnoRuntime.queryInterface( XEnumerationAccess.class, para );
            XEnumeration oEnum2 = oEnumB.createEnumeration();
            port = (XInterface) AnyConverter.toObject(
            new Type(XInterface.class),oEnum2.nextElement());
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            e.printStackTrace(log);
            log.println("Error: exception occured...");
        } catch ( com.sun.star.container.NoSuchElementException e ) {
            e.printStackTrace(log);
            log.println("Error: exception occured...");
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            e.printStackTrace(log);
            log.println("Error: exception occured...");
        }

        try {
            portP = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class, port);
            paraP = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class, para);
            paraP.setPropertyValue("NumberingStyleName","Numbering 4");
            nRules = paraP.getPropertyValue("NumberingRules");
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            log.println("Error, exception occured...");
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get Paragraph", e );
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            log.println("Error, exception occured...");
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get Paragraph", e );
        } catch ( com.sun.star.beans.UnknownPropertyException e ) {
            log.println("Error, exception occured...");
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get Paragraph", e );
        } catch ( com.sun.star.beans.PropertyVetoException e ) {
            log.println("Error, exception occured...");
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get Paragraph", e );
        }


        try {
            oObj = (XInterface) AnyConverter.toObject(
            new Type(XInterface.class),oEnum.nextElement());
        } catch ( Exception e) {
            log.println("Error, exception occured...");
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get Paragraph", e );
        }


        log.println( "creating a new environment for Paragraph object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        log.println("adding ObjectRelation 'PARA' for CharacterProperties");
        tEnv.addObjRelation("PARA", paraP);

        log.println("adding ObjectRelation 'PORTION' for CharacterProperties");
        tEnv.addObjRelation("PORTION", portP);

        log.println("adding ObjectRelation 'NRULES' for ParagraphProperties");
        tEnv.addObjRelation("NRULES", nRules);

        tEnv.addObjRelation("NoAttach","SwXParagraph");

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SwXParagraph

