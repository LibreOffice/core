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
