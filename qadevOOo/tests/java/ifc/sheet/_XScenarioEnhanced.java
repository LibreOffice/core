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
package ifc.sheet;

import com.sun.star.sheet.XScenarioEnhanced;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.uno.UnoRuntime;

import lib.MultiMethodTest;

public class _XScenarioEnhanced extends MultiMethodTest {
    public XScenarioEnhanced oObj = null;

    public void before() {
        oObj = UnoRuntime.queryInterface(
                       XScenarioEnhanced.class,
                       tEnv.getObjRelation("ScenarioSheet"));
    }

    public void _getRanges() {
        boolean res = true;
        CellRangeAddress[] getting = oObj.getRanges();
        System.out.println("Count " + getting.length);

        CellRangeAddress first = getting[0];

        if (!(first.Sheet == 1)) {
            log.println(
                    "wrong RangeAddress is returned, expected Sheet=0 and got " +
                    first.Sheet);
            res = false;
        }

        if (!(first.StartColumn == 0)) {
            log.println(
                    "wrong RangeAddress is returned, expected StartColumn=0 and got " +
                    first.StartColumn);
            res = false;
        }

        if (!(first.EndColumn == 10)) {
            log.println(
                    "wrong RangeAddress is returned, expected EndColumn=10 and got " +
                    first.EndColumn);
            res = false;
        }

        if (!(first.StartRow == 0)) {
            log.println(
                    "wrong RangeAddress is returned, expected StartRow=0 and got " +
                    first.StartRow);
            res = false;
        }

        if (!(first.EndRow == 10)) {
            log.println(
                    "wrong RangeAddress is returned, expected EndRow=10 and got " +
                    first.EndRow);
            res = false;
        }

        tRes.tested("getRanges()", res);
    }
}
