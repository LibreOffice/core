/*************************************************************************
 *
 *  $RCSfile: ScCellRangeObj.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-05-27 13:02:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package mod._sc;

import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.SheetCellRange</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::table::CellProperties</code></li>
*  <li> <code>com::sun::star::util::XMergeable</code></li>
*  <li> <code>com::sun::star::sheet::XCellRangeAddressable</code></li>
*  <li> <code>com::sun::star::sheet::XArrayFormulaRange</code></li>
*  <li> <code>com::sun::star::chart::XChartData</code></li>
*  <li> <code>com::sun::star::table::XColumnRowRange</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::style::CharacterProperties</code></li>
*  <li> <code>com::sun::star::sheet::XSheetOperation</code></li>
*  <li> <code>com::sun::star::table::XCellRange</code></li>
*  <li> <code>com::sun::star::style::ParagraphProperties</code></li>
*  <li> <code>com::sun::star::sheet::SheetCellRange</code></li>
* </ul>
* @see com.sun.star.sheet.SheetCellRange
* @see com.sun.star.table.CellProperties
* @see com.sun.star.util.XMergeable
* @see com.sun.star.sheet.XCellRangeAddressable
* @see com.sun.star.sheet.XArrayFormulaRange
* @see com.sun.star.chart.XChartData
* @see com.sun.star.table.XColumnRowRange
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.style.CharacterProperties
* @see com.sun.star.sheet.XSheetOperation
* @see com.sun.star.table.XCellRange
* @see com.sun.star.style.ParagraphProperties
* @see com.sun.star.sheet.SheetCellRange
* @see ifc.table._CellProperties
* @see ifc.util._XMergeable
* @see ifc.sheet._XCellRangeAddressable
* @see ifc.sheet._XArrayFormulaRange
* @see ifc.chart._XChartData
* @see ifc.table._XColumnRowRange
* @see ifc.beans._XPropertySet
* @see ifc.style._CharacterProperties
* @see ifc.sheet._XSheetOperation
* @see ifc.table._XCellRange
* @see ifc.style._ParagraphProperties
* @see ifc.sheet._SheetCellRange
*/
public class ScCellRangeObj extends TestCase {
    XSpreadsheetDocument xSheetDoc = null;

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
            throw new StatusException( "Couldn³t create document", e );
        }
    }

    /**
    * Disposes Spreadsheet document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = (XComponent)
            UnoRuntime.queryInterface (XComponent.class, xSheetDoc);
        oComp.dispose();
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves a collection of spreadsheets from a document,
    * and takes one of them. Retrieves some cell range from the spreadsheet.
    * The retrieved cell range is instance of the service
    * <code>com.sun.star.sheet.SheetCellRange</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'SHEET'</code> for
    *      {@link ifc.sheet._XArrayFormulaRange} (the spreadsheet which the cell
    *      range was retrieved from)</li>
    * </ul>
    * @see com.sun.star.sheet.XSpreadsheet
    */
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oInterface = null;
        XInterface oObj = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "Creating a test environment" );

        XSpreadsheets oSpreadsheets = ( (XSpreadsheetDocument)
                                        UnoRuntime.queryInterface(
                                        XSpreadsheetDocument.class,
                                        xSheetDoc) ).getSheets() ;
        XNameAccess oNames = (XNameAccess)
            UnoRuntime.queryInterface( XNameAccess.class, oSpreadsheets);

        XSpreadsheet oSheet = null;
        try {
            oSheet = (XSpreadsheet) AnyConverter.toObject(
                new Type(XSpreadsheet.class),
                    oNames.getByName(oNames.getElementNames()[0]));

            oObj = oSheet.getCellRangeByPosition( 0, 0, 3, 4 );
        } catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Error getting cell object from spreadsheet document", e);
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Error getting cell object from spreadsheet document", e);
        } catch(com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Error getting cell object from spreadsheet document", e);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Error getting cell object from spreadsheet document", e);
        }

        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation( "SHEET", oSheet );

        XPropertySet PropSet = (XPropertySet)
                    UnoRuntime.queryInterface(XPropertySet.class, oObj);
        tEnv.addObjRelation("PropSet",PropSet);

        return tEnv;
    }

}    // finish class ScCellRangeObj

