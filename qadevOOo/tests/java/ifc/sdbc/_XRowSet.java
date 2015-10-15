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

import ifc.sdb._XRowSetApproveBroadcaster;
import lib.MultiMethodTest;
import lib.StatusException;

import com.sun.star.lang.EventObject;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XRowSet;
import com.sun.star.sdbc.XRowSetListener;

/**
* Testing <code>com.sun.star.sdbc.XRowSet</code>
* interface methods :
* <ul>
*  <li><code> execute()</code></li>
*  <li><code> addRowSetListener()</code></li>
*  <li><code> removeRowSetListener()</code></li>
* </ul> <p>
* Required object relations :
* <ul>
* <li> <code>'XRowSetApproveBroadcaster.ApproveChecker'</code>:
*      implementation of inner interface
*      <code>ifs.sdb._XRowSetApproveBroadcaster.RowSetApproveChecker</code>
*      which can move cursor within a rowset, change row, and change the
*      whole rowset. </li>
* </ul> <p>
* It is better to recreate the object after test, because of unknown
* actions made by <code>RowSetApproveChecker</code> interface implementation.
* @see com.sun.star.sdbc.XRowSet
* @see ifc.sdb._XRowSetApproveBroadcaster
*/
public class _XRowSet extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XRowSet oObj = null ;
    private _XRowSetApproveBroadcaster.RowSetApproveChecker checker = null ;
    private final TestListener listener = new TestListener() ;

    private static class TestListener implements XRowSetListener {
        public boolean cursorMoved = false ;
        public boolean rowChanged = false ;
        public boolean rowSetChanged = false ;

        public void reset() {
            cursorMoved = false ;
            rowChanged = false ;
            rowSetChanged = false ;
        }
        public void cursorMoved(EventObject ev) {
            cursorMoved = true ;
        }
        public void rowChanged(EventObject ev) {
            rowChanged = true ;
        }
        public void rowSetChanged(EventObject ev) {
            rowSetChanged = true ;
        }
        public void disposing(EventObject ev) {}
    }

    /**
    * Retrieves relation.
    * @throw StatusException If relation not found.
    */
    @Override
    public void before() throws StatusException {
        checker = (_XRowSetApproveBroadcaster.RowSetApproveChecker)
            tEnv.getObjRelation("XRowSetApproveBroadcaster.ApproveChecker") ;

        if (checker == null) {
            log.println("Required relation not found !!!") ;
            throw new StatusException("Required relation not found !!!",
                new NullPointerException()) ;
        }
    }

    /**
    * Reexecutes the RowSet and checks that listener was called. <p>
    * Has OK status if no exceptions were raised and listener was called.
    */
    public void _execute() {
        requiredMethod("addRowSetListener()");
        listener.reset();
        try {
            oObj.execute() ;
        } catch (SQLException e) {
            log.println("Exception occurred :" + e) ;
        }

        tRes.tested("execute()", listener.rowSetChanged);
    }

    /**
    * Adds listener and calls methods moveCursor, changeRow,
    * changeRowSet of the relation and then checks if appropriate
    * methods of the listener were called. <p>
    * Has OK status if all listener methods were called.
    */
    public void _addRowSetListener() {
        boolean result = true ;

        oObj.addRowSetListener(listener) ;

        checker.moveCursor() ;
        result &= listener.cursorMoved ;
        if (!listener.cursorMoved)
            log.println("cursorMoved event wasn't called") ;
        listener.reset() ;

        checker.changeRow() ;
        result &= listener.rowChanged ;
        if (!listener.rowChanged)
            log.println("rowChanged event wasn't called") ;
        listener.reset() ;

        checker.changeRowSet() ;
        result &= listener.rowSetChanged ;
        if (!listener.rowSetChanged)
            log.println("rowSetChanged event wasn't called") ;
        listener.reset() ;

        tRes.tested("addRowSetListener()", result) ;
    }

    /*
    * Removes listener added before, and checks for no listener
    * methods were called on response to rowSet manipulations. <p>
    * Methods to be successfully completed before :
    * <ul>
    * <li> <code>addRowSetListener()</code> </li>
    * </ul> <p>
    * Has OK status if no listeners methods were called.
    */
    public void _removeRowSetListener() {
        requiredMethod("addRowSetListener()") ;

        boolean result = true ;

        oObj.removeRowSetListener(listener) ;

        checker.moveCursor() ;
        if (listener.cursorMoved) {
            log.println("cursorMoved is erroneously set");
            result = false;
        }
        listener.reset() ;

        checker.changeRow() ;
        if (listener.rowChanged) {
            log.println("rowChanged is erroneously set");
            result = false;
        }
        listener.reset() ;

        checker.changeRowSet() ;
        if (listener.rowSetChanged) {
            log.println("rowSetChanged is erroneously set");
            result = false;
        }

        tRes.tested("removeRowSetListener()", result) ;
    }

    /**
    * Disposes test environment.
    */
    @Override
    public void after() {
        disposeEnvironment() ;
    }

}  // finish class _XRowSet

