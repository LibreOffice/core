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
            if (!getting.equals(expectedName)) {
                log.println("didn't get the expected Name:");
                log.println("getting: " + getting);
                log.println("Expected: " + expectedName);
                res = false;
            }
        }

        nameFor = expectedName.toUpperCase();

        log.println("Getting exact name for " + nameFor);
        getting = oObj.getExactName(nameFor);

        if (getting == null) {
            res = false;
        } else {
            if (!getting.equals(expectedName)) {
                log.println("didn't get the expected Name:");
                log.println("getting: " + getting);
                log.println("Expected: " + expectedName);
                res = false;
            }
        }

        tRes.tested("getExactName()", res);
    }

    /**
     * Checking if the Object relation expectedName is given
     * throw a StatusException if not.
     */
    @Override
    protected void before() {
        expectedName = (String) tEnv.getObjRelation("expectedName");

        if (expectedName == null) {
            throw new StatusException(Status.failed(
                                              "Object relation expectedName is missing"));
        }
    }
}
