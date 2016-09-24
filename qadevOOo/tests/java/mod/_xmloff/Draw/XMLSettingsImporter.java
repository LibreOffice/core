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

package mod._xmloff.Draw;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.comp.Draw.XMLSettingsImporter</code>. <p>
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
    XComponent xDrawDoc = null;

    /**
    * New draw document created.
    */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF());

        log.println( "creating a drawdocument" );
        xDrawDoc = SOF.createDrawDoc(null);
    }

    /**
    * Text draw destroyed.
    */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing document " );
        xDrawDoc.dispose();
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.comp.Draw.XMLSettingsImporter</code><p>
    *
    * The draw document is set as a target document for importer.
    * Imported XML-data contains only settings tags including
    * tag with property 'IsLayerMode'.
    * After import property 'IsLayerMode' getting from target
    * document is checked.
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
            (TestParameters tParam, PrintWriter log) throws Exception {

        XInterface oObj = null;
        Object oInt = null ;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        XMultiServiceFactory xMSF = tParam.getMSF() ;

        oInt = xMSF.createInstance
            ("com.sun.star.comp.Draw.XMLSettingsImporter") ;

        oObj = (XInterface) oInt ;

        // create testobject here
        log.println( "creating a new environment for Paragraph object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding relation
        tEnv.addObjRelation("TargetDocument", xDrawDoc) ;

        // adding relation for XDocumentHandler
        String[][] xml = new String[][] {
            {"start", "office:document-settings",
                "xmlns:office", "CDATA", "http://openoffice.org/2000/office",
                "xmlns:config", "CDATA", "http://openoffice.org/2001/config"},
                {"start", "office:settings"},
                    {"start", "config:config-item-set",
                        "config:name", "CDATA", "view-settings"},
                        {"start", "config:config-item-map-indexed",
                            "config:name", "CDATA", "Views"},
                            {"start", "config:config-item-map-entry"},
                                {"start", "config:config-item",
                                    "config:name", "CDATA", "IsLayerMode",
                                    "config:type", "CDATA", "boolean"},
                                {"chars", "true"},
                                {"end", "config:config-item"},
                            {"end", "config:config-item-map-entry"},
                        {"end", "config:config-item-map-indexed"},
                    {"end", "config:config-item-set"},
                {"end", "office:settings"},
            {"end", "office:document-settings"} };

        tEnv.addObjRelation("XDocumentHandler.XMLData", xml);
        final PrintWriter logF = log;
        XModel xDrawModel = UnoRuntime.queryInterface(XModel.class, xDrawDoc);
        XController xController = xDrawModel.getCurrentController();
        final XPropertySet xPropSet = UnoRuntime.queryInterface(XPropertySet.class, xController);

        tEnv.addObjRelation("XDocumentHandler.ImportChecker",
            new ifc.xml.sax._XDocumentHandler.ImportChecker() {
                public boolean checkImport() {
                    try {
                        Boolean value = (Boolean)
                            xPropSet.getPropertyValue("IsLayerMode");
                        logF.println("'IsLayerMode' property value is '"
                            + value + "'");
                        return value.booleanValue();
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
