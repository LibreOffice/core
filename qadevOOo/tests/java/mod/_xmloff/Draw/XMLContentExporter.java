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
import util.XMLTools;

import com.sun.star.container.XNamed;
import com.sun.star.document.XExporter;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
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
 * <code>com.sun.star.comp.Draw.XMLContentExporter</code>. <p>
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
public class XMLContentExporter extends TestCase {
    XComponent xDrawDoc = null;

    /**
    * New text document created.
    */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF());

        log.println( "creating a drawdocument" );
        xDrawDoc = SOF.createDrawDoc(null);
    }

    /**
     * Document disposed here.
     */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xDrawDoc " );
        xDrawDoc.dispose();
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.comp.Draw.XMLContentExporter</code> with
    * argument which is an implementation of <code>XDocumentHandler</code>
    * and which can check if required tags and character data is
    * exported. <p>
    * The draw document is set as a source document for exporter
    * created. Then a new page (Slide) created in this document and
    * both two pages renamed to 'NewSlide1' and 'NewSlide2'. After this
    * filter checks that new tags exist in the XML output.
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
        FilterChecker filter = new FilterChecker(log);
        Any arg = new Any(new Type(XDocumentHandler.class),filter);

        // Adding tags for checking existence of head tag and other tags
        filter.addTag(new XMLTools.Tag("office:document-content"));
        filter.addTag(new XMLTools.Tag("office:body"));
        filter.addTagEnclosed(
            new XMLTools.Tag("draw:page"),
            new XMLTools.Tag("office:body"));
        filter.addTag(new XMLTools.Tag("draw:page","draw:name","NewSlide1"));
        filter.addTag(new XMLTools.Tag("draw:page","draw:name","NewSlide2"));

        oObj = (XInterface) xMSF.createInstanceWithArguments(
            "com.sun.star.comp.Draw.XMLContentExporter",
            new Object[] {arg});
        XExporter xEx = UnoRuntime.queryInterface(XExporter.class,oObj);

        XDrawPagesSupplier supp = UnoRuntime.queryInterface(XDrawPagesSupplier.class, xDrawDoc);
        XDrawPages set = supp.getDrawPages();

        // This is an XML-export BUG (new slide named "NewSlide2"
        // can not be exported to XML)
        set.insertNewByIndex(1);

        XDrawPage page1 = UnoRuntime.queryInterface(XDrawPage.class, set.getByIndex(0));
        XNamed NPage1 = UnoRuntime.queryInterface(XNamed.class,page1);
        NPage1.setName("NewSlide1");
        XDrawPage page2 = UnoRuntime.queryInterface(XDrawPage.class, set.getByIndex(1));
        XNamed NPage2 = UnoRuntime.queryInterface(XNamed.class,page2);
        NPage2.setName("NewSlide2");
        xEx.setSourceDocument(xDrawDoc);

        // create testobject here
        log.println( "creating a new environment" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("MediaDescriptor", XMLTools.createMediaDescriptor(
            new String[] {"FilterName"},
            new Object[] {"sdraw: StarOffice XML (Draw)"}));
        tEnv.addObjRelation("SourceDocument",xDrawDoc);
        tEnv.addObjRelation("XFilter.Checker", filter);
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
            super(log,true) ;
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


