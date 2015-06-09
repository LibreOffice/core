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
import util.FrameDsc;
import util.InstCreator;
import util.SOfficeFactory;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XFootnote;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.text.Footnote</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::text::XFootnote</code></li>
 *  <li> <code>com::sun::star::lang::XComponent</code></li>
 *  <li> <code>com::sun::star::text::XSimpleText</code></li>
 *  <li> <code>com::sun::star::text::Footnote</code></li>
 *  <li> <code>com::sun::star::text::XTextContent</code></li>
 *  <li> <code>com::sun::star::text::XTextRange</code></li>
 *  <li> <code>com::sun::star::text::XText</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurrently.
 * @see com.sun.star.text.XFootnote
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.text.XSimpleText
 * @see com.sun.star.text.Footnote
 * @see com.sun.star.text.XTextContent
 * @see com.sun.star.text.XTextRange
 * @see com.sun.star.text.XText
 * @see ifc.text._XFootnote
 * @see ifc.lang._XComponent
 * @see ifc.text._XSimpleText
 * @see ifc.text._Footnote
 * @see ifc.text._XTextContent
 * @see ifc.text._XTextRange
 * @see ifc.text._XText
 */
public class SwXFootnote extends TestCase {
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
    * Creating a TestEnvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.text.Footnote</code>. Then inserts created footnote
    * to a text of document as content.
    *     Object relations created :
    * <ul>
    *  <li><code>'XTEXTINFO'</code> for
    *    {@link ifc.text._XText} </li>
    * </ul>
    */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) throws Exception {
        XFootnote oFootnote;

        log.println( "Creating a test environment" );
        // get a soffice factory object
        XMultiServiceFactory msf = UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDoc);
        log.println("creating a footnote");
        Object instance = null;
        oFootnote = UnoRuntime.queryInterface(XFootnote.class,
                msf.createInstance("com.sun.star.text.Footnote"));
        instance = msf.createInstance("com.sun.star.text.Footnote");

        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();

        log.println("inserting the footnote into text document");
        oText.insertTextContent(oCursor, oFootnote, false);

        TestEnvironment tEnv = new TestEnvironment(oFootnote);

        tEnv.addObjRelation("CONTENT", UnoRuntime.queryInterface(XTextContent.class,instance));
        tEnv.addObjRelation("RANGE", xTextDoc.getText().createTextCursor());

        log.println( "adding InstDescriptor object" );
        FrameDsc tDsc = new FrameDsc( 3000, 6000 );
        log.println( "adding InstCreator object" );
        tEnv.addObjRelation( "XTEXTINFO", new InstCreator( xTextDoc, tDsc ) );

        return tEnv;
    }

}    // finish class SwXFootnote

