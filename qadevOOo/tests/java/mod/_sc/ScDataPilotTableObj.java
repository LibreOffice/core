/*************************************************************************
 *
 *  $RCSfile: ScDataPilotTableObj.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 12:08:14 $
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

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.TableFilterField;
import com.sun.star.sheet.XDataPilotDescriptor;
import com.sun.star.sheet.XDataPilotTables;
import com.sun.star.sheet.XDataPilotTablesSupplier;
import com.sun.star.sheet.XSheetFilterDescriptor;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.CellAddress;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.DataPilotTable</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNamed</code></li>
*  <li> <code>com::sun::star::sheet::XDataPilotTable</code></li>
*  <li> <code>com::sun::star::sheet::XDataPilotDescriptor</code></li>
* </ul>
* @see com.sun.star.sheet.DataPilotTable
* @see com.sun.star.container.XNamed
* @see com.sun.star.sheet.XDataPilotTable
* @see com.sun.star.sheet.XDataPilotDescriptor
* @see ifc.container._XNamed
* @see ifc.sheet._XDataPilotTable
* @see ifc.sheet._XDataPilotDescriptor
*/
public class ScDataPilotTableObj extends TestCase {
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
            UnoRuntime.queryInterface(XComponent.class, xSheetDoc) ;
        oComp.dispose();
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves a collection of spreadsheets from a document
    * and takes one of them. Fills some table in the spreadsheet.
    * Obtains the collection of data pilot tables using the interface
    * <code>XDataPilotTablesSupplier</code>. Creates a data pilot descriptor
    * for the filled table. Obtains the collection of all the data pilot fields
    * using the interface <code>XDataPilotDescriptor</code>. Sets field
    * orientation for every of the data pilot fields. Sets the definitions of
    * the filter fields for the created description. Inserts new data pilot
    * table with this descriptor to the collection.
    * This new data pilot table is the instance of the service
    * <code>com.sun.star.sheet.DataPilotTable</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'OUTPUTRANGE'</code> for
    *      {@link ifc.sheet._XDataPilotTable}(the cell range address of the
    *      created data pilot table) </li>
    * <li> <code>'FIELDSAMOUNT'</code> for
    *      {@link ifc.sheet._XDataPilotDescriptor}(the number of filled fields
    *      which descriptor was created for) </li>
    *  <li> <code>'CELLFORCHANGE'</code> for
    *      {@link ifc.sheet._XDataPilotTable}(value of this cell will be changed)</li>
    *  <li> <code>'CELLFORCHECK'</code> for
    *      {@link ifc.sheet._XDataPilotTable}(value of this cell must be changed
    *      after refresh call)</li>
    * </ul>
    * @see com.sun.star.sheet.DataPilotTable
    * @see com.sun.star.sheet.XDataPilotTablesSupplier
    * @see com.sun.star.sheet.XDataPilotDescriptor
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        CellAddress sCellAddress = new CellAddress();
        sCellAddress.Sheet = 0;
        sCellAddress.Column = 7;
        sCellAddress.Row = 8;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "Creating a test environment" );
        log.println("getting sheets");
        XSpreadsheets xSpreadsheets = (XSpreadsheets)xSheetDoc.getSheets();
        XIndexAccess oIndexAccess = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class, xSpreadsheets);
        XSpreadsheet oSheet = null;
        Object oChangeCell = null;
        Object oCheckCell = null;
        try {
            oSheet = (XSpreadsheet) AnyConverter.toObject(
                    new Type(XSpreadsheet.class),oIndexAccess.getByIndex(0));
            oChangeCell = oSheet.getCellByPosition(1, 5);
            oCheckCell = oSheet.getCellByPosition(
                sCellAddress.Column, sCellAddress.Row + 3);
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
        log.println("Getting sheet - " + (oSheet == null ? "FAILED" : "OK"));
        try {
            log.println("Filing a table");
            for (int i = 1; i < 6; i++) {
                oSheet.getCellByPosition(0, i).setFormula("Row" + i);
                oSheet.getCellByPosition(i, 0).setFormula("Col" + i);
            }

            for (int i = 1; i < 6; i++)
                for (int j = 1; j < 6; j++) {
                    oSheet.getCellByPosition(i, j).setValue(2.5 * j + i);
                }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't fill some cells", e);
        }

        CellRangeAddress sCellRangeAddress = new CellRangeAddress();
        sCellRangeAddress.Sheet = 0;
        sCellRangeAddress.StartColumn = 1;
        sCellRangeAddress.StartRow = 0;
        sCellRangeAddress.EndColumn = 5;
        sCellRangeAddress.EndRow = 5;

        TableFilterField[] filterFields = new TableFilterField[2];
        filterFields[0] = new TableFilterField();
        filterFields[0].Connection = com.sun.star.sheet.FilterConnection.AND;
        filterFields[0].Field = 1;
        filterFields[0].IsNumeric = true;
        filterFields[0].StringValue = "4";
        filterFields[0].Operator = com.sun.star.sheet.FilterOperator.GREATER;
        filterFields[1] = new TableFilterField();
        filterFields[1].Connection = com.sun.star.sheet.FilterConnection.AND;
        filterFields[1].Field = 1;
        filterFields[1].IsNumeric = true;
        filterFields[1].StringValue = "12";
        filterFields[1].Operator = com.sun.star.sheet.FilterOperator.LESS_EQUAL;

        XDataPilotTablesSupplier DPTS = (XDataPilotTablesSupplier)
            UnoRuntime.queryInterface(XDataPilotTablesSupplier.class, oSheet);
        log.println("Getting test object");
        XDataPilotTables DPT = DPTS.getDataPilotTables();
        XDataPilotDescriptor DPDsc = DPT.createDataPilotDescriptor();
        DPDsc.setSourceRange(sCellRangeAddress);

        XSheetFilterDescriptor SFD = DPDsc.getFilterDescriptor();
        SFD.setFilterFields(filterFields);

        XPropertySet fieldPropSet = null;
        try {
            Object oDataPilotField = DPDsc.getDataPilotFields().getByIndex(0);
            fieldPropSet = (XPropertySet)
                UnoRuntime.queryInterface(XPropertySet.class, oDataPilotField);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create a test environment", e);
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create a test environment", e);
        }

        try {
            fieldPropSet.setPropertyValue("Function",
                com.sun.star.sheet.GeneralFunction.SUM);
            fieldPropSet.setPropertyValue("Orientation",
                com.sun.star.sheet.DataPilotFieldOrientation.DATA);
        } catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create a test environment", e);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create a test environment", e);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create a test environment", e);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create a test environment", e);
        }

        if (DPT.hasByName("DataPilotTable")) {
            DPT.removeByName("DataPilotTable");
        }
        DPT.insertNewByName("DataPilotTable", sCellAddress, DPDsc);
        try {
            oObj = (XInterface) AnyConverter.toObject(
                new Type(XInterface.class),DPT.getByName(DPT.getElementNames()[0]));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create a test environment", e);
        } catch (com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create a test environment", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create a test environment", e);
        }

        log.println("Creating object - " +
                                    ((oObj == null) ? "FAILED" : "OK"));

        TestEnvironment tEnv = new TestEnvironment( oObj );

        // Other parameters required for interface tests
        tEnv.addObjRelation("OUTPUTRANGE", sCellAddress);
        tEnv.addObjRelation("CELLFORCHANGE", oChangeCell);
        tEnv.addObjRelation("CELLFORCHECK", oCheckCell);
        tEnv.addObjRelation("FIELDSAMOUNT", new Integer(5));

        return tEnv;
    }

}


