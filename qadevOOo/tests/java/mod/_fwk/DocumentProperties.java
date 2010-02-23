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

package mod._fwk;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.document.XDocumentInfoSupplier;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.document.DocumentInfo</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li><code>com::sun::star::beans::XFastPropertySet</code></li>
*  <li><code>com::sun::star::beans::XPropertySet</code></li>
*  <li><code>com::sun::star::document::XDocumentInfo</code></li>
*  <li><code>com::sun::star::document::XStandaloneDocumentInfo</code></li>
*  <li><code>com::sun::star::lang::XComponent</code></li>
* </ul><p>
* @see com.sun.star.beans.XFastPropertySet
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.document.XDocumentInfo
* @see com.sun.star.document.XStandaloneDocumentInfo
* @see com.sun.star.lang.XComponent
* @see ifc.beans._XFastPropertySet
* @see ifc.beans._XPropertySet
* @see ifc.document._XDocumentInfo
* @see ifc.document._XStandaloneDocumentInfo
* @see ifc.lang._XComponent
*/
public class DocumentProperties extends TestCase {

    XTextDocument xTextDoc;

    /**
     * Disposes the document, if exists, created in
     * <code>createTestEnvironment</code> method.
     */
    protected void cleanup( TestParameters Param, PrintWriter log) {

        log.println("disposing xTextDoc");

        if (xTextDoc != null) {
            xTextDoc.dispose();
        }
    }

    /**
    * Creates a text document.
    * Obtains the property <code>'DocumentInfo'</code> of the created document.
    */
    public TestEnvironment createTestEnvironment( TestParameters Param,
        PrintWriter log ) throws StatusException {

        XInterface oObj = null;

        log.println( "creating a test environment" );

        if (xTextDoc != null) xTextDoc.dispose();

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());

        try {
            log.println( "creating a text document" );
            xTextDoc = SOF.createTextDoc(null);
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }

        shortWait();

        XDocumentInfoSupplier xdis = (XDocumentInfoSupplier)
                UnoRuntime.queryInterface(XDocumentInfoSupplier.class, xTextDoc);
        //oObj = (XInterface)UnoRuntime.queryInterface(XInterface.class, docInfo);
        oObj = xdis.getDocumentInfo();
        TestEnvironment tEnv = new TestEnvironment( oObj );

        return tEnv;
    } // finish method getTestEnvironment

    /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(500) ;
        } catch (InterruptedException e) {
            log.println("While waiting :" + e) ;
        }
    }
}
