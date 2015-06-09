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

import com.sun.star.beans.XPropertySet;
import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextEmbeddedObjectsSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;


public class SwXTextEmbeddedObjects extends TestCase {

    XTextDocument oDoc;

    /**
     * in general this method creates a testdocument
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *
     *  @see TestParameters
     *    @see PrintWriter
     *
     */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );
        oDoc = SOF.createTextDoc(null);
    }

    /**
     * in general this method disposes the testenvironment and document
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *
     *  @see TestParameters
     *    @see PrintWriter
     *
     */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        util.DesktopTools.closeDoc(oDoc);
    }


    /**
     *    creating a TestEnvironment for the interfaces to be tested
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *  @return    Status class
     *
     *  @see TestParameters
     *    @see PrintWriter
     */
    @Override
    public TestEnvironment createTestEnvironment( TestParameters tParam,
                  PrintWriter log ) throws StatusException {

        XInterface oObj = null;

        // create testobject here
        XTextCursor xCursor = oDoc.getText().createTextCursor();
        try {
            XMultiServiceFactory xMultiServiceFactory = UnoRuntime.queryInterface(XMultiServiceFactory.class, oDoc);
            Object o = xMultiServiceFactory.createInstance("com.sun.star.text.TextEmbeddedObject" );
            XTextContent xTextContent = UnoRuntime.queryInterface(XTextContent.class, o);
            String sChartClassID = "12dcae26-281f-416f-a234-c3086127382e";
            XPropertySet xPropertySet = UnoRuntime.queryInterface(XPropertySet.class, xTextContent);
            xPropertySet.setPropertyValue( "CLSID", sChartClassID );

            oDoc.getText().insertTextContent( xCursor, xTextContent, false );
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
        }

        XTextEmbeddedObjectsSupplier oTEOS = UnoRuntime.queryInterface(XTextEmbeddedObjectsSupplier.class, oDoc);

        oObj = oTEOS.getEmbeddedObjects();

          TestEnvironment tEnv = new TestEnvironment( oObj );
        return tEnv;

        } // finish method getTestEnvironment

}    // finish class SwXTextEmbeddedObjects

