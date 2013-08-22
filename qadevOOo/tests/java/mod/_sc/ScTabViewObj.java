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
    private XSpreadsheetDocument xSpreadsheetDoc;
    private XSpreadsheetDocument xSpreadsheetDoc2;

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
        XComponent oComp = UnoRuntime.queryInterface(XComponent.class, xSpreadsheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
        XComponent oComp2 = UnoRuntime.queryInterface(XComponent.class, xSpreadsheetDoc2) ;
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

        XModel aModel = UnoRuntime.queryInterface(XModel.class, xSpreadsheetDoc);

        XModel aSecondModel = UnoRuntime.queryInterface(XModel.class, xSpreadsheetDoc2);

        XInterface oObj = aModel.getCurrentController();

        log.println("getting sheets");
        XSpreadsheets xSpreadsheets = xSpreadsheetDoc.getSheets();

        log.println("getting a sheet");
        XSpreadsheet oSheet = null;
        XIndexAccess oIndexAccess = UnoRuntime.queryInterface(XIndexAccess.class, xSpreadsheets);
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
            cellRange = oSheet.getCellRangeByPosition(0, 0, 3, 3);
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

        tEnv.addObjRelation("Comparer", new Comparator<Object>() {
            public int compare(Object o1, Object o2) {
                XCellRangeAddressable adr1 = UnoRuntime.queryInterface(XCellRangeAddressable.class, o1);
                XCellRangeAddressable adr2 = UnoRuntime.queryInterface(XCellRangeAddressable.class, o2);
                if (adr1 == null || adr2 == null) return -1;
                CellRangeAddress range1 = adr1.getRangeAddress();
                CellRangeAddress range2 = adr2.getRangeAddress();
                return ValueComparer.equalValue(range1, range2) ? 0 : 1 ;
            }});

            tEnv.addObjRelation("XUserInputInterception.XModel", aModel);

            // XForm for com.sun.star.view.XFormLayerAccess

            XForm myForm = null;
            String kindOfControl="CommandButton";
            XShape aShape = null;
            try{
                log.println("adding contol shape '" + kindOfControl + "'");
                XComponent oComp = UnoRuntime.queryInterface(XComponent.class, xSpreadsheetDoc) ;

                aShape = FormTools.createControlShape(oComp, 3000, 4500, 15000, 10000, kindOfControl);

            } catch (Exception e){
                e.printStackTrace(log);
                throw new StatusException("Couldn't create following control shape : '" +
                    kindOfControl + "': ", e);
            }

            log.println("adding relation for com.sun.star.view.XFormLayerAccess: XForm");
            try {
                log.println( "getting Drawpages" );
                XDrawPagesSupplier oDPS = UnoRuntime.queryInterface(XDrawPagesSupplier.class,xSpreadsheetDoc);
                XDrawPages oDP = oDPS.getDrawPages();
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
