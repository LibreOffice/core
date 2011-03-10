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

package complex.dataPilot;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNamed;
import com.sun.star.lang.XMultiServiceFactory;
// import com.sun.star.sheet.TableFilterField;
import com.sun.star.sheet.XDataPilotDescriptor;
import com.sun.star.sheet.XDataPilotTable;
import com.sun.star.sheet.XDataPilotTables;
import com.sun.star.sheet.XDataPilotTablesSupplier;
// import com.sun.star.sheet.XSheetFilterDescriptor;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.CellAddress;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;
import complex.dataPilot._XPropertySet;
import complex.dataPilot._XNamed;
import complex.dataPilot._XDataPilotDescriptor;
import complex.dataPilot._XDataPilotTable;
// import complexlib.ComplexTestCase;
import lib.StatusException;
import lib.TestParameters;
import util.SOfficeFactory;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;


/**
 * check the DataPilot of Calc.
 */
public class CheckDataPilot {
    /** The data pilot field object **/
    private XInterface mDataPilotFieldObject = null;
    /** The data pilot table object **/
    private XInterface mDataPilotTableObject = null;


    private XSpreadsheetDocument xSheetDoc = null;

    /**
     * A field is filled some values. This integer determines the size of the
     * field in x and y direction.
     */
    private int mMaxFieldIndex = 6;

    /**
     * The test parameters
     */
    private static TestParameters param = null;

    /**
     * Get all test methods
     * @return The test methods
     */
    public String[] getTestMethodNames() {
        return new String[]{"testDataPilotTableObject",
                            "testDataPilotFieldObject"};
    }

    /**
     * Test the data pilot field object:
     * simply execute the interface tests in a row
     */
    @Test public void testDataPilotFieldObject() {
        System.out.println("Starting 'testDataPilotFieldObject'");
        // _XNamed
        XNamed xNamed = UnoRuntime.queryInterface(
                                    XNamed.class, mDataPilotFieldObject);

        _XNamed _xNamed = new _XNamed(xNamed/*, log*/, param);
        assertTrue("_getName failed.",_xNamed._getName());
        assertTrue("_setName failed.",_xNamed._setName());

        // _XPropertySet
        XPropertySet xProp = UnoRuntime.queryInterface(
                                    XPropertySet.class, mDataPilotFieldObject);
        _XPropertySet _xProp = new _XPropertySet(xProp/*, log*/, param);
        assertTrue("_getPropertySetInfo failed.",_xProp._getPropertySetInfo());
        assertTrue("_addPropertyChangeListener failed.",_xProp._addPropertyChangeListener());
        assertTrue("_addVetoableChangeListener failed.",_xProp._addVetoableChangeListener());
        assertTrue("_setPropertyValue failed.",_xProp._setPropertyValue());
        assertTrue("_getPropertyValue failed.",_xProp._getPropertyValue());
        assertTrue("_removePropertyChangeListener failed.",_xProp._removePropertyChangeListener());
        assertTrue("_removeVetoableChangeListener failed.",_xProp._removeVetoableChangeListener());

    }

    /**
     * Test the data pilot table object:
     * simply execute the interface tests in a row
     */
    @Test public void testDataPilotTableObject() {
        System.out.println("Starting 'testDataPilotTableObject'");
        // _XNamed
        XNamed xNamed = UnoRuntime.queryInterface(
                                    XNamed.class, mDataPilotTableObject);
        _XNamed _xNamed = new _XNamed(xNamed/*, log*/, param);
        assertTrue("_getName failed.",_xNamed._getName());
        assertTrue("_setName failed.",_xNamed._setName());

        // _XDataPilotTable
        XDataPilotTable xDataPilotTable =
                    UnoRuntime.queryInterface(XDataPilotTable.class,
                    mDataPilotTableObject);
        _XDataPilotTable _xDataPilotTable =
                    new _XDataPilotTable(xDataPilotTable/*, log*/, param);
        assertTrue("before failed.", _xDataPilotTable.before());
        assertTrue("_getOutputRange failed.", _xDataPilotTable._getOutputRange()) ;
//        assertTrue("_refresh failed.", _xDataPilotTable._refresh()) ;

        // _XDataPilotDescriptor
        XDataPilotDescriptor xDataPilotDescriptor =
                    UnoRuntime.queryInterface(XDataPilotDescriptor.class,
                    mDataPilotTableObject);
        _XDataPilotDescriptor _xDataPilotDescriptor =
                    new _XDataPilotDescriptor(xDataPilotDescriptor/*, log*/, param);
        assertTrue("before failed.", _xDataPilotDescriptor.before());
        assertTrue("_setTag failed.", _xDataPilotDescriptor._setTag()) ;
        assertTrue("_getTag failed.", _xDataPilotDescriptor._getTag()) ;
        assertTrue("_getFilterDescriptor failed.", _xDataPilotDescriptor._getFilterDescriptor()) ;
        assertTrue("_getDataPilotFields failed.", _xDataPilotDescriptor._getDataPilotFields()) ;
        assertTrue("_getColumnFields failed.", _xDataPilotDescriptor._getColumnFields()) ;
        assertTrue("_getRowFields failed.", _xDataPilotDescriptor._getRowFields()) ;
        assertTrue("_getDataFields failed.", _xDataPilotDescriptor._getDataFields()) ;
        assertTrue("_getHiddenFields failed.", _xDataPilotDescriptor._getHiddenFields()) ;
        assertTrue("_getPageFields failed.", _xDataPilotDescriptor._getPageFields()) ;
        assertTrue("_setSourceRange failed.", _xDataPilotDescriptor._setSourceRange()) ;
        assertTrue("_getSourceRange failed.", _xDataPilotDescriptor._getSourceRange()) ;
    }

    /**
     * create an environment for the test
     */
    @Before public void before() {
//        Object oInterface = null;

        // SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)param.getMSF() );
        final XMultiServiceFactory xMsf = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        SOfficeFactory SOF = SOfficeFactory.getFactory(xMsf);

        param = new TestParameters();
        param.put("ServiceFactory", xMsf);

        // the cell range
        CellRangeAddress sCellRangeAdress = new CellRangeAddress();
        sCellRangeAdress.Sheet = 0;
        sCellRangeAdress.StartColumn = 1;
        sCellRangeAdress.StartRow = 0;
        sCellRangeAdress.EndColumn = mMaxFieldIndex-1;
        sCellRangeAdress.EndRow = mMaxFieldIndex - 1;

        // position of the data pilot table
        CellAddress sCellAdress = new CellAddress();
        sCellAdress.Sheet = 0;
        sCellAdress.Column = 7;
        sCellAdress.Row = 8;

        try {
            System.out.println( "Creating a Spreadsheet document" );
            xSheetDoc = SOF.createCalcDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace();
            throw new StatusException( "Couldn't create document", e );
        }

        System.out.println("Getting a sheet");
        XSpreadsheets xSpreadsheets = xSheetDoc.getSheets();
        XSpreadsheet oSheet = null;
        XSpreadsheet oSheet2 = null;
        XIndexAccess oIndexAccess =
            UnoRuntime.queryInterface(XIndexAccess.class, xSpreadsheets);

        try {
            oSheet = (XSpreadsheet) AnyConverter.toObject(
                    new Type(XSpreadsheet.class),oIndexAccess.getByIndex(0));
            oSheet2 = (XSpreadsheet) AnyConverter.toObject(
                    new Type(XSpreadsheet.class),oIndexAccess.getByIndex(1));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace();
            throw new StatusException( "Couldn't get a spreadsheet", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace();
            throw new StatusException( "Couldn't get a spreadsheet", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace();
            throw new StatusException( "Couldn't get a spreadsheet", e);
        }

        try {
            System.out.println("Filling a table");
            for (int i = 1; i < mMaxFieldIndex; i++) {
                oSheet.getCellByPosition(i, 0).setFormula("Col" + i);
                oSheet.getCellByPosition(0, i).setFormula("Row" + i);
                oSheet2.getCellByPosition(i, 0).setFormula("Col" + i);
                oSheet2.getCellByPosition(0, i).setFormula("Row" + i);
            }

            for (int i = 1; i < mMaxFieldIndex; i++)
            {
                for (int j = 1; j < mMaxFieldIndex; j++)
                {
                    oSheet.getCellByPosition(i, j).setValue(i * (j + 1));
                    oSheet2.getCellByPosition(i, j).setValue(i * (j + 2));
                }
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't fill some cells", e);
        }

        // change a value of a cell and check the change in the data pilot
        // (for the XDataPilotTable.refresh() test)
        Object oChangeCell = null;
        Object oCheckCell = null;
        Integer aChangeValue = null;
        try {
            // cell of data
            oChangeCell = oSheet.getCellByPosition(1, 5);
            int x = sCellAdress.Column;
            int y = sCellAdress.Row + 3;
            // cell of the data pilot output
            oCheckCell = oSheet.getCellByPosition(x, y);
            aChangeValue = new Integer(27);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace();
            throw new StatusException( "Couldn't get cells for changeing.", e);
        }


        // create the test objects
        System.out.println("Getting test objects") ;
        XDataPilotTablesSupplier DPTS =
            UnoRuntime.queryInterface(XDataPilotTablesSupplier.class, oSheet);
        XDataPilotTables DPT = DPTS.getDataPilotTables();
        XDataPilotDescriptor DPDsc = DPT.createDataPilotDescriptor();
        DPDsc.setSourceRange(sCellRangeAdress);

        XPropertySet fieldPropSet = null;
        try {
            Object oDataPilotField = DPDsc.getDataPilotFields().getByIndex(0);
            fieldPropSet =
                UnoRuntime.queryInterface(XPropertySet.class, oDataPilotField);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't create a test environment", e);
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't create a test environment", e);
        }

        try {
            fieldPropSet.setPropertyValue("Function",
                com.sun.star.sheet.GeneralFunction.SUM);
            fieldPropSet.setPropertyValue("Orientation",
                com.sun.star.sheet.DataPilotFieldOrientation.DATA);
        } catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't create a test environment", e);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't create a test environment", e);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't create a test environment", e);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't create a test environment", e);
        }

        System.out.println("Insert the DataPilotTable");
        if (DPT.hasByName("DataPilotTable")) {
            DPT.removeByName("DataPilotTable");
        }
        DPT.insertNewByName("DataPilotTable", sCellAdress, DPDsc);
        try {
            mDataPilotTableObject = (XInterface) AnyConverter.toObject(
                new Type(XInterface.class),DPT.getByName(DPT.getElementNames()[0]));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't create a test environment", e);
        } catch (com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't create a test environment", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't create a test environment", e);
        }

        XIndexAccess IA = DPDsc.getDataPilotFields();
        try {
            mDataPilotFieldObject = (XInterface)AnyConverter.toObject(
                                new Type(XInterface.class),IA.getByIndex(0));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't get data pilot field", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't get data pilot field", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't get data pilot field", e);
        }

        // Other parameters required for interface tests
        param.put("OUTPUTRANGE", sCellAdress);
        param.put("CELLFORCHANGE", oChangeCell);
        param.put("CELLFORCHECK", oCheckCell);
        param.put("CHANGEVALUE", aChangeValue);
        param.put("FIELDSAMOUNT", new Integer(5));

    }

        /*
     * this method closes a calc document and resets the corresponding class variable xSheetDoc
     */
    protected boolean closeSpreadsheetDocument() {
        boolean worked = true;

        System.out.println("    disposing xSheetDoc ");

        try {
            XCloseable oCloser =  UnoRuntime.queryInterface(
                                         XCloseable.class, xSheetDoc);
            oCloser.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            worked = false;
            System.out.println("Couldn't close document");
        } catch (com.sun.star.lang.DisposedException e) {
            worked = false;
            System.out.println("Document already disposed");
        } catch (java.lang.NullPointerException e) {
            worked = false;
            System.out.println("Couldn't get XCloseable");
        }

        xSheetDoc = null;

        return worked;
    }

    @After public void after()
        {
            closeSpreadsheetDocument();
        }


    @BeforeClass public static void setUpConnection() throws Exception {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();


}
