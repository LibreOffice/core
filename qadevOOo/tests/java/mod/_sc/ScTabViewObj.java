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

import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XNameContainer;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.form.XForm;
import com.sun.star.lang.WrappedTargetException;
import java.io.PrintWriter;
import java.util.Comparator;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.FormTools;
import util.SOfficeFactory;
import util.ValueComparer;

import com.sun.star.container.XIndexAccess;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XCellRangeAddressable;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XCell;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.sheet.SpreadsheetView</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::sheet::XViewSplitable</code></li>
 *  <li> <code>com::sun::star::sheet::XViewFreezable</code></li>
 *  <li> <code>com::sun::star::sheet::SpreadsheetViewSettings</code></li>
 *  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 *  <li> <code>com::sun::star::container::XIndexAccess</code></li>
 *  <li> <code>com::sun::star::container::XElementAccess</code></li>
 *  <li> <code>com::sun::star::sheet::XSpreadsheetView</code></li>
 *  <li> <code>com::sun::star::view::XSelectionSupplier</code></li>
 * </ul>
 * @see com.sun.star.sheet.XViewSplitable
 * @see com.sun.star.sheet.XViewFreezable
 * @see com.sun.star.sheet.SpreadsheetViewSettings
 * @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.container.XIndexAccess
 * @see com.sun.star.container.XElementAccess
 * @see com.sun.star.sheet.XSpreadsheetView
 * @see com.sun.star.view.XSelectionSupplier
 * @see ifc.sheet._XViewSplitable
 * @see ifc.sheet._XViewFreezable
 * @see ifc.sheet._SpreadsheetViewSettings
 * @see ifc.beans._XPropertySet
 * @see ifc.container._XIndexAccess
 * @see ifc.container._XElementAccess
 * @see ifc.sheet._XSpreadsheetView
 * @see ifc.view._XSelectionSupplier
 */
public class ScTabViewObj extends TestCase {
    public static XSpreadsheetDocument xSpreadsheetDoc;
    public static XSpreadsheetDocument xSpreadsheetDoc2;

    /**
     * Creates Spreadsheet document.
     */
    public void initialize( TestParameters Param, PrintWriter log ) {
        // get a soffice factory object

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());

        try {
            log.println("creating two spreadsheet documents");
            xSpreadsheetDoc = SOF.createCalcDoc(null);
            try {
                Thread.sleep(1000);
            } catch (java.lang.InterruptedException e) {}
            xSpreadsheetDoc2 = SOF.createCalcDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document ", e );
        }
    }

    /**
     * Disposes Spreadsheet document.
     */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = (XComponent)
        UnoRuntime.queryInterface(XComponent.class, xSpreadsheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
        XComponent oComp2 = (XComponent)
        UnoRuntime.queryInterface(XComponent.class, xSpreadsheetDoc2) ;
        util.DesktopTools.closeDoc(oComp2);
    }

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Retieves the current controller of the spreadsheet document using the
     * interface <code>XModel</code>. The controller is the instance of the
     * service <code>com.sun.star.sheet.SpreadsheetView</code>. Retrieves
     * a collection of spreadsheets from the document and takes one of  them.
     * Fills two cells in the spreadsheet.
     * Object relations created :
     * <ul>
     *  <li> <code>'Sheet'</code> for
     *      {@link ifc.sheet._XSpreadsheetView}(the retrieved spreadsheet)</li>
     * </ul>
     * @see com.sun.star.frame.XModel
     * @see com.sun.star.sheet.SpreadsheetView
     */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {
        XDrawPage oDrawPage = null;

        XModel aModel = (XModel)
        UnoRuntime.queryInterface(XModel.class, xSpreadsheetDoc);

        XModel aSecondModel = (XModel)
        UnoRuntime.queryInterface(XModel.class, xSpreadsheetDoc2);

        XInterface oObj = aModel.getCurrentController();

        log.println("getting sheets");
        XSpreadsheets xSpreadsheets = (XSpreadsheets)xSpreadsheetDoc.getSheets();

        log.println("getting a sheet");
        XSpreadsheet oSheet = null;
        XIndexAccess oIndexAccess = (XIndexAccess)
        UnoRuntime.queryInterface(XIndexAccess.class, xSpreadsheets);
        try {
            oSheet = (XSpreadsheet) AnyConverter.toObject(
                new Type(XSpreadsheet.class), oIndexAccess.getByIndex(1));
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

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("XDispatchProvider.URL", ".uno:Copy") ;

        log.println("adding 'Sheet' as ObjRelation");
        tEnv.addObjRelation("Sheet", oSheet);
        tEnv.addObjRelation("Frame",aModel.getCurrentController().getFrame());
        tEnv.addObjRelation("SecondModel",aSecondModel);
        tEnv.addObjRelation("FirstModel",aModel);

        //Relation for XControlAccess
        tEnv.addObjRelation("DOCUMENT", UnoRuntime.queryInterface(XComponent.class,xSpreadsheetDoc));
        tEnv.addObjRelation("XControlAccess.isSheet", Boolean.TRUE);
        //Relations for XSelectionSupplier
        XCell cell_1 = null;
        XCell cell_2 = null;
        Object cellRange = null;
        try {
            cellRange = (Object)oSheet.getCellRangeByPosition(0, 0, 3, 3);
            cell_1 = oSheet.getCellByPosition(5,5);
            cell_2 = oSheet.getCellByPosition(7,7);
            cell_2.setValue(17.5);
            cell_1.setValue(5.5);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get some cell", e);
        }

        Object[] selections = {oSheet, cellRange, cell_1, cell_2};
        tEnv.addObjRelation("Selections", selections);

        tEnv.addObjRelation("Comparer", new Comparator() {
            public int compare(Object o1, Object o2) {
                XCellRangeAddressable adr1 = (XCellRangeAddressable)
                UnoRuntime.queryInterface(XCellRangeAddressable.class, o1);
                XCellRangeAddressable adr2 = (XCellRangeAddressable)
                UnoRuntime.queryInterface(XCellRangeAddressable.class, o2);
                if (adr1 == null || adr2 == null) return -1;
                CellRangeAddress range1 = adr1.getRangeAddress();
                CellRangeAddress range2 = adr2.getRangeAddress();
                return ValueComparer.equalValue(range1, range2) ? 0 : 1 ;
            }
            public boolean equals(Object obj) {
                return compare(this, obj) == 0;
            } });

            tEnv.addObjRelation("XUserInputInterception.XModel", aModel);

            // XForm for com.sun.star.view.XFormLayerAccess

            XForm myForm = null;
            String kindOfControl="CommandButton";
            XShape aShape = null;
            try{
                log.println("adding contol shape '" + kindOfControl + "'");
                XComponent oComp = (XComponent) UnoRuntime.queryInterface(XComponent.class, xSpreadsheetDoc) ;

                aShape = FormTools.createControlShape(oComp, 3000, 4500, 15000, 10000, kindOfControl);

            } catch (Exception e){
                e.printStackTrace(log);
                throw new StatusException("Couldn't create following control shape : '" +
                    kindOfControl + "': ", e);
            }

            log.println("adding relation for com.sun.star.view.XFormLayerAccess: XForm");
            try {
                log.println( "getting Drawpages" );
                XDrawPagesSupplier oDPS = (XDrawPagesSupplier)
                UnoRuntime.queryInterface(XDrawPagesSupplier.class,xSpreadsheetDoc);
                XDrawPages oDP = (XDrawPages) oDPS.getDrawPages();
                oDP.insertNewByIndex(1);
                oDP.insertNewByIndex(2);
                oDrawPage = (XDrawPage) AnyConverter.toObject(
                    new Type(XDrawPage.class),oDP.getByIndex(0));
                if (oDrawPage == null)
                    log.println("ERROR: could not get DrawPage: null");

                oDrawPage.add(aShape);
                log.println("getting XForm");
                XNameContainer xForm = FormTools.getForms(oDrawPage);
                try {
                    myForm = (XForm) AnyConverter.toObject(new Type(XForm.class),xForm.getByName("Standard"));
                } catch (WrappedTargetException ex) {
                    log.println("ERROR: could not XFormm 'Standard': " + ex.toString());
                } catch (com.sun.star.lang.IllegalArgumentException ex) {
                    log.println("ERROR: could not XFormm 'Standard': " + ex.toString());
                } catch (NoSuchElementException ex) {
                    log.println("ERROR: could not XFormm 'Standard': " + ex.toString());
                }
                if (myForm == null)
                    log.println("ERROR: could not get XForm: null");
                tEnv.addObjRelation("XFormLayerAccess.XForm", myForm);
            } catch (com.sun.star.lang.IndexOutOfBoundsException ex) {
                log.println("ERROR: could not add ObjectRelation 'XFormLayerAccess.XForm': " + ex.toString());
            } catch (WrappedTargetException ex) {
                log.println("ERROR: could not add ObjectRelation 'XFormLayerAccess.XForm': " + ex.toString());
            } catch (com.sun.star.lang.IllegalArgumentException ex) {
                log.println("ERROR: could not add ObjectRelation 'XFormLayerAccess.XForm': " + ex.toString());
            }
            return tEnv;
    }

}    // finish class ScTabViewObj
