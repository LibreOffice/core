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
import com.sun.star.form.validation.XValidityConstraintListener;
import com.sun.star.uno.UnoRuntime;

import lib.MultiMethodTest;


public class _XValidityConstraintListener extends MultiMethodTest {
    public XValidityConstraintListener oObj;
    protected boolean ValidatorCalled = false;

    public void _validityConstraintChanged() {
        boolean res = false;
        try {
            XValidatable xValidatable = (XValidatable) UnoRuntime.queryInterface(
                                                XValidatable.class,
                                                tEnv.getTestObject());

            log.println("adding Validator");
            XValidator xValidator = new MyValidator();
            xValidatable.setValidator(xValidator);
            ValidatorCalled = false;

            log.println("calling validityConstraintChanged()");
            oObj.validityConstraintChanged(
                    new com.sun.star.lang.EventObject());
            res = ValidatorCalled;
        } catch (com.sun.star.util.VetoException e) {
            e.printStackTrace();
        }
        tRes.tested("validityConstraintChanged()",res);
    }

    /*
     * The validator to add this Listener implementation
     *
     */
    public class MyValidator implements XValidator {
        public void addValidityConstraintListener(com.sun.star.form.validation.XValidityConstraintListener xValidityConstraintListener)
            throws com.sun.star.lang.NullPointerException {
            log.println("\t Validator::addValidityConstraintListener called");
        }

        public String explainInvalid(Object obj) {
            log.println("\t Validator::explainInvalid() called");

            return "explainInvalid";
        }

        public boolean isValid(Object obj) {
            log.println("\t Validator::isValid() called");
            ValidatorCalled = true;
            return false;
        }

        public void removeValidityConstraintListener(com.sun.star.form.validation.XValidityConstraintListener xValidityConstraintListener)
            throws com.sun.star.lang.NullPointerException {
            log.println("\t Validator::removeValidityConstraintListener called");
        }
    }
}