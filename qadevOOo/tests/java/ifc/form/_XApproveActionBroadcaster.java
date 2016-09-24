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
import lib.Status;
import lib.StatusException;

import com.sun.star.form.XApproveActionBroadcaster;
import com.sun.star.form.XApproveActionListener;
import com.sun.star.lang.EventObject;

/**
* Testing <code>com.sun.star.form.XApproveActionBroadcaster</code>
* interface methods:
* <ul>
*  <li><code> addApproveActionListener() </code></li>
*  <li><code> removeApproveActionListener() </code></li>
* </ul><p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.form.XApproveActionBroadcaster
*/
public class _XApproveActionBroadcaster extends MultiMethodTest {
    public XApproveActionBroadcaster oObj = null;

    /**
    * Class we need to test methods.
    */
    protected class TestListener implements XApproveActionListener {
        public boolean approve = false ;

        public void init() {
            approve = false ;
        }
        public void disposing(EventObject ev) {}
        public boolean approveAction(EventObject ev) {
            log.println("XApproveActionListener: ActionListener was called");
            return approve ;
        }

    }

    private final TestListener listener = new TestListener();

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _addApproveActionListener() {
        oObj.addApproveActionListener(listener) ;
        tRes.tested("addApproveActionListener()", true);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addApproveActionListener() </code> : adds listener to an
    * object </li>
    * </ul>
    */
    public void _removeApproveActionListener() {
        requiredMethod("addApproveActionListener()");
        listener.init() ;
        listener.approve = true ;
        oObj.removeApproveActionListener(listener);
        tRes.tested("removeApproveActionListener()", true);
    }


    /**
    * Just log output
    */
    @Override
    protected void after() {
        log.println("Skipping all XApproveActionBroadcaster methods, since they"
                + " need user interaction");
        throw new StatusException(Status.skipped(true));
    }

}


