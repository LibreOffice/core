/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XSubmissionSupplier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:57:37 $
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
