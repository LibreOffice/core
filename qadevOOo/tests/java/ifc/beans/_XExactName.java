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

package ifc.beans;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.XExactName;


public class _XExactName extends MultiMethodTest {
    protected String expectedName = "";
    public XExactName oObj = null;

    public void _getExactName() {
        String nameFor = expectedName.toLowerCase();

        log.println("Getting exact name for " + nameFor);

        String getting = oObj.getExactName(nameFor);

        boolean res = true;

        if (getting == null) {
            res = false;
        } else {
            res = getting.equals(expectedName);
        }

        if (!res) {
            log.println("didn't get the expected Name:");
            log.println("getting: " + getting);
            log.println("Expected: " + expectedName);
        }

        nameFor = expectedName.toUpperCase();

        log.println("Getting exact name for " + nameFor);
        getting = oObj.getExactName(nameFor);

        if (getting == null) {
            res = false;
        } else {
            res &= getting.equals(expectedName);
        }

        if (!getting.equals(expectedName)) {
            log.println("didn't get the expected Name:");
            log.println("getting: " + getting);
            log.println("Expected: " + expectedName);
        }

        tRes.tested("getExactName()", res);
    }

    /**
     * Checking if the Object relation expectedName is given
     * throw a StatusException if not.
     */
    protected void before() {
        expectedName = (String) tEnv.getObjRelation("expectedName");

        if (expectedName == null) {
            throw new StatusException(Status.failed(
                                              "Object relation expectedName is missing"));
        }
    }
}