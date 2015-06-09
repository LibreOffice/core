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

import com.sun.star.document.XImporter;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.comp.Writer.XMLImporter</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li><code>com::sun::star::lang::XInitialization</code></li>
 *  <li><code>com::sun::star::document::XImporter</code></li>
 *  <li><code>com::sun::star::document::XFilter</code></li>
 *  <li><code>com::sun::star::document::ImportFilter</code></li>
 *  <li><code>com::sun::star::beans::XPropertySet</code></li>
 *  <li><code>com::sun::star::xml::sax::XDocumentHandler</code></li>

 * </ul>
 * @see com.sun.star.lang.XInitialization
 * @see com.sun.star.document.XImporter
 * @see com.sun.star.document.XFilter
 * @see com.sun.star.document.ImportFilter
 * @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.xml.sax.XDocumentHandler
 * @see ifc.lang._XInitialization
 * @see ifc.document._XImporter
 * @see ifc.document._XFilter
 * @see ifc.document._XExporter
 * @see ifc.beans._XPropertySet
 * @see ifc.xml.sax._XDocumentHandler
 */
public class XMLImporter extends TestCase {
    XTextDocument xTextDoc;

    /**
    * New text document created.
    */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );

        log.println( "creating a textdocument" );
        xTextDoc = SOF.createTextDoc( null );
    }

    /**
    * Text document destroyed.
    */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.comp.Writer.XMLImporter</code><p>
    *
    * The text document is set as a target document for importer.
    * Imported tags contain tag including tag with test text.
    * After import text getting from target document is checked.
    *     Object relations created :
    * <ul>
    *  <li> <code>'XDocumentHandler.XMLData'</code> for
    *      {@link ifc.xml.sax._XDocumentHandler} interface </li>
    *  <li> <code>'XDocumentHandler.ImportChecker'</code> for
    *      {@link ifc.xml.sax._XDocumentHandler} interface </li>
    *  <li> <code>'TargetDocument'</code> for
    *      {@link ifc.document._XImporter} interface </li>
    * </ul>
    */
    @Override
    public TestEnvironment createTestEnvironment
            ( TestParameters tParam, PrintWriter log) throws Exception {

        XMultiServiceFactory xMSF = tParam.getMSF() ;
        XInterface oObj = null;

        oObj = (XInterface) xMSF.createInstance
            ("com.sun.star.comp.Writer.XMLImporter");
        XImporter xIm = UnoRuntime.queryInterface
            (XImporter.class,oObj);
        xIm.setTargetDocument(xTextDoc);

        // create testobject here
        log.println( "creating a new environment" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        final String impText = "XMLImporter test." ;
        String[][] xml = new String[][] {
            {"start", "office:document" ,
                "xmlns:office", "CDATA", "http://openoffice.org/2000/office",
                "office:class", "CDATA", "text",
                "xmlns:text", "CDATA", "http://openoffice.org/2000/text"},
            {"start", "office:body" },
            {"start", "text:p" },
            {"chars", impText},
            {"end", "text:p"},
            {"end", "office:body"},
            {"end", "office:document"}} ;

        tEnv.addObjRelation("XDocumentHandler.XMLData", xml) ;

        tEnv.addObjRelation("TargetDocument",xTextDoc);
        log.println("Implementation Name: "+util.utils.getImplName(oObj));

        final XTextDocument textDoc = UnoRuntime.queryInterface
            (XTextDocument.class, xTextDoc) ;
        final PrintWriter fLog = log ;
        tEnv.addObjRelation("XDocumentHandler.ImportChecker",
            new ifc.xml.sax._XDocumentHandler.ImportChecker() {
                public boolean checkImport() {
                    String docText = textDoc.getText().getString() ;
                    fLog.println("Document text returned = '" + docText + "'") ;
                    return impText.equals(docText) ;
                }
            }) ;

        return tEnv;

    }

}

