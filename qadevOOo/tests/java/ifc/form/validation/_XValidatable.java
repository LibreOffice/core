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