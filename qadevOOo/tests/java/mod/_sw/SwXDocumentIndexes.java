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
import com.sun.star.text.XDocumentIndexesSupplier;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;


/**
 * Test for the object, which is represented by collection of document indexes.
 * Object implements the following interfaces:
 * <ul>
 *  <li> <code>com::sun::star::container::XNameAccess</code></li>
 *  <li> <code>com::sun::star::container::XIndexAccess</code></li>
 *  <li> <code>com::sun::star::container::XElementAccess</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 * @see com.sun.star.container.XNameAccess
 * @see com.sun.star.container.XIndexAccess
 * @see com.sun.star.container.XElementAccess
 * @see ifc.container._XNameAccess
 * @see ifc.container._XIndexAccess
 * @see ifc.container._XElementAccess
 */
public class SwXDocumentIndexes extends TestCase {
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
    * <code>com.sun.star.text.ContentIndex</code>, then created content index is
    * inserted to the text document, and finally all document indexes are gotten
    * from a text document using <code>XDocumentIndexesSupplier</code> interface.
    */
    public TestEnvironment createTestEnvironment(
            TestParameters tParam, PrintWriter log ) throws StatusException {
        XInterface oObj = null;
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        log.println( "creating a test environment" );
        XTextContent xTC = null;

        try {
            xTC = SOF.createIndex(xTextDoc,"com.sun.star.text.ContentIndex");
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

        XDocumentIndexesSupplier xDocInd = (XDocumentIndexesSupplier)
                UnoRuntime.queryInterface(XDocumentIndexesSupplier.class,xTextDoc);

        oObj = xDocInd.getDocumentIndexes();
        TestEnvironment tEnv = new TestEnvironment(oObj);
        return tEnv;

    } // finish method getTestEnvironment

}    // finish class SwXDocumentIndexes

