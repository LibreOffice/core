/*************************************************************************
 *
 *  $RCSfile: _XRowSet.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:54:51 $
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
    private TestListener listener = new TestListener() ;

    private class TestListener implements XRowSetListener {
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
    * Has OK status if no exceptions were rised and listener was called.
    */
    public void _execute() {
        requiredMethod("addRowSetListener()");
        listener.reset();
        boolean result = true ;

        try {
            oObj.execute() ;
        } catch (SQLException e) {
            log.println("Exception occured :" + e) ;
            result = false ;
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
        result &= !listener.cursorMoved ;
        listener.reset() ;

        checker.changeRow() ;
        result &= !listener.rowChanged ;
        listener.reset() ;

        checker.changeRowSet() ;
        result &= !listener.rowSetChanged ;

        tRes.tested("removeRowSetListener()", result) ;
    }

    /**
    * Disposes test environment.
    */
    public void after() {
        disposeEnvironment() ;
    }

}  // finish class _XRowSet

