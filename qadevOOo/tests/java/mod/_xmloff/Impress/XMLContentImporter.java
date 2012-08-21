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

package mod._xmloff.Impress;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.container.XNamed;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.comp.Impress.XMLContentImporter</code>. <p>
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
public class XMLContentImporter extends TestCase {
    XComponent xImpressDoc = null;

    /**
    * New chart document created.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());

        try {
            log.println( "creating an impress document" );
            xImpressDoc = SOF.createImpressDoc(null);
        } catch ( Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
    * Disposes document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing document " );
        xImpressDoc.dispose();
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.comp.Impress.XMLContentImporter</code><p>
    *
    * The chart document is set as a target document for importer.
    * Imported XML-data contains the tag with a new page which has
    * some name.
    * After import page name getting from target document is checked.
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
    public synchronized TestEnvironment createTestEnvironment
        (TestParameters tParam,PrintWriter log) throws StatusException {

        XInterface oObj = null;
        Object oInt = null ;
        final String impPageName = "XMLImporterPage" ;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF() ;

        try {
            oInt = xMSF.createInstance
                ("com.sun.star.comp.Impress.XMLContentImporter") ;

        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log) ;
            throw new StatusException("Can't create component.", e) ;
        }

        oObj = (XInterface) oInt ;

        // create testobject here
        log.println( "creating a new environment for Paragraph object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding relation
        tEnv.addObjRelation("TargetDocument", xImpressDoc) ;

        // adding relation for XDocumentHandler
        String[][] xml = new String[][] {
            {"start", "office:document",
                "xmlns:office", "CDATA", "http://openoffice.org/2000/office",
                "xmlns:presentation", "CDATA", "http://openoffice.org/2000/presentation",
                "xmlns:svg", "CDATA", "http://openoffice.org/2000/svg",
                "xmlns:draw", "CDATA", "http://openoffice.org/2000/drawing",
                "office:class", "CDATA", "presentation"
                ,"office:version", "CDATA", "1.0"
                },
            {"start", "office:body"},
            {"start", "draw:page",
                "draw:name", "CDATA", impPageName,
                "draw:master-page-name", "CDATA", "Default"
            },
            {"start", "presentation:notes"},
            {"start", "draw:page-thumbnail",
                "draw:style-name", "CDATA", "gr1",
                "draw:layer", "CDATA", "layout",
                "svg:width", "CDATA", "12.768cm",
                "svg:height", "CDATA", "9.576cm",
                "svg:x", "CDATA", "4.411cm",
                "svg:y", "CDATA", "2.794cm",
                "presentation:class", "CDATA", "page",
                "draw:page-number", "CDATA", "1"
            },
            {"end", "draw:page-thumbnail"},
            {"start", "draw:text-box",
                "presentation:style-name", "CDATA", "Default-notes",
                "draw:layer", "CDATA", "layout",
                "svg:width", "CDATA", "15.021cm",
                "svg:height", "CDATA", "10.63cm",
                "svg:x", "CDATA", "3.292cm",
                "svg:y", "CDATA", "13.299cm",
                "presentation:class", "CDATA", "notes",
                "presentation:placeholder", "CDATA", "true"
            },
            {"end", "draw:text-box"},
            {"end", "presentation:notes"},
            {"end", "draw:page"},
            {"end", "office:body"},
            {"end", "office:document"}} ;

        tEnv.addObjRelation("XDocumentHandler.XMLData", xml) ;

        final PrintWriter logF = log ;

        tEnv.addObjRelation("XDocumentHandler.ImportChecker",
            new ifc.xml.sax._XDocumentHandler.ImportChecker() {
                public boolean checkImport() {
                    try {
                        XDrawPagesSupplier xPagesSup = UnoRuntime.queryInterface
                        (XDrawPagesSupplier.class, xImpressDoc) ;
                        XDrawPages xPages = xPagesSup.getDrawPages() ;
                        XNamed xPageName = UnoRuntime.queryInterface
                            (XNamed.class, xPages.getByIndex(0)) ;
                        String gName = xPageName.getName() ;
                        logF.println("Page name returned = '" + gName + "'") ;
                        return impPageName.equals(gName) ;
                    } catch (com.sun.star.uno.Exception e) {
                        logF.println("Exception occurred while checking filter :") ;
                        e.printStackTrace(logF) ;
                        return false ;
                    }
                }
            }) ;

        return tEnv;
    } // finish method getTestEnvironment
}

