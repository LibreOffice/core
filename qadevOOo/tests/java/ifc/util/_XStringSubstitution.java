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

import com.sun.star.util.XStringSubstitution;
import lib.MultiMethodTest;

public class _XStringSubstitution extends MultiMethodTest {

    public XStringSubstitution oObj;

    public void _getSubstituteVariableValue() {
        boolean res = true;
        try {
            log.println("try to get the valid variable $(user) ...");
            String toCheck = "$(user)";
            String eString = oObj.getSubstituteVariableValue(toCheck);
            res = eString.startsWith("file:///");
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("$(user) does not exist");
            tRes.tested("getSubstituteVariableValue()",false);
        }
        try {
            log.println("try to get a invalid variable...");
            String toCheck = "$(ThisVariableShouldNoExist)";
            String eString = oObj.getSubstituteVariableValue(toCheck);
            log.println("$(ThisVariableShouldNoExist) should not exist");
            tRes.tested("getSubstituteVariableValue()",false);

        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("expected exception was thrown.");
            res &= true;
        }

        tRes.tested("getSubstituteVariableValue()",res);
    }

    public void _substituteVariables() {
        boolean res = true;
        try {
            log.println("try to get a valid variable...");
            String toCheck = "$(user)";
            String eString = oObj.substituteVariables(toCheck, false);
            log.println(eString);
            res = eString.startsWith("file:///");
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("$(user) does not exist");
            tRes.tested("substituteVariables()",false);
        }
        try {
            log.println("try to get a invalid variable...");
            String toCheck = "$(ThisVariableShouldNoExist)";
            String eString = oObj.substituteVariables(toCheck,true);
            log.println("$(ThisVariableShouldNoExist) should not exist");
            tRes.tested("substituteVariables()",false);

        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("expected exception was thrown.");
            res &= true;
        }

        tRes.tested("substituteVariables()",res);
    }

    public void _reSubstituteVariables() {
        boolean res = true;
        log.println("try to get a valid variable...");
        String toCheck = "file:///";
        String eString = oObj.reSubstituteVariables(toCheck);
        log.println(eString);
        res = eString.startsWith("file:///");

        tRes.tested("reSubstituteVariables()",res);
    }

}
