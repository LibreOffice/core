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

import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCell;
import com.sun.star.text.XText;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.text.TextCursor</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::text::XTextCursor</code></li>
*  <li> <code>com::sun::star::text::XWordCursor</code></li>
*  <li> <code>com::sun::star::style::CharacterPropertiesComplex</code></li>
*  <li> <code>com::sun::star::text::XTextRange</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::container::XContentEnumerationAccess</code></li>
*  <li> <code>com::sun::star::beans::XPropertyState</code></li>
*  <li> <code>com::sun::star::style::CharacterProperties</code></li>
*  <li> <code>com::sun::star::text::XSentenceCursor</code></li>
*  <li> <code>com::sun::star::style::ParagraphProperties</code></li>
*  <li> <code>com::sun::star::text::XParagraphCursor</code></li>
*  <li> <code>com::sun::star::document::XDocumentInsertable</code></li>
*  <li> <code>com::sun::star::util::XSortable</code></li>
*  <li> <code>com::sun::star::style::CharacterPropertiesAsian</code></li>
* </ul>
* @see com.sun.star.text.TextCursor
* @see com.sun.star.text.XTextCursor
* @see com.sun.star.text.XWordCursor
* @see com.sun.star.style.CharacterPropertiesComplex
* @see com.sun.star.text.XTextRange
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.container.XContentEnumerationAccess
* @see com.sun.star.beans.XPropertyState
* @see com.sun.star.style.CharacterProperties
* @see com.sun.star.text.XSentenceCursor
* @see com.sun.star.style.ParagraphProperties
* @see com.sun.star.text.XParagraphCursor
* @see com.sun.star.document.XDocumentInsertable
* @see com.sun.star.util.XSortable
* @see com.sun.star.style.CharacterPropertiesAsian
* @see ifc.text._XTextCursor
* @see ifc.text._XWordCursor
* @see ifc.style._CharacterPropertiesComplex
* @see ifc.text._XTextRange
* @see ifc.beans._XPropertySet
* @see ifc.container._XContentEnumerationAccess
* @see ifc.beans._XPropertyState
* @see ifc.style._CharacterProperties
* @see ifc.text._XSentenceCursor
* @see ifc.style._ParagraphProperties
* @see ifc.text._XParagraphCursor
* @see ifc.document._XDocumentInsertable
* @see ifc.util._XSortable
* @see ifc.style._CharacterPropertiesAsian
*/
public class ScCellTextCursor extends TestCase {
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
    * Retrieves a collection of spreadsheets from a document
    * and takes one of them. Retrieves the cell from the spreadsheet
    * and creates text cursor of the cell using the interface
    * <code>XText</code>. This text cursor is the instance of
    * the service <code>com.sun.star.text.TextCursor</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'XTEXT'</code> for
    *      {@link ifc.text._XTextRange} (type of
    *      <code>XEnumerationAccess</code> that was retrieved from the
    *      collection of visible cells)</li>
    * </ul>
    * @see com.sun.star.text.XText
    */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;
        XCell aCell = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "Creating a test environment" );

        // get a soffice factory object
        try {
            log.println("Getting spreadsheet") ;
            XSpreadsheets oSheets = xSheetDoc.getSheets() ;
            XIndexAccess oIndexSheets = UnoRuntime.queryInterface(XIndexAccess.class, oSheets);
            XSpreadsheet oSheet = (XSpreadsheet) AnyConverter.toObject(
                    new Type(XSpreadsheet.class),oIndexSheets.getByIndex(0));

            log.println("Getting a cell from sheet") ;
            aCell = oSheet.getCellByPosition(1,1) ;

        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log) ;
            throw new StatusException(
                "Error getting cell object from spreadsheet document",e) ;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log) ;
            throw new StatusException(
                "Error getting cell object from spreadsheet document",e) ;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log) ;
            throw new StatusException(
                "Error getting cell object from spreadsheet document",e) ;
        }

        XText aText = UnoRuntime.queryInterface(XText.class, aCell);
        aText.setString("ScCellTextCursor");
        oObj = aText.createTextCursor();

        log.println( "creating a new environment for ScCellTextCursor object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // Object relations for interface tests
        tEnv.addObjRelation("XTEXT", aText);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class ScCellTextCursor

