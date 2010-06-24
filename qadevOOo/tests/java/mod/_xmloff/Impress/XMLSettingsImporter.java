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

package mod._xmloff.Impress;

import java.io.PrintWriter;

import lib.StatusException;
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
 * <code>com.sun.star.comp.Impress.XMLSettingsImporter</code>. <p>
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
    XComponent xImpressDoc = null;

    /**
    * New impress document created.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());

        try {
            log.println( "creating an impress document" );
            xImpressDoc = SOF.createImpressDoc(null);;
        } catch ( Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
    * Impress document destroyed.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing document " );
        xImpressDoc.dispose();
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.comp.Impress.XMLStylesImporter</code><p>
    *
    * The Impress document is set as a target document for importer.
    * Value of a property "IsLayerMode" has imported as 'true' to a target
    * document. After import property value getting from target document
    *  is checked.
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
        (TestParameters tParam, PrintWriter log ) throws StatusException {

        XInterface oObj = null;
        Object oInt = null ;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF() ;

        try {
            oInt = xMSF.createInstance
                ("com.sun.star.comp.Impress.XMLSettingsImporter") ;

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
            {"start", "office:document-settings",
                "xmlns:office", "CDATA", "http://openoffice.org/2000/office",
                "xmlns:xlink", "CDATA", "http://www.w3.org/1999/xlink",
                "xmlns:presentation", "CDATA",
                    "http://openoffice.org/2000/presentation",
                "xmlns:config", "CDATA", "http://openoffice.org/2001/config",
                "office:version", "CDATA", "1.0"},
            {"start", "office:settings"},
            {"start", "config:config-item-map-indexed", "config:name",
                "CDATA", "Views"},
            {"start", "config:config-item-map-entry"},
            {"start","config:config-item", "config:name", "CDATA",
                "IsLayerMode"},
            {"chars", "true"},
            {"end", "config:config-item"},
            {"end", "config:config-item-map-entry"},
            {"end", "config:config-item-map-indexed"},
            {"end", "office:settings"},
            {"end", "office:document-settings"}} ;

        tEnv.addObjRelation("XDocumentHandler.XMLData", xml) ;

        // get property before import
        XModel xImpressModel = (XModel)
            UnoRuntime.queryInterface(XModel.class, xImpressDoc);
        XController xController = xImpressModel.getCurrentController();
        final XPropertySet xPropSet = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class, xController);
        try {
            log.println("Property \"IsLayerMode\" before import is " +
                xPropSet.getPropertyValue("IsLayerMode"));
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Can't create component.", e);
        }
        final PrintWriter logF = log;
        tEnv.addObjRelation("XDocumentHandler.ImportChecker",
            new ifc.xml.sax._XDocumentHandler.ImportChecker() {
                public boolean checkImport() {
                    Boolean propValue;
                    try {
                        propValue = (Boolean) xPropSet.getPropertyValue
                            ("IsLayerMode");
                    } catch (com.sun.star.uno.Exception e) {
                        e.printStackTrace(logF);
                        throw new StatusException
                            ("Can't get property value.", e);
                    }
                    logF.println("Property \"IsLayerMode\" after import is "
                        + propValue);
                    if ( propValue.booleanValue() == true ) {
                        return true;
                    } else {
                        return false;
                    }
                }
            });

        return tEnv;
    } // end of getTestEnvironment

}
