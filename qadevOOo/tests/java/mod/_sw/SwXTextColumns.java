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
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
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
 * threads concurrently.
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
    * Creating a TestEnvironment for the interfaces to be tested. After style
    * families are gotten from text document using
    * <code>XStyleFamiliesSupplier</code> interface, style family indexed '2'
    * is obtained using <code>XIndexAccess</code> interface. Then style
    * named 'Standard' is gotten from previously obtained style family using
    * <code>XNameAccess</code> interface, and 'TextColumns' property value
    * of this style is returned as a test component. Finally, several
    * paragraphs within the text are inserted to a text document.
    */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {
        XInterface oObj = null;
        TestEnvironment tEnv = null;
        XStyle oStyle = null;

        log.println( "creating a test environment" );
        log.println("getting PageStyle");
        XStyleFamiliesSupplier oSFS = UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, xTextDoc);
        XNameAccess oSF = oSFS.getStyleFamilies();
        XIndexAccess oSFIA = UnoRuntime.queryInterface(XIndexAccess.class, oSF);

        try {
            XNameAccess oSFNA = (XNameAccess) AnyConverter.toObject(
                        new Type(XNameAccess.class),oSFIA.getByIndex(2));
            oStyle = (XStyle) AnyConverter.toObject(
                    new Type(XStyle.class),oSFNA.getByName("Standard"));
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            log.println("Error, exception occurred while getting style.");
            e.printStackTrace(log);
        } catch ( com.sun.star.lang.IndexOutOfBoundsException e ) {
            log.println("Error, exception occurred while getting style.");
            e.printStackTrace(log);
        } catch ( com.sun.star.container.NoSuchElementException e ) {
            log.println("Error, exception occurred while getting style.");
            e.printStackTrace(log);
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            log.println("Error, exception occurred while getting style.");
            e.printStackTrace(log);
        }

        try {
            log.println("Getting property ('TextColumns') value of style "
                + oStyle.getName());
            XPropertySet xProps = UnoRuntime.queryInterface(XPropertySet.class,oStyle);
            oObj = (XTextColumns) AnyConverter.toObject(
                new Type(XTextColumns.class),xProps.getPropertyValue("TextColumns"));
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            log.println("Exception occurred while getting style property");
            e.printStackTrace(log);
        } catch ( com.sun.star.beans.UnknownPropertyException e ) {
            log.println("Exception occurred while getting style property");
            e.printStackTrace(log);
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            log.println("Exception occurred while getting style property");
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
            log.println("Exception occurred while inserting Text");
            e.printStackTrace(log);
        }

        log.println("creating a new environment for object");
        tEnv = new TestEnvironment(oObj);
        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SwXTextColumns
