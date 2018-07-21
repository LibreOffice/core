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

import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.document.XExporter;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.style.XStyle;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.uno.Any;
import com.sun.star.uno.Exception;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.xml.sax.XDocumentHandler;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.comp.Draw.XMLStylesExporter</code>. <p>
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
public class XMLStylesExporter extends TestCase {
    XComponent xDrawDoc = null;
    protected static int counter = 0;

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
    * <code>com.sun.star.comp.Draw.XMLStylesExporter</code> with
    * argument which is an implementation of <code>XDocumentHandler</code>
    * and which can check if required tags and character data is
    * exported. <p>
    * The draw document is set as a source document for exporter
    * created. New style is added to the style family 'graphics' with unique name
    * every time. This made for checking if this style is successfully exported
    * within the document styles information.
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
    public TestEnvironment createTestEnvironment(
        TestParameters tParam, PrintWriter log) throws Exception {

        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );
        XMultiServiceFactory xMSF = tParam.getMSF() ;
        XInterface oObj = null;
        FilterChecker filter = new FilterChecker(log) ;
        Any arg = new Any(new Type(XDocumentHandler.class),filter);
        final String STYLE_NAME = "New style" + counter++ ;

        oObj = (XInterface) xMSF.createInstanceWithArguments(
            "com.sun.star.comp.Draw.XMLStylesExporter", new Object[] {arg});
        XExporter xEx = UnoRuntime.queryInterface(XExporter.class,oObj);
        xEx.setSourceDocument(xDrawDoc);

        //obtain style families
        XStyleFamiliesSupplier styleSup = UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, xDrawDoc);
        XNameAccess StyleFamilies = styleSup.getStyleFamilies();
        //obtain all style family names
        String[] styleFamiliesNames = StyleFamilies.getElementNames();
        String styleFamilyName = styleFamiliesNames[0];
        //obtain style family with name[0]
        Object objectStyle = StyleFamilies.getByName(styleFamilyName);
        XNameContainer xStyleFamilyName = UnoRuntime.queryInterface(XNameContainer.class, objectStyle);
        //create new style
        Object SC = SOF.createInstance(xDrawDoc, "com.sun.star.style.Style");
        XStyle Style = UnoRuntime.queryInterface(XStyle.class,SC);
        //add new style to style family with name[0]
        xStyleFamilyName.insertByName(STYLE_NAME,Style);

        // Checking Head Tag existence and that property has changed
        filter.addTag(new XMLTools.Tag ("office:document-styles"));
        filter.addTag(new XMLTools.Tag ("office:styles"));
        filter.addTagEnclosed(
            new XMLTools.Tag("style:style"),
            new XMLTools.Tag("office:styles") );
        filter.addTag(new XMLTools.Tag("style:style", "style:name", STYLE_NAME));

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
            super(log, false) ;
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

