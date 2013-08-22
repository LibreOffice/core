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

import com.sun.star.container.XIndexAccess;
import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCell;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.view.XSelectionSupplier;
import ifc.view._XPrintJobBroadcaster;
import java.io.File;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.SOfficeFactory;
import util.utils;


/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.SpreadsheetDocument</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::sheet::XSpreadsheetDocument</code></li>
*  <li> <code>com::sun::star::sheet::SpreadsheetDocumentSettings</code></li>
*  <li> <code>com::sun::star::lang::XComponent</code></li>
*  <li> <code>com::sun::star::frame::XModel</code></li>
*  <li> <code>com::sun::star::sheet::SpreadsheetDocument</code></li>
*  <li> <code>com::sun::star::util::XNumberFormatsSupplier</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
* </ul>
* @see com.sun.star.sheet.SpreadsheetDocument
* @see com.sun.star.sheet.XSpreadsheetDocument
* @see com.sun.star.sheet.SpreadsheetDocumentSettings
* @see com.sun.star.lang.XComponent
* @see com.sun.star.frame.XModel
* @see com.sun.star.sheet.SpreadsheetDocument
* @see com.sun.star.util.XNumberFormatsSupplier
* @see com.sun.star.beans.XPropertySet
* @see ifc.sheet._XSpreadsheetDocument
* @see ifc.sheet._SpreadsheetDocumentSettings
* @see ifc.lang._XComponent
* @see ifc.frame._XModel
* @see ifc.sheet._SpreadsheetDocument
* @see ifc.util._XNumberFormatsSupplier
* @see ifc.beans._XPropertySet
*/
public class ScModelObj extends TestCase {
    private XSpreadsheetDocument xSpreadsheetDoc;
    private XSpreadsheetDocument xSecondsheetDoc;

    /**
    * Disposes Spreadsheet documents.
    */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xSheetDoc ");

        XComponent oComp = UnoRuntime.queryInterface(
                                   XComponent.class, xSpreadsheetDoc);
        util.DesktopTools.closeDoc(oComp);

        oComp = UnoRuntime.queryInterface(XComponent.class,
                                                       xSecondsheetDoc);
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Disposes the spreadsheet documents if they was created already.
    * Creates two spreadsheet documents. Rertieves the current controller for
    * each of them. Obtains the collection of spreadsheets, takes one of them
    * and takes some cell from the spreadsheet. The created documents are the instances
    * of the service <code>com.sun.star.sheet.SpreadsheetDocument</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'SELSUPP'</code> for
    *      {@link ifc.frame._XModel}( the interface
    *      <code>XSelectionSupplier</code> that was queried from the current
    *      controller of the spreadsheet)</li>
    *  <li> <code>'TOSELECT'</code> for
    *      {@link ifc.frame._XModel}( the cell that was retrieved from the
    *      spreadsheet)</li>
    *  <li> <code>'CONT2'</code> for
    *      {@link ifc.frame._XModel}( the current controller of the second
    *      spreadsheet)</li>
    * </ul>
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param,
                                                                 PrintWriter log) {
        // creation of the testobject here
        // first we write what we are intend to do to log file
        log.println("craeting a test environment");

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                     (XMultiServiceFactory) Param.getMSF());

        if (xSpreadsheetDoc != null) {
            XComponent oComp = UnoRuntime.queryInterface(
                                       XComponent.class, xSpreadsheetDoc);
            util.DesktopTools.closeDoc(oComp);
        }

        if (xSecondsheetDoc != null) {
            XComponent oComp = UnoRuntime.queryInterface(
                                       XComponent.class, xSecondsheetDoc);
            util.DesktopTools.closeDoc(oComp);
        }

        try {
            log.println("creating two spreadsheet documents");
            xSpreadsheetDoc = SOF.createCalcDoc(null);
            xSecondsheetDoc = SOF.createCalcDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document ", e);
        }

        XModel model1 = UnoRuntime.queryInterface(XModel.class,
                                                           xSpreadsheetDoc);
        XModel model2 = UnoRuntime.queryInterface(XModel.class,
                                                           xSecondsheetDoc);
        XInterface oObj = model1;

        TestEnvironment tEnv = new TestEnvironment(oObj);

        XController cont1 = model1.getCurrentController();
        XController cont2 = model2.getCurrentController();
        cont1.getFrame().setName("cont1");
        cont2.getFrame().setName("cont2");

        XSelectionSupplier sel = UnoRuntime.queryInterface(
                                         XSelectionSupplier.class, cont1);

        XCell toSel = null;
        XCell[] xCalculatableCells = null;
        try {
            log.println("Getting spreadsheet");

            XSpreadsheets oSheets = xSpreadsheetDoc.getSheets();
            XIndexAccess oIndexSheets = UnoRuntime.queryInterface(
                                                XIndexAccess.class, oSheets);
            XSpreadsheet oSheet = (XSpreadsheet) AnyConverter.toObject(
                                          new Type(XSpreadsheet.class),
                                          oIndexSheets.getByIndex(0));

            log.println("Getting a cell from sheet");
            toSel = oSheet.getCellByPosition(2, 3);
            // create a simple formula for XCalculatable
            oSheet.getCellByPosition(4, 5).setValue(15);
            oSheet.getCellByPosition(5, 5).setValue(10);
            oSheet.getCellByPosition(6, 5).setFormula("= E6 * F6");
            xCalculatableCells = new XCell[]{
                oSheet.getCellByPosition(4, 5),
                oSheet.getCellByPosition(5, 5),
                oSheet.getCellByPosition(6, 5)
            };

            // Make sure there are at least two sheets:
            oSheets.insertNewByName("Some Sheet", (short) 1);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException(
                    "Error getting cell object from spreadsheet document", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException(
                    "Error getting cell object from spreadsheet document", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException(
                    "Error getting cell object from spreadsheet document", e);
        }

        log.println("Adding SelectionSupplier and Shape to select for XModel");
        tEnv.addObjRelation("SELSUPP", sel);
        tEnv.addObjRelation("TOSELECT", toSel);

        log.println("Adding cells for XCalculatable");
        tEnv.addObjRelation("XCalculatable.Cells", xCalculatableCells);
        log.println("adding Controller as ObjRelation for XModel");
        tEnv.addObjRelation("CONT2", cont2);

        // create object relation for XPrintJobBroadcaster
        String fileName = utils.getOfficeTempDirSys((XMultiServiceFactory) Param.getMSF())+"printfile.prt" ;
        File f = new File(fileName);
        if (f.exists()) {
            f.delete();
        }
        _XPrintJobBroadcaster.MyPrintJobListener listener = new _XPrintJobBroadcaster.MyPrintJobListener(oObj, fileName);
        tEnv.addObjRelation("XPrintJobBroadcaster.XPrintJobListener", listener);

        return tEnv;
    }

} // finish class ScModelObj
