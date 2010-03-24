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
* Test is <b> NOT </b> multithread compilant. <p>
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

    private TestListener listener = new TestListener();

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
    protected void after() {
        log.println("Skipping all XApproveActionBroadcaster methods, since they"
                + " need user interaction");
        throw new StatusException(Status.skipped(true));
    }

}


