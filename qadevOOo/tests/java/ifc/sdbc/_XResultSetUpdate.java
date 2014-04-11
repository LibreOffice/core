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

package ifc.sdbc;

import lib.MultiMethodTest;
import lib.StatusException;

import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XResultSetUpdate;
import com.sun.star.sdbc.XRowUpdate;
import com.sun.star.uno.UnoRuntime;

/**
/**
* Testing <code>com.sun.star.sdbc.XResultSetUpdate</code>
* interface methods :
* <ul>
*  <li><code> insertRow()</code></li>
*  <li><code> updateRow()</code></li>
*  <li><code> deleteRow()</code></li>
*  <li><code> cancelRowUpdates()</code></li>
*  <li><code> moveToInsertRow()</code></li>
*  <li><code> moveToCurrentRow()</code></li>
* </ul> <p>
* The test requires the following object relations :
* <ul>
*  <li><code>'XResultSetUpdate.UpdateTester'</code>
*   inner <code>UpdateTester</code> interface implementation :
*   is used for checking test results. See interface
*   documentation for more information.</li>
* </ul>
* The test is <b>not designed</b> for multithreaded testing. <p>
* After it's execution environment must be recreated.
* @see com.sun.star.sdbc.XResultSetUpdate
*/
public class _XResultSetUpdate extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XResultSetUpdate oObj = null ;

    private UpdateTester tester = null ;

    /**
    * Interface contains some methods for checking
    * test results. It's implementation must be passed
    * to this test.
    */
    public static interface UpdateTester {
        /**
        * @return Current number of rows.
        */
        public int rowCount() throws SQLException ;
        /**
        * Updates some data in the current row but doesn't commit
        * changes to the source.
        */
        public void update() throws SQLException ;
        /**
        * Checks if updates made by method <code>update</code> was
        * commited to the data source.
        */
        public boolean wasUpdated() throws SQLException ;
        /**
        * Returns current row number. Really it must returns value
        * < 1 if the current position is on insert row.
        */
        public int currentRow() throws SQLException ;
    }

    /**
    * Retrieves relation.
    * @throw StatusException If relation not found.
    */
    public void before() throws StatusException {
        tester = (UpdateTester)tEnv.getObjRelation
            ("XResultSetUpdate.UpdateTester") ;

        if (tester == null) {
            log.println("Required relation not found !!!") ;
            throw new StatusException("Required relation not found !!!",
                new NullPointerException()) ;
        }
    }

    /**
    * Calls method when the cursor position is on existing row.
    * Checks total number of rows before and after method call. <p>
    * Executes <code>moveToCurrentRow</code> method test before to
    * be sure that cursor is not on the insert row. <p>
    * Has OK status if after method execution number of rows decreased
    * by one.
    */
    public void _deleteRow() {
        executeMethod("moveToCurrentRow()") ;

        //temporary to avoid SOffice hanging
        executeMethod("updateRow()") ;
        executeMethod("cancelRowUpdates()") ;

        boolean result = true ;
        try {
            int rowsBefore = tester.rowCount() ;
            oObj.deleteRow() ;
            int rowsAfter = tester.rowCount() ;

            result = rowsBefore == rowsAfter + 1 ;
        } catch (SQLException e) {
            e.printStackTrace(log) ;
            result = false ;
        }

        tRes.tested("deleteRow()", result) ;
    }

    /**
    * Using relation methods first updates some data in the current
    * row, then calls <code>updateRow</code> method to commit data.
    * Then checks if the data changed was commited. <p>
    * Executes <code>moveToCurrentRow</code> method test before to
    * be sure that cursor is not on the insert row. <p>
    * Has OK status if data in the source was changed.
    */
    public void _updateRow() {
        executeMethod("moveToCurrentRow()") ;
        boolean result = true ;
        try {
            tester.update() ;
            oObj.updateRow() ;

            result = tester.wasUpdated() ;
        } catch (SQLException e) {
            e.printStackTrace(log) ;
            result = false ;
        }
        tRes.tested("updateRow()", result) ;
    }

    /**
    * Using relation methods first updates some data in the current
    * row, then calls <code>cancelRowUpdates</code> method.
    * Then checks if the data changed was not commited. <p>
    * Executes <code>moveToCurrentRow</code> method test before to
    * be sure that cursor is not on the insert row. <p>
    * Has OK status if data in the source was not changed.
    */
    public void _cancelRowUpdates() {
        executeMethod("moveToCurrentRow()") ;
        boolean result = true ;
        try {
            tester.update() ;
            oObj.cancelRowUpdates() ;

            result = !tester.wasUpdated() ;
        } catch (SQLException e) {
            e.printStackTrace(log) ;
            result = false ;
        }
        tRes.tested("cancelRowUpdates()", result) ;
    }

    /**
    * Tries to move cursor to insert row. Then checks current row
    * number. It must be less than 1. (0 as I know) <p>
    */
    public void _moveToInsertRow() {
        boolean result = true ;
        try {
            oObj.moveToInsertRow() ;

            result = tester.currentRow() < 1 ;
        } catch (SQLException e) {
            e.printStackTrace(log) ;
            result = false ;
        }
        tRes.tested("moveToInsertRow()", result) ;
    }

    /**
    * Returns cursor from insert row back to previous row. <p>
    * <code>moveToInsertRow</code> method test must be executed
    * first for positioning cursor to insert row. <p>
    * Has OK status if after method call current row number is
    * above 0.
    */
    public void _moveToCurrentRow() {
        boolean result = true ;
        try {
            oObj.moveToCurrentRow() ;

            result = tester.currentRow() >= 1 ;
        } catch (SQLException e) {
            e.printStackTrace(log) ;
            result = false ;
        }
        tRes.tested("moveToCurrentRow()", result) ;
    }

    /**
    * Moves cursor to the insert row, then calls the method
    * <code>insertRow</code>. Before and after call stores
    * total number of rows. <p>
    * Has OK status if after method call rows number increases
    * by one.
    */
    public void _insertRow() {
        executeMethod("moveToInsertRow()") ;
        boolean result = true ;
        try {
            oObj.moveToCurrentRow();
            int rowsBefore = tester.rowCount() ;
            oObj.moveToInsertRow() ;
            XRowUpdate rowU = UnoRuntime.queryInterface(XRowUpdate.class, oObj);
            rowU.updateString(1,"open");
            rowU.updateInt(2,5);
            rowU.updateDouble(5,3.4);
            rowU.updateBoolean(10,true);
            oObj.insertRow() ;
            oObj.moveToCurrentRow();
            int rowsAfter = tester.rowCount() ;
            result = rowsBefore + 1 == rowsAfter ;
        } catch (SQLException e) {
            e.printStackTrace(log) ;
            log.println("******"+e.getMessage());
            result = false ;
        }
        tRes.tested("insertRow()", result) ;
    }

    /**
    * Forces environment to be recreated.
    */
    public void after() {
        //disposeEnvironment() ;
    }
}  // finish class _XResultSetUpdate


