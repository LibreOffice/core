/*************************************************************************
 *
 *  $RCSfile: _XDataPilotTables.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:00:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
            log.println("Exception occured! " + e);
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
            log.println("Exception occured ! " + e);
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

