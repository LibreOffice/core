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
            XValidatable xValidatable = UnoRuntime.queryInterface(
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