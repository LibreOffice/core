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

package mod._sc;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.document.XImporter;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.comp.Calc.XMLContentImporter</code>. <p>
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
    static XSpreadsheetDocument xSheetDoc;
    static XComponent comp ;

    /**
    * New spreadsheet document created.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a Spreadsheet document" );
            xSheetDoc = SOF.createCalcDoc( null );
            comp = (XComponent) UnoRuntime.queryInterface
                (XComponent.class, xSheetDoc) ;
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
    * Spreadsheet document destroyed.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing document " );
        util.DesktopTools.closeDoc(comp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.comp.Calc.XMLContentImporter</code><p>
    *
    * The calc document is set as a target document for importer.
    * Imported XML-data contains only content tags including table
    * with test table name.
    * After import table name getting from
    * target document is checked.
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
    public synchronized TestEnvironment createTestEnvironment( TestParameters tParam,
                                                  PrintWriter log )
                                                    throws StatusException {

        XInterface oObj = null;
        Object oInt = null ;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF() ;

        try {
            oInt = xMSF.createInstance
                ("com.sun.star.comp.Calc.XMLContentImporter") ;
            XImporter imp = (XImporter) UnoRuntime.queryInterface
                (XImporter.class, oInt) ;
            imp.setTargetDocument(comp);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log) ;
            throw new StatusException("Can't create component.", e) ;
        }

        oObj = (XInterface) oInt ;

        // create testobject here
        log.println( "creating a new environment for Paragraph object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding relations
        tEnv.addObjRelation("TargetDocument", comp) ;

        final String impValue = "XMLContentImporter_test" ;
        // adding relation for XDocumentHandler
        String[][] xml = new String[][] {
            {"start", "office:document-content",
                "xmlns:office", "CDATA", "http://openoffice.org/2000/office",
                "xmlns:text", "CDATA", "http://openoffice.org/2000/text",
                "xmlns:table", "CDATA", "http://openoffice.org/2000/table",
                "office:class", "CDATA", "spreadsheet"
                },
            {"start", "office:body"},
            {"start", "table:table",
                "table:name", "CDATA", impValue},
            {"end", "table:table"},
            {"end", "office:body"},
            {"end", "office:document-content"}} ;

        tEnv.addObjRelation("XDocumentHandler.XMLData", xml) ;

        //save to log content before import
        final PrintWriter fLog = log;
        final XSpreadsheets xSheets = xSheetDoc.getSheets();

        log.println("Sheets before importing :") ;
        String[] names = xSheets.getElementNames() ;
        for (int i = 0; i < names.length; i++) {
            log.println("  " + names[i]) ;
        }

        tEnv.addObjRelation("XDocumentHandler.ImportChecker",
            new ifc.xml.sax._XDocumentHandler.ImportChecker() {
                public boolean checkImport() {
                    fLog.println("Sheet names :") ;
                    String[] snames = xSheets.getElementNames() ;
                    for (int i = 0; i < snames.length; i++) {
                        fLog.println("  " + snames[i]) ;
                    }
                    return xSheets.hasByName(impValue) ;
                }
            }) ;

        return tEnv;
    } // finish method getTestEnvironment
}

