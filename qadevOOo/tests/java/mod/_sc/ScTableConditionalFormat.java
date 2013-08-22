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

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.ConditionOperator;
import com.sun.star.sheet.XSheetConditionalEntries;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.CellAddress;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.TableConditionalFormat</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNameAccess</code></li>
*  <li> <code>com::sun::star::container::XIndexAccess</code></li>
*  <li> <code>com::sun::star::sheet::XSheetConditionalEntries</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
* </ul>
* @see com.sun.star.sheet.TableConditionalFormat
* @see com.sun.star.container.XNameAccess
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.sheet.XSheetConditionalEntries
* @see com.sun.star.container.XElementAccess
* @see ifc.container._XNameAccess
* @see ifc.container._XIndexAccess
* @see ifc.sheet._XSheetConditionalEntries
* @see ifc.container._XElementAccess
*/
public class ScTableConditionalFormat extends TestCase {

    private XSpreadsheetDocument xSpreadsheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());

        try {
            log.println( "creating a sheetdocument" );
            xSpreadsheetDoc = SOF.createCalcDoc(null);
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
        XComponent oComp = UnoRuntime.queryInterface (XComponent.class, xSpreadsheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves a collection of spreadsheets from the document and takes one of
    * them. Fills some cells of the spreadsheet. Retrieves value of the property
    * <code>'ConditionalFormat'</code> that is the collection of the conditions
    * of a conditional format. Adds new two conditional entries to the
    * collection using the interface <code>XSheetConditionalEntries</code>.
    * The retrieved collection is the instance of the service
    * <code>com.sun.star.sheet.TableConditionalFormat</code>.
    * @see com.sun.star.sheet.TableConditionalFormat
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        log.println("getting sheets");
        XSpreadsheets xSpreadsheets = xSpreadsheetDoc.getSheets();

        log.println("getting a sheet");
        XSpreadsheet oSheet = null;
        XIndexAccess oIndexAccess = UnoRuntime.queryInterface(XIndexAccess.class, xSpreadsheets);
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
            log.println("Exception occurred while filling cells");
            e.printStackTrace(log);
        }

        XPropertySet Props = UnoRuntime.queryInterface(XPropertySet.class, oSheet);
        try {
            oObj = (XInterface) AnyConverter.toObject(
                    new Type(XInterface.class),
                        Props.getPropertyValue("ConditionalFormat"));
        } catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get ConditionalFromat", e);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get ConditionalFromat", e);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get ConditionalFromat", e);
        }

        XSheetConditionalEntries xSCE = UnoRuntime.queryInterface(XSheetConditionalEntries.class, oObj);
        xSCE.addNew(Conditions(5));
        xSCE.addNew(Conditions(2));

        log.println("creating a new environment for object");
        TestEnvironment tEnv = new TestEnvironment(oObj);

        return tEnv;
    } // finish method getTestEnvironment

    /**
    * Returns the array of the property values that was filled by condition
    * values.
    */
    public PropertyValue[] Conditions(int nr) {
        PropertyValue[] con = new PropertyValue[5];
        CellAddress ca = new CellAddress();
        ca.Column = 1;
        ca.Row = 5;
        ca.Sheet = 0;
        con[0] = new PropertyValue();
        con[0].Name = "StyleName";
        con[0].Value = "Result2";
        con[1] = new PropertyValue();
        con[1].Name = "Formula1";
        con[1].Value = "$Sheet1.$B$"+nr;
        con[2] = new PropertyValue();
        con[2].Name = "Formula2";
        con[2].Value = "";
        con[3] = new PropertyValue();
        con[3].Name = "Operator";
        con[3].Value = ConditionOperator.EQUAL;
        con[4] = new PropertyValue();
        con[4].Name = "SourcePosition";
        con[4].Value = ca;
        return con;
    }

}    // finish class ScTableConditionalFormat

