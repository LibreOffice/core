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
import util.InstCreator;
import util.ParagraphDsc;
import util.SOfficeFactory;
import util.TableDsc;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.style.XStyle;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.text.XText;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;


/**
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
 * threads concurrently.
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
public class SwXHeadFootText extends TestCase {
    XTextDocument xTextDoc;

    /**
    * Creates text document.
    */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory(  tParam.getMSF() );
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
    * Creating a TestEnvironment for the interfaces to be tested. At first
    * style families are obtained from text document, then style 'Standard' has
    * gotten from style family 'PageStyles'. At the end, document header and
    * footer are switched on and document text is obtained.
    *     Object relations created :
    * <ul>
    *  <li> <code>'XTEXTINFO'</code> for
    *    {@link ifc.text._XRelativeTextContentInsert},
    *    {@link ifc.text._XText} : creates tables 6x4</li>
    *  <li> <code>'TEXTDOC'</code> for
    *    {@link ifc.text._XTextRangeCompare} : text document</li>
    *  <li> <code>'PARA'</code> for
    *    {@link ifc.text._XRelativeTextContentInsert} : paragraph creator</li>
    * </ul>
    */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) throws Exception {
        XInterface oObj = null;
        XPropertySet PropSet;
        XNameAccess PageStyles = null;
        XStyle StdStyle = null;

        log.println( "creating a test environment" );
        XStyleFamiliesSupplier StyleFam = UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, xTextDoc);
        XNameAccess StyleFamNames = StyleFam.getStyleFamilies();

        // obtains style 'Standatd' from style family 'PageStyles'
        PageStyles = (XNameAccess) AnyConverter.toObject(
            new Type(XNameAccess.class),StyleFamNames.getByName("PageStyles"));
        StdStyle = (XStyle) AnyConverter.toObject(
                new Type(XStyle.class),PageStyles.getByName("Standard"));

        PropSet = UnoRuntime.queryInterface( XPropertySet.class, StdStyle);

        // changing/getting some properties
        log.println( "Switching on header" );
        PropSet.setPropertyValue("HeaderIsOn", Boolean.TRUE);
        log.println( "Switching on footer" );
        PropSet.setPropertyValue("FooterIsOn", Boolean.TRUE);
        log.println( "Get header text" );
        oObj = UnoRuntime.queryInterface(
                    XText.class, PropSet.getPropertyValue("HeaderText"));

        log.println( "creating a new environment for bodytext object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        log.println( "    adding Paragraph" );
        ParagraphDsc pDsc = new ParagraphDsc();
        tEnv.addObjRelation( "PARA", new InstCreator( xTextDoc, pDsc ) );

        log.println( "adding InstDescriptor object" );
        TableDsc tDsc = new TableDsc( 6, 4 );

        log.println( "adding InstCreator object" );
        tEnv.addObjRelation( "XTEXTINFO", new InstCreator( xTextDoc, tDsc ) );

        tEnv.addObjRelation( "TEXT", oObj);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SwXHeadFootText
