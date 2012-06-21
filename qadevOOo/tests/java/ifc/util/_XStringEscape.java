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
