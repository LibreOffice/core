/*************************************************************************
 *
 *  $RCSfile: ScTableSheetObj.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-05-27 13:15:06 $
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
import com.sun.star.container.XIndexAccess;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XDesktop;
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
* <code>com.sun.star.sheet.Spreadsheet</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::style::ParagraphProperties</code></li>
*  <li> <code>com::sun::star::table::XTableChartsSupplier</code></li>
*  <li> <code>com::sun::star::sheet::XSpreadsheet</code></li>
*  <li> <code>com::sun::star::table::CellProperties</code></li>
*  <li> <code>com::sun::star::sheet::XSheetAnnotationsSupplier</code></li>
*  <li> <code>com::sun::star::table::XCellRange</code></li>
*  <li> <code>com::sun::star::sheet::XCellRangeAddressable</code></li>
*  <li> <code>com::sun::star::sheet::XDataPilotTablesSupplier</code></li>
*  <li> <code>com::sun::star::sheet::Spreadsheet</code></li>
*  <li> <code>com::sun::star::sheet::XCellRangeMovement</code></li>
*  <li> <code>com::sun::star::style::CharacterProperties</code></li>
*  <li> <code>com::sun::star::sheet::XSheetOperation</code></li>
*  <li> <code>com::sun::star::sheet::XScenariosSupplier</code></li>
*  <li> <code>com::sun::star::sheet::XSheetPageBreak</code></li>
*  <li> <code>com::sun::star::sheet::XArrayFormulaRange</code></li>
*  <li> <code>com::sun::star::sheet::XSheetCellRange</code></li>
*  <li> <code>com::sun::star::container::XNamed</code></li>
*  <li> <code>com::sun::star::sheet::SheetCellRange</code></li>
*  <li> <code>com::sun::star::chart::XChartData</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::util::XMergeable</code></li>
*  <li> <code>com::sun::star::table::XColumnRowRange</code></li>
* </ul>
* @see com.sun.star.style.ParagraphProperties
* @see com.sun.star.table.XTableChartsSupplier
* @see com.sun.star.sheet.XSpreadsheet
* @see com.sun.star.table.CellProperties
* @see com.sun.star.sheet.XSheetAnnotationsSupplier
* @see com.sun.star.table.XCellRange
* @see com.sun.star.sheet.XCellRangeAddressable
* @see com.sun.star.sheet.XDataPilotTablesSupplier
* @see com.sun.star.sheet.Spreadsheet
* @see com.sun.star.sheet.XCellRangeMovement
* @see com.sun.star.style.CharacterProperties
* @see com.sun.star.sheet.XSheetOperation
* @see com.sun.star.sheet.XScenariosSupplier
* @see com.sun.star.sheet.XSheetPageBreak
* @see com.sun.star.sheet.XArrayFormulaRange
* @see com.sun.star.sheet.XSheetCellRange
* @see com.sun.star.container.XNamed
* @see com.sun.star.sheet.SheetCellRange
* @see com.sun.star.chart.XChartData
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.util.XMergeable
* @see com.sun.star.table.XColumnRowRange
* @see ifc.style._ParagraphProperties
* @see ifc.table._XTableChartsSupplier
* @see ifc.sheet._XSpreadsheet
* @see ifc.table._CellProperties
* @see ifc.sheet._XSheetAnnotationsSupplier
* @see ifc.table._XCellRange
* @see ifc.sheet._XCellRangeAddressable
* @see ifc.sheet._XDataPilotTablesSupplier
* @see ifc.sheet._Spreadsheet
* @see ifc.sheet._XCellRangeMovement
* @see ifc.style._CharacterProperties
* @see ifc.sheet._XSheetOperation
* @see ifc.sheet._XScenariosSupplier
* @see ifc.sheet._XSheetPageBreak
* @see ifc.sheet._XArrayFormulaRange
* @see ifc.sheet._XSheetCellRange
* @see ifc.container._XNamed
* @see ifc.sheet._SheetCellRange
* @see ifc.chart._XChartData
* @see ifc.beans._XPropertySet
* @see ifc.util._XMergeable
* @see ifc.table._XColumnRowRange
*/
public class ScTableSheetObj extends TestCase {
    XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());

        try {
            log.println( "creating a sheetdocument" );
            xSheetDoc = SOF.createCalcDoc(null);;
        } catch (com.sun.star.uno.Exception e) {
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
            UnoRuntime.queryInterface (XComponent.class, xSheetDoc);
        oComp.dispose();
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves a collection of spreadsheets from the document and takes one of
    * them. Fills some cell in the spreadsheet. The retrieved spreadsheet is the
    * instance of the service <code>com.sun.star.sheet.Spreadsheet</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'noArray'</code> for
    *      {@link ifc.sheet._XArrayFormulaRange}(to avoid the test of
    *      the interface <code>XArrayFormulaRange</code>)</li>
    * </ul>
    * @see com.sun.star.sheet.XArrayFormulaRange
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oInterface = null;
        XInterface oObj = null;
        XComponentLoader oCLoader = null;
        XDesktop oDesktop = null;

        log.println("getting sheets");
        XSpreadsheets xSpreadsheets = (XSpreadsheets)xSheetDoc.getSheets();

        log.println("getting a sheet");
        XSpreadsheet oSheet = null;
        XIndexAccess oIndexAccess = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class, xSpreadsheets);
        try {
            oSheet = (XSpreadsheet) AnyConverter.toObject(
                    new Type(XSpreadsheet.class),oIndexAccess.getByIndex(0));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get a spreadsheet", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get a spreadsheet", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get a spreadsheet", e);
        }

        log.println("filling some cells");
        try {
            oSheet.getCellByPosition(5, 5).setValue(15);
            oSheet.getCellByPosition(1, 4).setValue(10);
            oSheet.getCellByPosition(2, 0).setValue(-5.15);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Exception occurred while filling cells", e);
        }

        oObj = (XInterface)UnoRuntime.queryInterface(XInterface.class, oSheet);

        log.println("creating a new environment for object");
        TestEnvironment tEnv = new TestEnvironment(oObj);

        log.println("adding ObjRelation 'noArray' to avoid the test" +
            " 'XArrayFormulaRange'");
        tEnv.addObjRelation( "noArray", "ScTableSheetObj" );

        XPropertySet PropSet = (XPropertySet)
                    UnoRuntime.queryInterface(XPropertySet.class, oObj);
        tEnv.addObjRelation("PropSet",PropSet);

        return tEnv;
    }
}

