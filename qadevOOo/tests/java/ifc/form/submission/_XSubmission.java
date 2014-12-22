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

        util.utils.pause(2000);

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
