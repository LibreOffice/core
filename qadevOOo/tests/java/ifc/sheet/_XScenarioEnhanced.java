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
package ifc.sheet;

import com.sun.star.sheet.XScenarioEnhanced;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.uno.UnoRuntime;

import lib.MultiMethodTest;


/**
 *
 * @author  sw93809
 */
public class _XScenarioEnhanced extends MultiMethodTest {
    public XScenarioEnhanced oObj = null;

    public void before() {
        oObj = (XScenarioEnhanced) UnoRuntime.queryInterface(
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