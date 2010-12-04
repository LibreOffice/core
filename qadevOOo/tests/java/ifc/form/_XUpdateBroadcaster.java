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

package ifc.form;

import lib.MultiMethodTest;
import lib.StatusException;

import com.sun.star.form.XUpdateBroadcaster;
import com.sun.star.form.XUpdateListener;
import com.sun.star.lang.EventObject;

/**
* Testing <code>com.sun.star.form.XUpdateBroadcaster</code>
* interface methods :
* <ul>
*  <li><code> addUpdateListener()</code></li>
*  <li><code> removeUpdateListener()</code></li>
* </ul>
* This test needs the following object relations :
* <ul>
*  <li> <code>'XUpdateBroadcaster.Checker'</code> : <code>
*    _XUpdateBroadcaster.UpdateChecker</code> interface implementation
*    which can update, commit data and check if the data was successfully
*    commited.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.form.XUpdateBroadcaster
*/
public class _XUpdateBroadcaster extends MultiMethodTest {

    public XUpdateBroadcaster oObj = null;
    UpdateChecker checker = null ;

    /**
    * Interface for relation. Updating, commiting and checking
    * if data was commited is object dependent behaviour.
    */
    public static interface UpdateChecker {
        /**
        * Method must make some data update in the object tested.
        */
        public void update() throws com.sun.star.uno.Exception ;
        /**
        * Method must commit data change made by method <code>update</code>.
        */
        public void commit() throws com.sun.star.uno.Exception ;
        /**
        * Checks if the data commited by <code>commit</code> method
        * became permanent in data source.
        * @return <code>true</code> if data was commited.
        */
        public boolean wasCommited() throws com.sun.star.uno.Exception ;
    }

    /**
    * Retrieves object relations.
    * @throws StatusException If one of relations not found.
    */
    public void before() {
        checker = (UpdateChecker)
            tEnv.getObjRelation("XUpdateBroadcaster.Checker") ;
        if (checker == null) {
            log.println("Relation not found") ;
            throw new StatusException("Relation not found",
                new NullPointerException("Relation not found")) ;
        }
    }

    /**
    * Listener implementation, which can accept or reject update
    * requests and store event calls.
    */
    protected class TestListener implements XUpdateListener {
        /**
        * Indicates must listener approve update requests or not.
        */
        public boolean approve = false ;
        /**
        * Indicates that <code>approveUpdate</code> method was called.
        */
        public boolean approveCalled = false ;
        /**
        * Indicates that <code>updated</code> method was called.
        */
        public boolean updateCalled = false ;

        /**
        * Clears all flags.
        */
        public void init() {
            approveCalled = false ;
            updateCalled = false ;
        }
        public void disposing(EventObject ev) {}
        public boolean approveUpdate(EventObject ev) {
            approveCalled = true ;
            return approve ;
        }
        public void updated(EventObject ev) {
            updateCalled = true ;
        }
    }

    private TestListener listener = new TestListener();

    /**
    * The listener methods calls are checked twice with approving
    * and rejecting updates. <p>
    * Has <b>OK</b> status if on update rejected only <code>
    * approveUpdate</code> listener method was called, and if
    * on update approved <code>approveUpdate</code> and
    * <code>updated</code> methods called, and data was commited
    * to the source.
    */
    public void _addUpdateListener() {
        boolean bResult = true;

        oObj.addUpdateListener(listener) ;

        try {
            checker.update() ;
            shortWait() ;
            checker.commit() ;
            shortWait() ;
            boolean commited = checker.wasCommited() ;

            shortWait() ;

            bResult = listener.approveCalled &&
                      ! listener.updateCalled &&
                      ! commited ;

            log.println("Calling with no approving : approveUpdate() was " +
                (listener.approveCalled ? "":"NOT")+" called, updated() was "+
                (listener.updateCalled ? "":"NOT")+" called, the value was " +
                (commited ? "" : "NOT") + " commited.") ;

            shortWait() ;

            listener.init() ;
            listener.approve = true ;
            shortWait() ;
            checker.update() ;
            shortWait() ;
            checker.commit() ;
            shortWait() ;
            commited = checker.wasCommited() ;

            shortWait() ;

            log.println("Calling with approving : approveUpdate() was " +
                (listener.approveCalled ? "":"NOT")+" called, updated() was "+
                (listener.updateCalled ? "":"NOT")+" called, the value was "+
                (commited ? "" : "NOT") + " commited.") ;

            bResult = listener.approveCalled &&
                      listener.updateCalled &&
                      commited ;
        } catch (com.sun.star.uno.Exception e) {
            bResult = false ;
            e.printStackTrace(log) ;
        }

        tRes.tested("addUpdateListener()", bResult);
    }

    /**
    * Removes listener, updates data, and checks if no listener
    * methods were called. <p>
    * Has <b> OK </b> status if after listener removing no of its methods
    * were called. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addUpdateListener </code> : to have a listener added.</li>
    * </ul>
    */
    public void _removeUpdateListener() {
        requiredMethod("addUpdateListener()");
        boolean bResult = true;

        listener.init() ;
        listener.approve = true ;

        oObj.removeUpdateListener(listener);

        try {
            checker.update() ;
            shortWait() ;
            checker.commit() ;

            shortWait() ;

            bResult = ! listener.approveCalled &&
                      ! listener.updateCalled ;
        }
        catch (com.sun.star.uno.Exception e) {
            log.println("Exception occurred during removeUpdateListener()");
            e.printStackTrace(log);
            bResult = false;
        }

        tRes.tested("removeUpdateListener()", bResult);
    }

    private void shortWait() {
        try {
            Thread.sleep(200);
        }
        catch (InterruptedException ex) {
        }

    }

    /**
    * Forces environment recreation.
    */
    protected void after() {
        disposeEnvironment();
    }

}


