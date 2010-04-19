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
            xTC = SOF.createIndex(xTextDoc, "com.sun.star.text.DocumentIndex");
            instance = SOF.createIndex(xTextDoc, "com.sun.star.text.DocumentIndex");
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
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't insert the Index", e);
        }

        TestEnvironment tEnv = new TestEnvironment(xTC);

        tEnv.addObjRelation("CONTENT", (XTextContent)
                        UnoRuntime.queryInterface(XTextContent.class,instance));
        oCursor.gotoEnd(false);
        tEnv.addObjRelation("RANGE", (XTextRange)
                        UnoRuntime.queryInterface(XTextRange.class, oCursor));

        // relation for XDocumentIndex
        tEnv.addObjRelation("TextDoc", xTextDoc);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SwXDocumentIndex

