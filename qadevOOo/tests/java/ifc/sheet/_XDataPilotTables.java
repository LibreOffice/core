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

package ifc.sheet;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.sheet.XDataPilotDescriptor;
import com.sun.star.sheet.XDataPilotTables;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.table.CellAddress;

/**
* Testing <code>com.sun.star.sheet.XDataPilotTables</code>
* interface methods :
* <ul>
*  <li><code> createDataPilotDescriptor()</code></li>
*  <li><code> insertNewByName()</code></li>
*  <li><code> removeByName()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'SHEET'</code> (of type <code>XSpreadsheet</code>):
*   to have a spreadsheet document for document content checking</li>
* <ul> <p>
* @see com.sun.star.sheet.XDataPilotTables
*/
public class _XDataPilotTables extends MultiMethodTest {

    public XDataPilotTables oObj = null;
    XDataPilotDescriptor DPDscr = null;
    String name = "XDataPilotTables";
    CellAddress CA = new CellAddress((short)0, 9, 8);
    XSpreadsheet oSheet = null;

    /**
    * Retrieves object relations.
    * @throws StatusException If one of relations not found.
    */
    @Override
    protected void before() {
        oSheet = (XSpreadsheet)tEnv.getObjRelation("SHEET");
        if (oSheet == null) throw new StatusException(Status.failed
            ("Relation 'SHEET' not found"));
    }

    /**
    * Test calls the method, stores returned value and checks returned value.
    * <p>Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _createDataPilotDescriptor(){
        DPDscr = oObj.createDataPilotDescriptor();
        tRes.tested("createDataPilotDescriptor()", DPDscr != null);
    }

    /**
    * Test calls the method inserting new table with new name and then calls
    * the method inserting table with existent name. <p>
    * Has <b> OK </b> status if the cell content where table was inserted is
    * equal to 'Filter' after first call and exception was thrown during
    * second call. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> createDataPilotDescriptor() </code> : to have
    *  <code>XDataPilotDescriptor</code> created by this method</li>
    * </ul>
    */
    public void _insertNewByName(){
        requiredMethod("createDataPilotDescriptor()");
        boolean bResult = true;
        log.println("Inserting new Table \"" + name + "\"");
        try {
            oObj.insertNewByName(name, CA, DPDscr);
            bResult &= oSheet.getCellByPosition
                (CA.Column, CA.Row).getFormula().equals("Filter");
        } catch (com.sun.star.uno.Exception e) {
            log.println("Exception occurred! " + e);
            bResult = false;
        }

        log.println(bResult ? "OK" : "FAILED");
        log.println("Trying to insert element with existent name");

        try {
            oObj.insertNewByName(name,new CellAddress((short)0, 7, 7), DPDscr);
            log.println("No exception! - FAILED");
            bResult = false;
        } catch (com.sun.star.uno.RuntimeException e) {
            log.println("Expected exception - OK " + e);
        }

        log.println("Inserting new table " + (bResult ? "OK" : "FAILED"));
        tRes.tested("insertNewByName()", bResult);
    }

    /**
    * Test calls the method for existent table and for unexistent table. <p>
    * Has <b> OK </b> status if the cell where table was removed from is empty
    * after first call and exception was thrown during second call. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code>insertNewByName()</code>: to have name of existent table</li>
    * </ul>
    */
    public void _removeByName(){
        requiredMethod("insertNewByName()");
        boolean bResult = true;
        log.println("Remove table with name " + name);
        try {
            oObj.removeByName(name);
            bResult &= oSheet.getCellByPosition
                (CA.Column, CA.Row).getFormula().equals("");
        } catch (com.sun.star.uno.Exception e) {
            log.println("Exception occurred ! " + e);
            bResult = false;
        }
        log.println(bResult ? "OK" : "FAILED");
        log.println("Removing unexistent element");
        try {
            oObj.removeByName(name);
            log.println("No exception! - FAILED");
            bResult = false;
        } catch (com.sun.star.uno.RuntimeException e) {
            log.println("Expected exception - OK " + e);
        }

        log.println("Removing a table " + (bResult ? "OK" : "FAILED"));
        tRes.tested("removeByName()", bResult);
    }

}

