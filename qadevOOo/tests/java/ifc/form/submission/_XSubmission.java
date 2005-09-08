/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XSubmission.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:57:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
package ifc.form.submission;

import com.sun.star.form.submission.XSubmission;
import com.sun.star.form.submission.XSubmissionVetoListener;
import com.sun.star.task.XInteractionHandler;
import com.sun.star.uno.UnoRuntime;

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
