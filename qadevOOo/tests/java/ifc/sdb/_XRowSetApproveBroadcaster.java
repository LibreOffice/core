/*************************************************************************
 *
 *  $RCSfile: _XRowSetApproveBroadcaster.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-11-18 16:23:30 $
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

package ifc.sdb;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.lang.EventObject;
import com.sun.star.sdb.RowChangeEvent;
import com.sun.star.sdb.XRowSetApproveBroadcaster;
import com.sun.star.sdb.XRowSetApproveListener;

/**
* <code>com.sun.star.sdb.XRowSetApproveBroadcaster</code> interface test. <p>
* Required object relations :
* <ul>
* <li> <code>'XRowSetApproveBroadcaster.ApproveChecker'</code>:
*      implementation of inner interface <code>RowSetApproveChecker</code>
*      which can move cursor within a rowset, change row, and change the
*      whole rowset. </li>
* </ul> <p>
* It is better to recreate the object after test, because of unknown
* actions made by <code>RowSetApproveChecker</code> interface implementation.
*
* @see com.sun.star.sdb.XRowSetApproveBroadcaster
* @see _XRowSetApproveBroadcaster.RowSetApproveChecker
*/
public class _XRowSetApproveBroadcaster extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XRowSetApproveBroadcaster oObj = null ;

    /**
    * The purpose of this interface is to pass to this test
    * relation which can make some operations with row set
    * on which <code>XRowSetApproveListener</code>s can react.
    * @see com.sun.star.sdb.XRowSetApproveListener
    */
    public static interface RowSetApproveChecker {
        /**
        * Moves cursor within row set. Method <code>approveCursorMove</code>
        * of <code>XRowSetApproveListener</code> must be called.
        */
        public void moveCursor() ;
        /**
        * Change rows in row set. Method <code>approveRowChange</code>
        * of <code>XRowSetApproveListener</code> must be called.
        * @return <code>RowChangeEvent</code> structure which contains
        * what type of change was made and how many rows it affected.
        * @see com.sun.star.sdb.RowChangeEvent
        */
        public RowChangeEvent changeRow() ;
        /**
        * Change the whole row set. Method <code>approveRowSetChange</code>
        * of <code>XRowSetApproveListener</code> must be called.
        */
        public void changeRowSet() ;
    }

    /**
    * Implementation of <code>XRowSetApproveListener</code> interface
    * which just detects and stores approve requipements. They are checked
    * later.
    */
    private class TestListener implements XRowSetApproveListener {
        public boolean approveRequests = true ;
        public boolean approveCursorMoveCalled = false ;
        public boolean approveRowChangeCalled = false ;
        public RowChangeEvent approveRowChangeEvent = null ;
        public boolean approveRowSetChangeCalled = false ;

        public TestListener(boolean approve) {
            approveRequests = approve ;
        }

        public void reset() {
            approveCursorMoveCalled = false ;
            approveRowChangeCalled = false ;
            approveRowSetChangeCalled = false ;
        }
        public boolean approveCursorMove(EventObject ev) {
            approveCursorMoveCalled = true ;
            return approveRequests ;
        }
        public boolean approveRowChange(RowChangeEvent ev) {
            approveRowChangeCalled = true ;
            approveRowChangeEvent = ev ;
            return approveRequests ;
        }
        public boolean approveRowSetChange(EventObject ev) {
            approveRowSetChangeCalled = true ;
            return approveRequests ;
        }
        public void disposing(EventObject ev) {}
    }
    private TestListener listener1 = null ;

    private RowSetApproveChecker checker = null ;

    /**
    * Tries to retrieve object relation.
    */
    public void before() {
        checker = (RowSetApproveChecker) tEnv.getObjRelation
            ("XRowSetApproveBroadcaster.ApproveChecker") ;

        if (checker == null) {
            log.println("!!! Relation for test not found !!!") ;
            throw new StatusException(Status.failed
                ("!!! Relation for test not found !!!")) ;
        }
    }

    /**
    * Creates and adds listener, then call <code>RowSetApproveChecker</code>
    * methods for listener methods to be called. Then checks if
    * listener methods were called on appropriate actions. <p>
    * Has OK status : If and only if appropriate listener methods called,
    * and listener <code>approveRowChange</code> method has write parameter,
    * i.e. type and rows number expected.
    */
    public void _addRowSetApproveListener() {
        listener1 = new TestListener(true) ;
        oObj.addRowSetApproveListener(listener1) ;
        log.println("Listener added.") ;

        boolean result = true ;

        checker.moveCursor() ;
        log.println("Cursor moved.") ;
        result &= listener1.approveCursorMoveCalled ;

        listener1.reset() ;
        RowChangeEvent actualEvent = checker.changeRow() ;
        log.println("Row changed.") ;

        RowChangeEvent event = listener1.approveRowChangeEvent ;
        result &= listener1.approveRowChangeCalled ;

        boolean eventOK = event.Action == actualEvent.Action &&
                          event.Rows == actualEvent.Rows ;

        result &= eventOK ;

        listener1.reset() ;
        checker.changeRowSet();
        log.println("Row set changed.") ;
        result &= listener1.approveRowSetChangeCalled ;

        tRes.tested("addRowSetApproveListener()", result) ;
    }

    /**
    * Removes listener inserted before, then perform all actions
    * on which listener must react. <p>
    * Has OK status if no listener methods were called. <p>
    * Methods required to pass before :
    * <ul>
    * <li> <code>_addRowSetApproveListener</code> </li>
    * </ul>
    */
    public void _removeRowSetApproveListener() {
        requiredMethod("addRowSetApproveListener()") ;

        listener1.reset() ;

        oObj.removeRowSetApproveListener(listener1) ;

        checker.moveCursor() ;
        checker.changeRow() ;
        checker.changeRowSet() ;

        tRes.tested("removeRowSetApproveListener()",
            !listener1.approveCursorMoveCalled &&
            !listener1.approveRowChangeCalled &&
            !listener1.approveRowSetChangeCalled) ;
    }

    /**
    * Disposes object environment.
    */
    public void after() {
        disposeEnvironment() ;
    }

}  // finish class _XRowSetApproveBroadcaster


