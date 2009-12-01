/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XSubmission.java,v $
 * $Revision: 1.5 $
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
package ifc.form.submission;

import com.sun.star.form.submission.XSubmission;
import com.sun.star.form.submission.XSubmissionVetoListener;

import lib.MultiMethodTest;


public class _XSubmission extends MultiMethodTest {
    public XSubmission oObj = null;
    public boolean HandlerWasCalled = false;

    public void _addSubmissionVetoListener() {
        log.println(
            "submitting with VetoListener ... exception should appear");

        boolean res = true;
        XSubmissionVetoListener aListener = new MyListener();

        try {
            oObj.addSubmissionVetoListener(aListener);
            oObj.submit();
            res = false;
            log.println(
                "the expected exception wasn't thrown ... FAILED");
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println(
                "Expected exception was thrown while calling submit() "
                + e.getMessage() + "FAILED");
            res = false;
        } catch (com.sun.star.lang.NoSupportException e) {
            log.println(
                "NoSupportExpected exception was thrown while calling submit() "
                + e.getMessage() + "FAILED");
            res = false;
        } catch (com.sun.star.util.VetoException e) {
            log.println(
                "VetoException was thrown while calling submit() "
                + e.getMessage() + "OK");
        }

        try {
            oObj.removeSubmissionVetoListener(aListener);
        } catch (com.sun.star.lang.NoSupportException e) {
            log.println(
                "NoSupportExpected exception was thrown while removing the listener) "
                + e.getMessage() + "FAILED");
            res = false;
        }

        tRes.tested("addSubmissionVetoListener()", res);
    }

    public void _removeSubmissionVetoListener() {
        log.println(
            "submitting with VetoListener ... exception should appear");

        boolean res = true;
        XSubmissionVetoListener aListener = new MyListener();

        try {
            oObj.addSubmissionVetoListener(aListener);
            oObj.submit();
            res = false;
            log.println(
                "the expected exception wasn't thrown ... FAILED");
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println(
                "WrappedTargetException exception was thrown while calling submit() "
                + e.getMessage() + "FAILED");
            res = false;
        } catch (com.sun.star.lang.NoSupportException e) {
            log.println(
                "NoSupportExpected exception was thrown while calling submit() "
                + e.getMessage() + "FAILED");
            res = false;
        } catch (com.sun.star.util.VetoException e) {
            log.println(
                "VetoException was thrown while calling submit() "
                + e.getMessage() + "OK");
        }

        log.println("removing the listener");

        try {
            oObj.removeSubmissionVetoListener(aListener);
        } catch (com.sun.star.lang.NoSupportException e) {
            log.println(
                "NoSupportExpected exception was thrown while removing the listener) "
                + e.getMessage() + "FAILED");
            res = false;
        }

        log.println("Sleeping 2s");

        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            // sleeping didn't work
        }

        log.println("... done");

        log.println(
            "submitting after VetoListener has been removed... no exception should appear");

        try {
            oObj.submit();
            log.println("No Exception ... OK");
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println(
                "WrappedTargetException was thrown while calling submit() "
                + e.getMessage() + "FAILED");
            res = false;
        } catch (com.sun.star.util.VetoException e) {
            log.println(
                "VetoException was thrown while calling submit() "
                + e.getMessage() + "FAILED");
            res = false;
        }

        tRes.tested("removeSubmissionVetoListener()", res);
    }

    /** Calls submit and returns true if no exception was thrown
     *  then adds a SubmissionVetoListener and checks if the
     *  exception is thrown in case of a veto of this listener.
     */
    public void _submit() {
        boolean res = true;
        log.println(
            "submitting without VetoListener ... no exception should appear");

        try {
            oObj.submit();
            log.println("No Exception ... OK");
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println(
                "Exception was thrown while calling submit() "
                + e.getMessage() + "FAILED");
            res = false;
        } catch (com.sun.star.util.VetoException e) {
            log.println(
                "VetoException was thrown while calling submit() "
                + e.getMessage() + "FAILED");
            res = false;
        }

        tRes.tested("submit()", res);
    }

    public class MyListener implements XSubmissionVetoListener {
        public void disposing(
            com.sun.star.lang.EventObject eventObject) {
        }

        public void submitting(
            com.sun.star.lang.EventObject eventObject)
                throws com.sun.star.util.VetoException {
            log.println("MyVetoListener was called");
            throw new com.sun.star.util.VetoException(
                "submission isn't allowed ...");
        }
    }

}
