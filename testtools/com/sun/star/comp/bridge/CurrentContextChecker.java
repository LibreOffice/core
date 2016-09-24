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

package com.sun.star.comp.bridge;

import com.sun.star.uno.Any;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XCurrentContext;
import test.testtools.bridgetest.XCurrentContextChecker;

final class CurrentContextChecker implements XCurrentContextChecker {


    public boolean perform(
        XCurrentContextChecker other, int setSteps, int checkSteps)
    {
        if (setSteps == 0) {
            XCurrentContext old = UnoRuntime.getCurrentContext();
            UnoRuntime.setCurrentContext(
                new XCurrentContext() {
                    public Object getValueByName(String Name) {
                        return Name.equals(KEY)
                            ? (Object) VALUE : (Object) Any.VOID;
                    }
                });
            try {
                return performCheck(other, setSteps, checkSteps);
            } finally {
                UnoRuntime.setCurrentContext(old);
            }
        } else {
            return performCheck(other, setSteps, checkSteps);
        }
    }

    private boolean performCheck(
        XCurrentContextChecker other, int setSteps, int checkSteps)
    {
        // assert other != null && checkSteps >= 0;
        if (checkSteps == 0) {
            XCurrentContext context = UnoRuntime.getCurrentContext();
            if (context == null) {
                return false;
            }
            Any a = Any.complete(context.getValueByName(KEY));
            return
                a.getType().equals(Type.STRING) && a.getObject().equals(VALUE);
        } else {
            return other.perform(
                this, setSteps >= 0 ? setSteps - 1 : -1, checkSteps - 1);
        }
    }

    private static final String KEY = "testtools.bridgetest.Key";
    private static final String VALUE = "good";
}
