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

package ifc.scripting;

import drafts.com.sun.star.script.framework.provider.XFunctionProvider;
import drafts.com.sun.star.script.framework.provider.XFunction;

import lib.MultiMethodTest;
import lib.StatusException;
import lib.Parameters;

import java.util.Collection;
import java.util.Iterator;

public class _XFunctionProvider extends MultiMethodTest {

    public XFunctionProvider oObj = null;

    /**
    * Retrieves object relation.
    */
    public void before() throws StatusException {
    }

    public void _getFunction() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_getFunction");

        Iterator tests;

        if (c != null) {
            tests = c.iterator();

            while (tests.hasNext()) {
                result &= runGetFunctionTest((Parameters)tests.next());
            }
        }
        else {
            result = false;
        }

        tRes.tested("getFunction()", result);
    }

    private boolean runGetFunctionTest(Parameters testdata) {
        String description = testdata.get("description");
        String logicalname = testdata.get("logicalname");
        String expected = testdata.get("expected");
        String output = "";

        log.println(testdata.get("description"));

        XFunction function = oObj.getFunction(logicalname);

        if (function == null)
            output = "null";
        else
            output = "XFunction.class";

        log.println("expected: " + expected + ", output: " + output);
        if (output.equals(expected))
            return true;
        else
            return false;
    }
}
