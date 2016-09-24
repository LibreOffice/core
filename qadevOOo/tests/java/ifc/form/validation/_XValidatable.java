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

    public static class MyValidator implements XValidator {
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