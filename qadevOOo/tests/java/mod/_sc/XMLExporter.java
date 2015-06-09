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

package mod._sc;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;
import util.XMLTools;

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNamed;
import com.sun.star.document.XExporter;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.xml.sax.XDocumentHandler;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.comp.Calc.XMLExporter</code>. <p>
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
public class XMLExporter extends TestCase {
    static XComponent xSheetDoc;

    /**
     * New spreadsheet document created.
     */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );
        log.println( "creating a calc document" );
        xSheetDoc = SOF.openDoc("scalc","_blank");
    }

    /**
     * Spreadsheet document disposed
     */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xCalcDoc " );
        util.DesktopTools.closeDoc(xSheetDoc);
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.comp.Calc.XMLExporter</code> with
    * argument which is an implementation of <code>XDocumentHandler</code>
    * and which can check if required tags and character data is
    * exported. <p>
    * The calc document is set as a source document for exporter
    * created. Checking whether tags for table corresponding to some sheet
    * is exported.<p>
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
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) throws Exception {

        XMultiServiceFactory xMSF = tParam.getMSF() ;
        XInterface oObj = null;
        FilterChecker Filter = new FilterChecker(log);
        Any arg = new Any(new Type(XDocumentHandler.class), Filter);
        final String SHEET_NAME = "XMLExporter_SheetTestName";

        oObj = (XInterface) xMSF.createInstanceWithArguments(
            "com.sun.star.comp.Calc.XMLExporter", new Object[] {arg} );
        XExporter xEx = UnoRuntime.queryInterface
            (XExporter.class,oObj);
        xEx.setSourceDocument(xSheetDoc);

        //set name of sheet
        XSpreadsheetDocument xSpreadsheetDoc = UnoRuntime.queryInterface(XSpreadsheetDocument.class, xSheetDoc);
        XSpreadsheets xSpreadsheets = xSpreadsheetDoc.getSheets();
        XIndexAccess xSheetsIndexArray = UnoRuntime.queryInterface(XIndexAccess.class, xSpreadsheets);
        XSpreadsheet xSheet = (XSpreadsheet) AnyConverter.toObject(
                new Type(XSpreadsheet.class),xSheetsIndexArray.getByIndex(0));
        XNamed xSheetNamed = UnoRuntime.queryInterface(XNamed.class, xSheet);
        xSheetNamed.setName(SHEET_NAME);

        log.println("fill sheet with contnet...");
        util.CalcTools.fillCalcSheetWithContent(xSheet, 3, 3, 50, 50);

        // adding tags which must be contained in XML output
        Filter.addTag( new XMLTools.Tag("office:document") );
        Filter.addTagEnclosed(
            new XMLTools.Tag("office:meta"),
            new XMLTools.Tag("office:document") );
        Filter.addTagEnclosed(
            new XMLTools.Tag("office:settings"),
            new XMLTools.Tag("office:document") );
        Filter.addTagEnclosed(
            new XMLTools.Tag("office:script"),
            new XMLTools.Tag("office:document") );
        Filter.addTagEnclosed(
            new XMLTools.Tag("office:styles"),
            new XMLTools.Tag("office:document") );
        Filter.addTagEnclosed(
            new XMLTools.Tag("office:body"),
            new XMLTools.Tag("office:document") );
        Filter.addTagEnclosed(
            new XMLTools.Tag("table:table"),
            new XMLTools.Tag("office:body") );
        Filter.addTag(
            new XMLTools.Tag("table:table", "table:name", SHEET_NAME) );

        // create testobject here
        log.println( "creating a new environment" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("MediaDescriptor", XMLTools.createMediaDescriptor(
            new String[] {"FilterName"},
            new Object[] {"scalc: StarOffice XML (Calc)"}));
        tEnv.addObjRelation("SourceDocument",xSheetDoc);
        tEnv.addObjRelation("XFilter.Checker", Filter) ;
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

