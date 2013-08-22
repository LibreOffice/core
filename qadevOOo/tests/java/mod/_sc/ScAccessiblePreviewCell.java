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

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.awt.XWindow;
import com.sun.star.container.XIndexAccess;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
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
import com.sun.star.util.URL;
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
public class ScAccessiblePreviewCell extends TestCase {
    private XSpreadsheetDocument xSheetDoc = null;

    /**
     * Creates a spreadsheet document.
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
     * Disposes a spreadsheet document.
     */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = UnoRuntime.queryInterface
            (XComponent.class, xSheetDoc);
        util.DesktopTools.closeDoc(oComp);
    }


    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Obtains the accessible object for a one of cell in preview mode.
     */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;
        XCell xCell = null;

        try {
            log.println("Getting spreadsheet") ;
            XSpreadsheets oSheets = xSheetDoc.getSheets() ;
            XIndexAccess oIndexSheets = UnoRuntime.queryInterface(XIndexAccess.class, oSheets);
            XSpreadsheet oSheet = (XSpreadsheet) AnyConverter.toObject(
                    new Type(XSpreadsheet.class),oIndexSheets.getByIndex(0));

            log.println("Getting a cell from sheet") ;
            xCell = oSheet.getCellByPosition(0, 0);
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

        xCell.setFormula("Value");

        XModel xModel = UnoRuntime.queryInterface(XModel.class, xSheetDoc);

        XController xController = xModel.getCurrentController();

        //switch to 'Print Preview' mode
        try {
            XDispatchProvider xDispProv = UnoRuntime.queryInterface(XDispatchProvider.class, xController);
            XURLTransformer xParser = UnoRuntime.queryInterface(XURLTransformer.class,
         ((XMultiServiceFactory)Param.getMSF()).createInstance("com.sun.star.util.URLTransformer"));
            URL[] aParseURL = new URL[1];
            aParseURL[0] = new URL();
            aParseURL[0].Complete = ".uno:PrintPreview";
            xParser.parseStrict(aParseURL);
            URL aURL = aParseURL[0];
            XDispatch xDispatcher = xDispProv.queryDispatch(aURL, "", 0);
            if(xDispatcher != null)
                xDispatcher.dispatch( aURL, null );
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't change mode");
            throw new StatusException(Status.failed("Couldn't change mode"));
        }

        shortWait();

        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = AccessibilityTools.getCurrentWindow((XMultiServiceFactory)Param.getMSF(), xModel);
        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        oObj = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.TABLE_CELL, true);

        log.println("ImplementationName " + utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer(){
                public void fireEvent() {
                    System.out.println("Fire Event");
                }
            });

        return tEnv;
    }

    protected void shortWait() {
        try {
            Thread.sleep(1000) ;
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e);
        }
    }
}
