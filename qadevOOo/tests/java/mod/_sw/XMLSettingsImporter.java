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

import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.XController;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.view.XViewSettingsSupplier;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.comp.Writer.XMLSettingsImporter</code>. <p>
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
public class XMLSettingsImporter extends TestCase {
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
    * <code>com.sun.star.comp.Writer.XMLSettingsImporter</code><p>
    *
    * The text document is set as a target document for importer.
    * Imported XML-data contains only settings tags including
    * title tag with test zoom factor.
    * After import zoom factor getting from target document is checked.
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
        (TestParameters tParam, PrintWriter log ) throws Exception {

        XInterface oObj = null;
        Object oInt = null ;
        final short impZoom = 50 ;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        XMultiServiceFactory xMSF = tParam.getMSF() ;

        oInt = xMSF.createInstance
            ("com.sun.star.comp.Writer.XMLSettingsImporter") ;

        oObj = (XInterface) oInt ;

        // create testobject here
        log.println( "creating a new environment for Paragraph object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding relation
        tEnv.addObjRelation("TargetDocument", xTextDoc) ;

        // adding relation for XDocumentHandler
        String[][] xml = new String[][] {
            {"start", "office:document-settings",
                "xmlns:office", "CDATA", "http://openoffice.org/2000/office",
                "xmlns:config", "CDATA", "http://openoffice.org/2001/config",
                "xmlns:xlink", "CDATA", "http://www.w3.org/1999/xlink",
                },
                {"start", "office:settings"},
                    {"start", "config:config-item-set",
                        "config:name", "CDATA", "view-settings"
                    },
                        {"start", "config:config-item-map-indexed",
                            "config:name", "CDATA", "Views"},
                            {"start", "config:config-item-map-entry"},
                                {"start", "config:config-item",
                                    "config:name", "CDATA", "ZoomFactor",
                                    "config:type", "CDATA", "short"
                                },
                                    {"chars", String.valueOf(impZoom)},
                                {"end", "config:config-item"},
                                {"start", "config:config-item",
                                    "config:name", "CDATA", "ZoomType",
                                    "config:type", "CDATA", "short"
                                },
                                    {"chars", "0"},
                                {"end", "config:config-item"},
                            {"end", "config:config-item-map-entry"},
                        {"end", "config:config-item-map-indexed"},
                    {"end", "config:config-item-set"},
                {"end", "office:settings"},
            {"end", "office:document-settings"}} ;

        tEnv.addObjRelation("XDocumentHandler.XMLData", xml) ;

        final PrintWriter logF = log ;
        XController xController = xTextDoc.getCurrentController();
        XViewSettingsSupplier xViewSetSup = UnoRuntime.queryInterface(XViewSettingsSupplier.class, xController);
        final XPropertySet xPropSet = xViewSetSup.getViewSettings();
        tEnv.addObjRelation("XDocumentHandler.ImportChecker",
            new ifc.xml.sax._XDocumentHandler.ImportChecker() {
                public boolean checkImport() {
                    try {
                        Short gValue = (Short)
                            xPropSet.getPropertyValue("ZoomValue");
                        logF.println("ZoomValue property value = " + gValue) ;
                        return impZoom == gValue.shortValue() ;
                    } catch (com.sun.star.uno.Exception e) {
                        logF.println("Exception while checking import :") ;
                        e.printStackTrace(logF) ;
                        return false ;
                    }
                }
            }) ;

        return tEnv;
    } // finish method getTestEnvironment
}

