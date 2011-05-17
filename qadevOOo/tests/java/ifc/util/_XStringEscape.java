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

import lib.MultiMethodTest;

import com.sun.star.util.XStringEscape;

public class _XStringEscape extends MultiMethodTest {

    public XStringEscape oObj;

    public void _escapeString() {
        log.println("The Implementation of this Interface doesn't really do anything");
        boolean res = true;
        try {
            String toCheck = ";:<>/*";
            String eString = oObj.escapeString(toCheck);
            res = toCheck.equals(eString);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            System.out.println("IllegalArgument");
        }
        tRes.tested("escapeString()",res);
    }

    public void _unescapeString() {
        log.println("The Implementation of this Interface doesn't really do anything");
        boolean res = true;
        try {
            String toCheck = ";:<>/*";
            String ueString = oObj.unescapeString(toCheck);
            res = toCheck.equals(ueString);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            System.out.println("IllegalArgument");
        }
        tRes.tested("unescapeString()",res);
    }

}
