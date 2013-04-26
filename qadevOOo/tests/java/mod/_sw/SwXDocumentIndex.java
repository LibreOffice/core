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

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextRange;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;


/**
 * Test for object which is represented by service
 * <code>com.sun.star.text.DocumentIndex</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::lang::XComponent</code></li>
 *  <li> <code>com::sun::star::text::XDocumentIndex</code></li>
 *  <li> <code>com::sun::star::text::BaseIndex</code></li>
 *  <li> <code>com::sun::star::text::XTextContent</code></li>
 *  <li> <code>com::sun::star::text::DocumentIndex</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.text.XDocumentIndex
 * @see com.sun.star.text.BaseIndex
 * @see com.sun.star.text.XTextContent
 * @see com.sun.star.text.DocumentIndex
 * @see ifc.lang._XComponent
 * @see ifc.text._XDocumentIndex
 * @see ifc.text._BaseIndex
 * @see ifc.text._XTextContent
 * @see ifc.text._DocumentIndex
 */
public class SwXDocumentIndex extends TestCase {
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
    * Creates an instance of the service
    * <code>com.sun.star.text.DocumentIndex</code>, then created document index
    * is inserted to the text of the document as content.
    *
    */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {
        XTextContent xTC = null;
        Object instance = null;

        SOfficeFactory SOF = SOfficeFactory.getFactory((XMultiServiceFactory)tParam.getMSF());
        log.println( "creating a test environment" );
        try {
            xTC = SOfficeFactory.createIndex(xTextDoc, "com.sun.star.text.DocumentIndex");
            instance = SOfficeFactory.createIndex(xTextDoc, "com.sun.star.text.DocumentIndex");
        }
        catch ( com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create the Index", e);
        }

        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();

        log.println("inserting the Index into text document");
        try {
            oText.insertTextContent(oCursor, xTC, false);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't insert the Index", e);
        }

        TestEnvironment tEnv = new TestEnvironment(xTC);

        tEnv.addObjRelation("CONTENT", UnoRuntime.queryInterface(XTextContent.class,instance));
        oCursor.gotoEnd(false);
        tEnv.addObjRelation("RANGE", UnoRuntime.queryInterface(XTextRange.class, oCursor));

        // relation for XDocumentIndex
        tEnv.addObjRelation("TextDoc", xTextDoc);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SwXDocumentIndex

