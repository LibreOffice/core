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
*    committed.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.form.XUpdateBroadcaster
*/
public class _XUpdateBroadcaster extends MultiMethodTest {

    public XUpdateBroadcaster oObj = null;
    UpdateChecker checker = null ;

    /**
    * Interface for relation. Updating, committing and checking
    * if data was committed is object dependent behaviour.
    */
    public interface UpdateChecker {
        /**
        * Method must make some data update in the object tested.
        */
        void update() throws com.sun.star.uno.Exception ;
        /**
        * Method must commit data change made by method <code>update</code>.
        */
        void commit() throws com.sun.star.uno.Exception ;
        /**
        * Checks if the data committed by <code>commit</code> method
        * became permanent in data source.
        * @return <code>true</code> if data was committed.
        */
        boolean wasCommited() throws com.sun.star.uno.Exception ;
    }

    /**
    * Retrieves object relations.
    * @throws StatusException If one of relations not found.
    */
    @Override
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
    protected static class TestListener implements XUpdateListener {
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

    private final TestListener listener = new TestListener();

    /**
    * The listener methods calls are checked twice with approving
    * and rejecting updates. <p>
    * Has <b>OK</b> status if on update rejected only <code>
    * approveUpdate</code> listener method was called, and if
    * on update approved <code>approveUpdate</code> and
    * <code>updated</code> methods called, and data was committed
    * to the source.
    */
    public void _addUpdateListener() {
        boolean bResult = true;

        oObj.addUpdateListener(listener) ;

        try {
            checker.update() ;
            waitForEventIdle();
            checker.commit() ;
            waitForEventIdle();
            boolean committed = checker.wasCommited() ;

            waitForEventIdle();

            bResult = listener.approveCalled &&
                      ! listener.updateCalled &&
                      ! committed ;

            log.println("Calling with no approving : approveUpdate() was " +
                (listener.approveCalled ? "":"NOT")+" called, updated() was "+
                (listener.updateCalled ? "":"NOT")+" called, the value was " +
                (committed ? "" : "NOT") + " committed.") ;

            waitForEventIdle();

            listener.init() ;
            listener.approve = true ;
            waitForEventIdle();
            checker.update() ;
            waitForEventIdle();
            checker.commit() ;
            waitForEventIdle();
            committed = checker.wasCommited() ;

            waitForEventIdle();

            log.println("Calling with approving : approveUpdate() was " +
                (listener.approveCalled ? "":"NOT")+" called, updated() was "+
                (listener.updateCalled ? "":"NOT")+" called, the value was "+
                (committed ? "" : "NOT") + " committed.") ;

            bResult = listener.approveCalled &&
                      listener.updateCalled &&
                      committed ;
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
            waitForEventIdle();
            checker.commit() ;

            waitForEventIdle();

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

    /**
    * Forces environment recreation.
    */
    @Override
    protected void after() {
        disposeEnvironment();
    }

}


