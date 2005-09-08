/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XValidatable.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:57:53 $
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
package ifc.form.validation;

import com.sun.star.form.validation.XValidatable;
import com.sun.star.form.validation.XValidator;

import lib.MultiMethodTest;


public class _XValidatable extends MultiMethodTest {
    public XValidatable oObj;

    public void _getValidator() {
        requiredMethod("setValidator()");

        XValidator xValidator = oObj.getValidator();
        boolean res = xValidator.isValid(Boolean.FALSE);
        tRes.tested("getValidator()", res);
    }

    public void _setValidator() {
        boolean res = false;

        try {
            oObj.setValidator(new MyValidator());

            XValidator xValidator = oObj.getValidator();
            String getting = xValidator.explainInvalid(null);
            res = getting.equals("explainInvalid");

            if (!res) {
                log.println("\tExpected: explainInvalid");
                log.println("\tGetting: " + getting);
                log.println("FAILED");
            }
        } catch (com.sun.star.util.VetoException e) {
            e.printStackTrace();
        }

        tRes.tested("setValidator()", res);
    }

    public class MyValidator implements XValidator {
        public void addValidityConstraintListener(com.sun.star.form.validation.XValidityConstraintListener xValidityConstraintListener)
            throws com.sun.star.lang.NullPointerException {
        }

        public String explainInvalid(Object obj) {
            return "explainInvalid";
        }

        public boolean isValid(Object obj) {
            return true;
        }

        public void removeValidityConstraintListener(com.sun.star.form.validation.XValidityConstraintListener xValidityConstraintListener)
            throws com.sun.star.lang.NullPointerException {
        }
    }
}