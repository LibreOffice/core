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
import com.sun.star.awt.XWindow;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCell;
import com.sun.star.table.XColumnRowRange;
import com.sun.star.table.XTableColumns;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by accessible component of
 * a cell in the spreadsheet. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleSelection</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleEventBroadcaster</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleComponent</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleTable</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleContext</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleSelection
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see com.sun.star.accessibility.XAccessibleComponent
 * @see com.sun.star.accessibility.XAccessibleTable
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see ifc.accessibility._XAccessibleSelection
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleComponent
 * @see ifc.accessibility._XAccessibleTable
 * @see ifc.accessibility._XAccessibleContext
 */
public class ScAccessibleCell extends TestCase {

    private XSpreadsheetDocument xSpreadsheetDoc = null;

    /**
    * Called to create an instance of <code>TestEnvironment</code>
    * with an object to test and related objects.
    * Switches the document to Print Preview mode.
    * Obtains accessible object for the page view.
    *
    * @param Param test parameters
    * @param log writer to log information while testing
    *
    * @see TestEnvironment
    * @see #getTestEnvironment
    */
    @Override
    protected TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) throws Exception {

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(  Param.getMSF());

        log.println("creating a spreadsheetdocument");
        xSpreadsheetDoc = SOF.createCalcDoc(null);

        XInterface oObj = null;

        XModel aModel = UnoRuntime.queryInterface(XModel.class, xSpreadsheetDoc);

        XWindow xWindow = AccessibilityTools.getCurrentWindow(aModel);
        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        oObj = AccessibilityTools.getAccessibleObjectForRole
            (xRoot, AccessibleRole.TABLE_CELL, "B1");

        log.println("ImplementationName " + utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        // relation for XAccessibleEventBroadcaster
        XCell xCell = null;
        final String text = "XAccessibleText";
        try {
            XSpreadsheets oSheets = xSpreadsheetDoc.getSheets() ;
            XIndexAccess oIndexSheets = UnoRuntime.queryInterface(XIndexAccess.class, oSheets);
            XSpreadsheet oSheet = (XSpreadsheet) AnyConverter.toObject(
                    new Type(XSpreadsheet.class),oIndexSheets.getByIndex(0));
            xCell = oSheet.getCellByPosition(1, 0) ;
            xCell.setFormula(text);
            XColumnRowRange oColumnRowRange = UnoRuntime.queryInterface(XColumnRowRange.class, oSheet);
            XTableColumns oColumns = oColumnRowRange.getColumns();
            XIndexAccess oIndexAccess = UnoRuntime.queryInterface(XIndexAccess.class, oColumns);
            XPropertySet column = UnoRuntime.queryInterface(
                                XPropertySet.class,oIndexAccess.getByIndex(1));
            column.setPropertyValue("OptimalWidth", Boolean.TRUE);
        } catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception creating relation :");
            e.printStackTrace(log);
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Exception creating relation :");
            e.printStackTrace(log);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("Exception creating relation :");
            e.printStackTrace(log);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            log.println("Exception creating relation :");
            e.printStackTrace(log);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception creating relation :");
            e.printStackTrace(log);
        }

        tEnv.addObjRelation("EditOnly",
                    "This method is only supported if the Cell is in edit mode");

        final XCell fCell = xCell ;

        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer(){
                public void fireEvent() {
                    fCell.setFormula("firing event");
                    fCell.setFormula(text);
                }
            });

        tEnv.addObjRelation("XAccessibleText.Text", text);

        return tEnv;

    }

    /**
    * Called while disposing a <code>TestEnvironment</code>.
    * Disposes calc document.
    * @param Param test parameters
    * @param log writer to log information while testing
    */
    @Override
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = UnoRuntime.queryInterface (XComponent.class, xSpreadsheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }
}
