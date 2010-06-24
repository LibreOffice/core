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

package com.sun.star.comp.bridge;

import com.sun.star.uno.Any;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XCurrentContext;
import test.testtools.bridgetest.XCurrentContextChecker;

final class CurrentContextChecker implements XCurrentContextChecker {
    public CurrentContextChecker() {}

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
