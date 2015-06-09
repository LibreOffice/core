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

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XDocumentIndexMark;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;


/**
 * Test for object which is represented by service
 * <code>com.sun.star.text.DocumentIndexMark</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::lang::XComponent</code></li>
 *  <li> <code>com::sun::star::text::BaseIndexMark</code></li>
 *  <li> <code>com::sun::star::text::XTextContent</code></li>
 *  <li> <code>com::sun::star::text::TextContent</code></li>
 *  <li> <code>com::sun::star::text::DocumentIndexMark</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurrently.
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.text.BaseIndexMark
 * @see com.sun.star.text.XTextContent
 * @see com.sun.star.text.TextContent
 * @see com.sun.star.text.DocumentIndexMark
 * @see ifc.lang._XComponent
 * @see ifc.text._BaseIndexMark
 * @see ifc.text._XTextContent
 * @see ifc.text._TextContent
 * @see ifc.text._DocumentIndexMark
 */
public class SwXDocumentIndexMark extends TestCase {
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
    * <code>com.sun.star.text.DocumentIndexMark</code>, attaches text to
    * created DocumentIndexMark, and inserts DocumentIndexMark to the
    * text document.
    */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {

        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();

        XMultiServiceFactory oDocMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDoc);

        Object oDIM = null;
        Object instance = null;
        try {
            oDIM = oDocMSF.createInstance
                ("com.sun.star.text.DocumentIndexMark");
            instance = oDocMSF.createInstance
                ("com.sun.star.text.DocumentIndexMark");
        } catch ( com.sun.star.uno.Exception e ) {
            log.println("Error:" + e);
        }

        XDocumentIndexMark xDIM = UnoRuntime.queryInterface(XDocumentIndexMark.class, oDIM);

        oText.insertTextContent(oCursor, xDIM, false);

        TestEnvironment tEnv = new TestEnvironment(xDIM);

        tEnv.addObjRelation("CONTENT", UnoRuntime.queryInterface(XTextContent.class,instance));
        tEnv.addObjRelation("RANGE", xTextDoc.getText().createTextCursor());

        return tEnv;

    } // finish method getTestEnvironment

 }    // finish class SwXDocumentIndexMark

