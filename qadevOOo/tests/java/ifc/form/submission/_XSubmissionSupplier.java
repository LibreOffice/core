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

package ifc.form.submission;

import com.sun.star.form.submission.XSubmission;
import com.sun.star.form.submission.XSubmissionSupplier;
import com.sun.star.form.submission.XSubmissionVetoListener;
import com.sun.star.task.XInteractionHandler;
import lib.MultiMethodTest;

public class _XSubmissionSupplier extends MultiMethodTest
{
    public XSubmissionSupplier oObj = null;
    public boolean submitWasCalled = false;

    public void _Submission() {
        XSubmission stub = new MyXSubmission();
        oObj.setSubmission (stub);
        XSubmission xSubmission = oObj.getSubmission ();
        try {
            xSubmission.submit ();
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception during submit "+e.getMessage ());
        }
        catch (com.sun.star.util.VetoException e) {
            log.println("VetoException during submit "+e.getMessage ());
        }
        tRes.tested ("Submission()",submitWasCalled);
    }

    private class MyXSubmission implements XSubmission {

        public void submit () throws com.sun.star.util.VetoException, com.sun.star.lang.WrappedTargetException
        {
            submitWasCalled=true;
            log.println("MyXSubmission: someone called submit :-)");
        }

        public void submitWithInteraction( XInteractionHandler handler ) throws com.sun.star.util.VetoException, com.sun.star.lang.WrappedTargetException
        {
            log.println("MyXSubmission: someone called submitWithInteraction :-)");
        }

        public void addSubmissionVetoListener( XSubmissionVetoListener listener ) throws com.sun.star.lang.NoSupportException
        {
            throw new com.sun.star.lang.NoSupportException();
        }

        public void removeSubmissionVetoListener( XSubmissionVetoListener listener ) throws com.sun.star.lang.NoSupportException
        {
            throw new com.sun.star.lang.NoSupportException();
        }

    }

}
