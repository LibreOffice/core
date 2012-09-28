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
import util.XMLTools;

import com.sun.star.beans.XPropertySet;
import com.sun.star.document.XDocumentInfoSupplier;
import com.sun.star.document.XExporter;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Any;
import com.sun.star.uno.Exception;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.xml.sax.XDocumentHandler;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.comp.Impress.XMLMetaExporter</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li><code>com::sun::star::lang::XInitialization</code></li>
 *  <li><code>com::sun::star::document::ExportFilter</code></li>
 *  <li><code>com::sun::star::document::XFilter</code></li>
 *  <li><code>com::sun::star::document::XExporter</code></li>
 *  <li><code>com::sun::star::beans::XPropertySet</code></li>
 * </ul>
 * @see com.sun.star.lang.XInitialization
 * @see com.sun.star.document.ExportFilter
 * @see com.sun.star.document.XFilter
 * @see com.sun.star.document.XExporter
 * @see com.sun.star.beans.XPropertySet
 * @see ifc.lang._XInitialization
 * @see ifc.document._ExportFilter
 * @see ifc.document._XFilter
 * @see ifc.document._XExporter
 * @see ifc.beans._XPropertySet
 */
public class XMLMetaExporter extends TestCase {
    XComponent xImpressDoc = null;

    /**
    * New impress document created.
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
    * Impress document disposed
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "disposing xImpressDoc " );
        xImpressDoc.dispose();
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.comp.Impress.XMLMetaExporter</code> with
    * argument which is an implementation of <code>XDocumentHandler</code>
    * and which can check if required tags and character data is
    * exported. <p>
    * The impress document is set as a source document for exporter
    * created. A meta property 'Title' is set to specific value. This made
    * for checking if this value is successfully exported within
    * the document meta information.
    *     Object relations created :
    * <ul>
    *  <li> <code>'MediaDescriptor'</code> for
    *      {@link ifc.document._XFilter} interface </li>
    *  <li> <code>'XFilter.Checker'</code> for
    *      {@link ifc.document._XFilter} interface </li>
    *  <li> <code>'SourceDocument'</code> for
    *      {@link ifc.document._XExporter} interface </li>
    * </ul>
    */
    public synchronized TestEnvironment createTestEnvironment
        (TestParameters tParam, PrintWriter log) throws StatusException {

        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF() ;
        XInterface oObj = null;

        FilterChecker filter = new FilterChecker(log);
        Any arg = new Any(new Type(XDocumentHandler.class), filter);
        final String NAME = "XMLMetaExporter";

        try {
            oObj = (XInterface) xMSF.createInstanceWithArguments(
                "com.sun.star.comp.Impress.XMLMetaExporter",
                new Object[]{arg});
            XExporter xEx = UnoRuntime.queryInterface(XExporter.class,oObj);
            xEx.setSourceDocument(xImpressDoc);

        //change title name
            XDocumentInfoSupplier infoSup = UnoRuntime.queryInterface
            (XDocumentInfoSupplier.class, xImpressDoc) ;
            XPropertySet docInfo = UnoRuntime.queryInterface
                (XPropertySet.class, infoSup.getDocumentInfo()) ;
            docInfo.setPropertyValue("Title", NAME);

        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log) ;
            throw new StatusException("Can't create component.", e) ;
        }

        // Checking tags existance and changed property value
        filter.addTag(new XMLTools.Tag ("office:document-meta"));
        filter.addTagEnclosed(
            new XMLTools.Tag("office:meta"),
            new XMLTools.Tag("office:document-meta") );
        filter.addTagEnclosed(
            new XMLTools.Tag("dc:title"),
            new XMLTools.Tag("office:meta") );
        filter.addCharactersEnclosed(
            NAME,
            new XMLTools.Tag ("dc:title") );

        // create testobject here
        log.println( "creating a new environment" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("MediaDescriptor", XMLTools.createMediaDescriptor(
            new String[] {"FilterName"},
            new Object[] {"simpress: StarOffice XML (Impress)"}));
        tEnv.addObjRelation("SourceDocument", xImpressDoc);
        tEnv.addObjRelation("XFilter.Checker", filter) ;
        return tEnv;
    }

    /**
    * This class checks the XML for tags and data required and returns
    * checking result to <code>XFilter</code> interface test. All
    * the information about errors occurred in XML data is written
    * to log specified.
    * @see ifc.document._XFilter
    */
    protected class FilterChecker extends XMLTools.XMLChecker
            implements ifc.document._XFilter.FilterChecker {

        /**
        * Creates a class which will write information
        * into log specified.
        */
        public FilterChecker(PrintWriter log) {
            super(log, true);
        }
        /**
        * <code>_XFilter.FilterChecker</code> interface method
        * which returns the result of XML checking.
        * @return <code>true</code> if the XML data exported was
        * valid (i.e. all necessary tags and character data exists),
        * <code>false</code> if some errors occurred.
        */
        public boolean checkFilter() {
            return check();
        }
    }
}
