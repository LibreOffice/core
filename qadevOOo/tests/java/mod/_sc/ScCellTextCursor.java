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
    static XSpreadsheetDocument xSheetDoc = null;

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
        XComponent oComp = (XComponent)
            UnoRuntime.queryInterface (XComponent.class, xSheetDoc) ;
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
            XIndexAccess oIndexSheets = (XIndexAccess)
                UnoRuntime.queryInterface(XIndexAccess.class, oSheets);
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

        XText aText = (XText) UnoRuntime.queryInterface(XText.class, aCell);
        aText.setString("ScCellTextCursor");
        oObj = aText.createTextCursor();

        log.println( "creating a new environment for ScCellTextCursor object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // Object relations for interface tests
        tEnv.addObjRelation("XTEXT", aText);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class ScCellTextCursor

