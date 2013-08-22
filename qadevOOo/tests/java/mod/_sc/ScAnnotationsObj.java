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

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSheetAnnotation;
import com.sun.star.sheet.XSheetAnnotationAnchor;
import com.sun.star.sheet.XSheetAnnotationsSupplier;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;
import com.sun.star.text.XSimpleText;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which represents a collection of annotations
* for a spreadsheet document (implements
* <code>com.sun.star.sheet.CellAnnotations</code>). <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XIndexAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::sheet::XSheetAnnotations</code></li>
* </ul> <p>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.sheet.CellAnnotations
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.sheet.XSheetAnnotations
* @see ifc.container._XIndexAccess
* @see ifc.container._XElementAccess
* @see ifc.sheet._XSheetAnnotations
*/
public class ScAnnotationsObj extends TestCase {
    private XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a Spreadsheet document" );
            xSheetDoc = SOF.createCalcDoc(null);
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }

    }

    /**
    * Disposes Spreadsheet document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = UnoRuntime.queryInterface (XComponent.class, xSheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * From a document collection of spreadsheets a single one is
    * retrieved and one annotation is added to it. Then a collection
    * of annotations is retrieved using spreadsheet's
    * <code>com.sun.star.sheet.XSheetAnnotationsSupplier</code> interface.
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "Creating a test environment" );

        log.println("Getting test object ") ;

        XSpreadsheetDocument xSpreadsheetDoc = UnoRuntime.queryInterface(XSpreadsheetDocument.class, xSheetDoc);
        XSpreadsheets sheets = xSpreadsheetDoc.getSheets();

        XNameAccess oNames = UnoRuntime.queryInterface( XNameAccess.class, sheets );
        XCell oCell = null;
        XSpreadsheet oSheet  = null;
        try {
            oSheet = (XSpreadsheet) AnyConverter.toObject(
                    new Type(XSpreadsheet.class),
                        oNames.getByName(oNames.getElementNames()[0]));
            // adding an annotation...
            XCellRange oCRange = UnoRuntime.queryInterface(XCellRange.class, oSheet);
            oCell = oCRange.getCellByPosition(10,10);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Error getting test object from spreadsheet document",e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log) ;
            throw new StatusException(
                "Error getting test object from spreadsheet document",e) ;
        } catch (com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace(log) ;
            throw new StatusException(
                "Error getting test object from spreadsheet document",e) ;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log) ;
            throw new StatusException(
                "Error getting test object from spreadsheet document",e) ;
        }

        XSheetAnnotationAnchor oAnnoA = UnoRuntime.queryInterface(XSheetAnnotationAnchor.class, oCell);
        XSheetAnnotation oAnno = oAnnoA.getAnnotation();
        XSimpleText sText = UnoRuntime.queryInterface(XSimpleText.class, oAnno);
        sText.setString("ScAnnotationsObj");

        XSheetAnnotationsSupplier supp = UnoRuntime.queryInterface(
            XSheetAnnotationsSupplier.class, oSheet);
        oObj = supp.getAnnotations();

        TestEnvironment tEnv = new TestEnvironment( oObj );

        return tEnv;
    }

}    // finish class ScAnnotationsObj


