/*************************************************************************
 *
 *  $RCSfile: _XActiveDataControl.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:42:07 $
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

package ifc.io;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.io.XActiveDataControl;
import com.sun.star.io.XStreamListener;
import com.sun.star.lang.EventObject;

/**
 * Testing <code>com.sun.star.io.XActiveDataControl</code>
 * interface methods :
 * <ul>
 *  <li><code> addListener()</code></li>
 *  <li><code> removeListener()</code></li>
 *  <li><code> start()</code></li>
 *  <li><code> terminate()</code></li>
 * </ul> <p>
 *
 * Tests <code>XActiveDataControl</code> interface. First, it registers a listener
 * and performs <code>start()</code> and <code>terminate()</code> calls. The
 * events received in the listener are analyzed to verify the result.<p>
 *
 * @see com.sun.star.io.XActiveDataControl
 */
public class _XActiveDataControl extends MultiMethodTest {

    /**
     * Contains the object under test.
     */
    public XActiveDataControl oObj = null;

    /**
     * Indicates that the <code>XStreamListener.started()</code> method has
     * been called.
     */
    private boolean startCalled = false;

    /**
     * Indicates that the <code>XStreamListener.terminated()</code> method has
     * been called.
     */
    private boolean terminateCalled = false;

    /**
     * Indicates that the <code>XEventListener.closed()</code> method has
     * been called.
     */
    private boolean closeCalled = false;

    /**
     * Indicates that the <code>XStreamListener.error()</code> method has
     * been called.
     */
    private boolean errorCalled = false;

    /**
     * Contains the error, if <code>XStreamListener.error(Object error)</code>
     * method was called.
     */
    private Object error;

    /**
     * Indicates that the <code>XEventListener.disposing()</code> method has
     * been called.
     */
    private boolean smthngElseCalled = false;

    /**
     * The listener is used to verify results of the methods.
     */
    private TestStreamListener listener = new TestStreamListener();

    /**
     * XStreamListener implementation. Sets variables
     * (<cod>estartedCalled</code>, <code>terminatedCalled</code>, etc.) to
     * <tt>true</tt> if the appropriate method was called (for example, if
     * <code>started()</code> was called, the <code>startedCalled</code>
     * field is set).
     */
    private class TestStreamListener implements XStreamListener {
        public void started() {
            startCalled = true ;
        }
        public void terminated() {
            terminateCalled = true ;
        }
        public void error(Object e) {
            error = e;
            errorCalled = true ;
        }
        public void closed() {
            closeCalled = true ;
        }
        public void disposing(EventObject e) {
            smthngElseCalled = true ;
        }

    }

    /**
     * Tests <code>addListener()</code>. The verification is performed later, in
     * <code>_terminate()</code> method.
     */
    public void _addListener() {
        oObj.addListener(listener);
    }

    /**
     * Starts the data activity (e.g. data pump). Verifictation is performed
     * later, in <code>_terminate()</code> method.
     */
    public void _start() {
        executeMethod("addListener()");

        oObj.start();

        // waiting a little bit for data transfered
        try {
            Thread.sleep(200);
        } catch (InterruptedException e) {
            e.printStackTrace(log) ;
            throw new StatusException(Status.failed(e.getMessage()));
        }
    }

    /**
     * Tests <code>removeListener()</code>. Before, it ensures that other
     * tests are perforemed and that <code>addListener()</code> is okay. Then,
     * calls <code>XActiveDataControl.start()</code> and checks that no method
     * of the listener was called.
     */
    public void _removeListener() {
        // performing other tests before, so, that don't break them
        try {
            executeMethod("terminate()");
        } catch (StatusException e) {
            // the result doesn't matter
        }

        // check that addListener() is okay
        requiredMethod("addListener()");

        // clearing previous records
        startCalled = false;
        terminateCalled = false;
        errorCalled = false;
        error = null;
        smthngElseCalled = false;

        // removing the listener
        oObj.removeListener(listener);

        // starting the activity
        oObj.start();

        // wait a little bit to allow for listeners to be called
        try {
            Thread.sleep(200);
        } catch (InterruptedException e) {
            e.printStackTrace(log) ;
            throw new StatusException(Status.failed(e.getMessage()));
        }

        // check that no removed listener's method was called
        tRes.tested("removeListener()",!startCalled &&
            !terminateCalled && !errorCalled && !smthngElseCalled) ;
    }

    /**
     * Tests <code>terminate()</code>. First, ensures that <code>start()</code>
     * has been called. Then, verifies <code>start()</code>,
     * <code>addListener()</code> and <code>terminate()</code> results, by
     * checking that the appropriate listener's methods have been called.
     */
    public void _terminate() {
        // ensuring that the activity has been started
        executeMethod("start()");

        // terminating the activity
        oObj.terminate();

        // waiting a little bit for listeners to be called
        try {
            Thread.sleep(200);
        } catch (InterruptedException e) {
            e.printStackTrace(log) ;
            throw new StatusException(Status.failed(e.getMessage()));
        }

        // check, if any error occured
        if (errorCalled) {
            Status.failed("Unexpected error");
            log.println("Unexpected error " + error);
            ((Exception)error).printStackTrace(log);
        }

        // verification of start() method - startedCalled method should be
        // called
        if (!tRes.tested("start()", startCalled)) {
            log.println("XStreamListener.started() was not called()");
        }

        // check that any listener method is called
        tRes.tested("addListener()", startCalled ||
                terminateCalled || errorCalled || smthngElseCalled);

        // checking that terminated() has been called or streams were closed
        // before terminate() call, in this case termination has no sense.
        tRes.tested("terminate()", terminateCalled || closeCalled);
    }

    /**
     * Disposes the test environment, since it is used.
     */
    public void after() {
        tEnv.dispose();
    }
}


