/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CurrentContextChecker.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 14:43:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2006 by Sun Microsystems, Inc.
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
