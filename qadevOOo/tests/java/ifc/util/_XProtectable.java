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
package ifc.util;

import com.sun.star.util.XProtectable;
import lib.MultiMethodTest;

/**
 * Check the XProtectable interface.
 */
public class _XProtectable extends MultiMethodTest {
    public XProtectable oObj = null;
    String sPassWord = "TopSecret";

    /**
     * Check, if the sheet is protected.
     * Has OK status, if this is the case.
     */
    public void _isProtected() {
        requiredMethod("protect()");
        boolean result = oObj.isProtected();
        tRes.tested("isProtected()", result);
    }

    /**
     * Protect the sheet
     */
    public void _protect() {
        oObj.protect(sPassWord);
        tRes.tested("protect()", true);
    }

    /**
     * Unprotect with wrong password, see if it's still protected.
     * Unprotect with correct password, see if it's unprotected.
     */
    public void _unprotect() {
        requiredMethod("isProtected()");
        boolean result = true;
        try {
            oObj.unprotect("WrongPassword");
//            result = false;
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Correct Exception thrown.");
        }
        // just check if it's still protected
        result &= oObj.isProtected();
        try {
            oObj.unprotect(sPassWord);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Wrong Exception thrown: password is correct.");
            result = false;
        }
        result &= !oObj.isProtected();
        tRes.tested("unprotect()", result);
    }

}
