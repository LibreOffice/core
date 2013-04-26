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

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextEmbeddedObjectsSupplier;
import com.sun.star.text.XTextFrame;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;


/**
 *
 * initial description
 * @see com.sun.star.container.XNamed
 * @see com.sun.star.document.XEmbeddedObjectSupplier
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.text.TextEmbeddedObject
 * @see com.sun.star.text.XTextContent
 *
 */
public class SwXTextEmbeddedObject extends TestCase {
    XTextDocument xTextDoc;

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
     *    creating a Testenvironment for the interfaces to be tested
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *  @return    Status class
     *
     *  @see TestParameters
     *    @see PrintWriter
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam,
                                                    PrintWriter log) {
        XInterface oObj = null;

        // create testobject here
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory) tParam.getMSF());
        try {
            xTextDoc = SOF.createTextDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't open document", e);
        }

        XTextCursor xCursor = xTextDoc.getText().createTextCursor();
        try {
            XMultiServiceFactory xMultiServiceFactory = UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDoc);
            Object o = xMultiServiceFactory.createInstance("com.sun.star.text.TextEmbeddedObject" );
            XTextContent xTextContent = UnoRuntime.queryInterface(XTextContent.class, o);
            String sChartClassID = "12dcae26-281f-416f-a234-c3086127382e";
            XPropertySet xPropertySet = UnoRuntime.queryInterface(XPropertySet.class, xTextContent);
            xPropertySet.setPropertyValue( "CLSID", sChartClassID );

            xTextDoc.getText().insertTextContent( xCursor, xTextContent, false );
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
        }

        XTextEmbeddedObjectsSupplier oTEOS = UnoRuntime.queryInterface(
                                                     XTextEmbeddedObjectsSupplier.class,
                                                     xTextDoc);

        XNameAccess oEmObj = oTEOS.getEmbeddedObjects();
        XIndexAccess oEmIn = UnoRuntime.queryInterface(
                                     XIndexAccess.class, oEmObj);

        try {
            oObj = (XInterface) AnyConverter.toObject(
                           new Type(XInterface.class), oEmIn.getByIndex(0));
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get Object", e);
        }

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("NoAttach", "SwXTextEmbeddedObject");

        XTextFrame aFrame = SOfficeFactory.createTextFrame(xTextDoc, 500, 500);
        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();
        XTextContent the_content = UnoRuntime.queryInterface(
                                           XTextContent.class, aFrame);

        try {
            oText.insertTextContent(oCursor, the_content, true);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Couldn't insert frame " + e.getMessage());
        }

        tEnv.addObjRelation("TextFrame", aFrame);

        tEnv.addObjRelation("NoSetSize", "SwXTextEmbeddedObject");
        tEnv.addObjRelation("NoPos", "SwXTextEmbeddedObject");

        return tEnv;
    } // finish method getTestEnvironment
} // finish class SwXTextEmbeddedObject
