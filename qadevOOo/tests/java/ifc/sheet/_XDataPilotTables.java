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

