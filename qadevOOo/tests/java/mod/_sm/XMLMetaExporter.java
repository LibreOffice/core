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

package mod._sm;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;
import util.XMLTools;

import com.sun.star.beans.XPropertyContainer;
import com.sun.star.document.XDocumentPropertiesSupplier;
import com.sun.star.document.XDocumentProperties;
import com.sun.star.document.XExporter;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Any;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.xml.sax.XDocumentHandler;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.comp.Math.XMLExporter</code>. <p>
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
    XComponent xMathDoc;

    /**
     * New math document created.
     */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );

        log.println( "creating a math document" );
        xMathDoc = SOF.createMathDoc(null);
    }

    /**
     * Document disposed here.
     */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xMathDoc " );
        xMathDoc.dispose();
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.comp.Math.XMLExporter</code> with
    * argument which is an implementation of <code>XDocumentHandler</code>
    * and which can check if required tags and character data is
    * exported. <p>
    * The math document is set as a source document for exporter
    * created. A new user info field inserted into document. This made
    * for checking if this info field is successfully exported within
    * the document content.
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
    @Override
    protected TestEnvironment createTestEnvironment
            (TestParameters tParam, PrintWriter log) throws Exception {
        XMultiServiceFactory xMSF = tParam.getMSF() ;
        XInterface oObj = null;
        final String expName = "XMLMetaExporterName" ;
        final String expValue = "XMLMetaExporterValue" ;

        FilterChecker filter = new FilterChecker(log);
        Any arg = new Any(new Type(XDocumentHandler.class), filter);

        oObj = (XInterface) xMSF.createInstanceWithArguments(
            "com.sun.star.comp.Math.XMLMetaExporter", new Object[] {arg});
        XExporter xEx = UnoRuntime.queryInterface
            (XExporter.class,oObj);
        xEx.setSourceDocument(xMathDoc);

        // setting a new name and value for user info field
        XDocumentPropertiesSupplier xPropSup = UnoRuntime.queryInterface
            (XDocumentPropertiesSupplier.class, xMathDoc);
        final XDocumentProperties xDocProps = xPropSup.getDocumentProperties();
        XPropertyContainer xProps = xDocProps.getUserDefinedProperties();
        xProps.addProperty(expName, (short)0, expValue);

        // checking tags required
        filter.addTag(new XMLTools.Tag("office:document-meta")) ;
        filter.addCharactersEnclosed(expValue,
            new XMLTools.Tag("meta:user-defined", "meta:name", expName)) ;

        // create testobject here
        log.println( "creating a new environment" );
        TestEnvironment tEnv = new TestEnvironment( oObj );


        tEnv.addObjRelation("MediaDescriptor", XMLTools.createMediaDescriptor(
            new String[] {"FilterName"},
            new Object[] {"smath: StarOffice XML (Math)"}));
        tEnv.addObjRelation("SourceDocument",xMathDoc);
        tEnv.addObjRelation("XFilter.Checker", filter) ;

        log.println("Implementation Name: "+util.utils.getImplName(oObj));

        return tEnv;
    }

    /**
     * This class checks the XML for tags and data required and returns
     * checking result to <code>XFilter</code> interface test. All
     * the information about errors occurred in XML data is written
     * to log specified.
     * @see ifc.document._XFilter
     */
    private class FilterChecker extends XMLTools.XMLChecker
        implements ifc.document._XFilter.FilterChecker {

        /**
         * Creates a class which will write information
         * into log specified.
         */
        private FilterChecker(PrintWriter log) {
            super(log, true) ;
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
}    // finish class TestCase

