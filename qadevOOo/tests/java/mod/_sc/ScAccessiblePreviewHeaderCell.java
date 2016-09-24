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

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.lang.DisposedException;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.table.XCell;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;
import com.sun.star.util.XCloseable;
import com.sun.star.util.XURLTransformer;


/**
 * Object implements the following interfaces:
 * <ul>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleComponent</code>
 *  </li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleContext</code>
 *  </li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleSelection
 *  </code></li>
 *  <li><code>::com::sun::star::accessibility::XAccessibleValue</code>
 *  </li>
 *  <li><code>::com::sun::star::accessibility::XAccessibleEventBroadcaster
 *  </code></li>
 * </ul>
 * @see com.sun.star.accessibility.XAccessibleComponent
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see com.sun.star.accessibility.XAccessibleSelection
 * @see com.sun.star.accessibility.XAccessibleValue
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleComponent
 * @see ifc.accessibility._XAccessibleContext
 * @see ifc.accessibility._XAccessibleSelection
 * @see ifc.accessibility._XAccessibleTable
 */
public class ScAccessiblePreviewHeaderCell extends TestCase {
    private XSpreadsheetDocument xSheetDoc = null;

    /**
     * Creates a spreadsheet document.
     */
    @Override
    protected void initialize(TestParameters tParam, PrintWriter log) throws Exception {
    }

    /**
     * Disposes a spreadsheet document.
     */
    @Override
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xSheetDoc ");

        if (xSheetDoc != null) {
            try {
                XCloseable oComp = UnoRuntime.queryInterface(
                                           XCloseable.class, xSheetDoc);
                oComp.close(true);
                xSheetDoc = null;
            } catch (com.sun.star.util.CloseVetoException e) {
                e.printStackTrace(log);
            } catch (com.sun.star.lang.DisposedException e) {
                log.println("document already disposed");
                xSheetDoc = null;
            }
        }
    }

    /**
     * Creating a TestEnvironment for the interfaces to be tested.
     * Sets a value of the cell 'A1'. Sets the property 'PrintHeaders'
     * of the style 'Default' of the family 'PageStyles' to true.
     * Switchs the document to preview mode and then obtains the
     * accessible object for the header cell.
     */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param,
                                                                 PrintWriter log) throws Exception {
        if (xSheetDoc != null) {
            XComponent oComp = UnoRuntime.queryInterface(
                                       XComponent.class, xSheetDoc);
            util.DesktopTools.closeDoc(oComp);
        }

        SOfficeFactory SOF = SOfficeFactory.getFactory( Param.getMSF());

        log.println("creating a Spreadsheet document");
        xSheetDoc = SOF.createCalcDoc(null);

        XModel xModel = UnoRuntime.queryInterface(XModel.class,
                                                           xSheetDoc);

        XController xController = xModel.getCurrentController();

        //setting value of cell A1
        XCell xCell = null;

        log.println("Getting spreadsheet");

        XSpreadsheets oSheets = xSheetDoc.getSheets();
        XIndexAccess oIndexSheets = UnoRuntime.queryInterface(
                                            XIndexAccess.class, oSheets);
        XSpreadsheet oSheet = (XSpreadsheet) AnyConverter.toObject(
                                      new Type(XSpreadsheet.class),
                                      oIndexSheets.getByIndex(0));

        log.println("Getting a cell from sheet");
        xCell = oSheet.getCellByPosition(0, 0);

        xCell.setFormula("Value");

        //setting property 'PrintHeaders' of the style 'Default'
        XStyleFamiliesSupplier xSFS = UnoRuntime.queryInterface(
                                              XStyleFamiliesSupplier.class,
                                              xSheetDoc);
        XNameAccess xNA = xSFS.getStyleFamilies();
        XPropertySet xPropSet = null;

        Object oPageStyles = xNA.getByName("PageStyles");
        xNA = UnoRuntime.queryInterface(XNameAccess.class,
                                                      oPageStyles);

        Object oDefStyle = xNA.getByName("Default");
        xPropSet = UnoRuntime.queryInterface(
                           XPropertySet.class, oDefStyle);

        xPropSet.setPropertyValue("PrintHeaders", Boolean.TRUE);

        //switching to 'Print Preview' mode
        XDispatchProvider xDispProv = UnoRuntime.queryInterface(
                                              XDispatchProvider.class,
                                              xController);
        XURLTransformer xParser = UnoRuntime.queryInterface(
                                          XURLTransformer.class,
                                          Param.getMSF()
                                               .createInstance("com.sun.star.util.URLTransformer"));
        URL[] aParseURL = new URL[1];
        aParseURL[0] = new URL();
        aParseURL[0].Complete = ".uno:PrintPreview";
        xParser.parseStrict(aParseURL);

        URL aURL = aParseURL[0];
        XDispatch xDispatcher = xDispProv.queryDispatch(aURL, "", 0);

        if (xDispatcher != null) {
            xDispatcher.dispatch(aURL, null);
        }

        XInterface oObj = null;
        for (int i = 0;; ++i) {
            Thread.sleep(500);
            try {
                XAccessible xRoot = AccessibilityTools.getAccessibleObject(
                    AccessibilityTools.getCurrentWindow(
                        xModel));
                if (xRoot != null) {
                    oObj = AccessibilityTools.getAccessibleObjectForRole(
                        xRoot, AccessibleRole.TABLE, "A");
                    if (oObj == null) {
                        log.println("Version with a fixed #103863#?");
                        oObj = AccessibilityTools.getAccessibleObjectForRole(
                            xRoot, AccessibleRole.TABLE_CELL, true);
                    }
                    if (oObj != null) {
                        break;
                    }
                }
            } catch (DisposedException e) {
                log.println("Ignoring DisposedException");
            }
            if (i == 20) { // give up after 10 sec
                throw new RuntimeException(
                    "Couldn't get AccessibleRole.TABLE/TABLE_CELL object");
            }
            log.println("No TABLE/TABLE_CELL found yet, retrying");
        }

        log.println("ImplementationName " + utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("EventProducer",
                            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
            public void fireEvent() {
            }
        });

        return tEnv;
    }
}
