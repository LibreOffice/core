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

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
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
 * threads concurrently.
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

    @Override
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");

        try {
            XCloseable closer = UnoRuntime.queryInterface(
            XCloseable.class, xTextDoc);
            closer.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            log.println("couldn't close document");
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("couldn't close document");
        }
    }


    /**
     * Creating a TestEnvironment for the interfaces to be tested. To obtain
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
    @Override
    protected TestEnvironment createTestEnvironment
    (TestParameters tParam, PrintWriter log) throws Exception {

        if (xTextDoc != null) {
            log.println("    disposing xTextDoc ");

            try {
                XCloseable closer = UnoRuntime.queryInterface(
                XCloseable.class, xTextDoc);
                closer.close(true);
            } catch (com.sun.star.util.CloseVetoException e) {
                log.println("couldn't close document");
            } catch (com.sun.star.lang.DisposedException e) {
                log.println("couldn't close document");
            }
        }
        SOfficeFactory SOF = SOfficeFactory.getFactory(  tParam.getMSF() );
        log.println( "creating a textdocument" );
        xTextDoc = SOF.createTextDoc( null );

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

        // Enumeration
        XEnumerationAccess oEnumA = UnoRuntime.queryInterface(XEnumerationAccess.class, oText );
        XEnumeration oEnum = oEnumA.createEnumeration();

        try {
            para = (XInterface) AnyConverter.toObject(
            new Type(XInterface.class),oEnum.nextElement());
            XEnumerationAccess oEnumB = UnoRuntime.queryInterface( XEnumerationAccess.class, para );
            XEnumeration oEnum2 = oEnumB.createEnumeration();
            port = (XInterface) AnyConverter.toObject(
            new Type(XInterface.class),oEnum2.nextElement());
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            e.printStackTrace(log);
            log.println("Error: exception occurred...");
        } catch ( com.sun.star.container.NoSuchElementException e ) {
            e.printStackTrace(log);
            log.println("Error: exception occurred...");
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            e.printStackTrace(log);
            log.println("Error: exception occurred...");
        }

        portP = UnoRuntime.queryInterface(XPropertySet.class, port);
        paraP = UnoRuntime.queryInterface(XPropertySet.class, para);
        paraP.setPropertyValue("NumberingStyleName","Numbering IVX");
        nRules = paraP.getPropertyValue("NumberingRules");


        oObj = (XInterface) AnyConverter.toObject(
        new Type(XInterface.class),oEnum.nextElement());


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

