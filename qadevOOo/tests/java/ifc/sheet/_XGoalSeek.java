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

import com.sun.star.container.XIndexAccess;
import com.sun.star.sheet.GoalResult;
import com.sun.star.sheet.XGoalSeek;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.CellAddress;
import com.sun.star.uno.UnoRuntime;
import lib.MultiMethodTest;
import lib.StatusException;

/**
 *
 */
public class _XGoalSeek extends MultiMethodTest {
    public XGoalSeek oObj = null;
    XSpreadsheet xSheet = null;
    CellAddress aFormula = null;
    CellAddress aValue = null;

    @Override
    public void before() {
        Exception ex = null;
        // get two sheets
        try {
            XSpreadsheetDocument xSpreadsheetDocument = UnoRuntime.queryInterface(XSpreadsheetDocument.class, oObj);
            XSpreadsheets oSheets = xSpreadsheetDocument.getSheets();
            XIndexAccess oIndexSheets = UnoRuntime.queryInterface(
                                                XIndexAccess.class, oSheets);
            xSheet = UnoRuntime.queryInterface(
                                      XSpreadsheet.class, oIndexSheets.getByIndex(1));
        }
        catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            ex = e;
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            ex = e;
        }
        catch(NullPointerException e) {
            ex = e;
        }
        if (ex != null) {
            throw new StatusException("Could not get a sheet.", ex);
        }

        // set value and formula
        try {
            xSheet.getCellByPosition(3, 4).setValue(9);
            xSheet.getCellByPosition(3, 5).setFormula("= SQRT(D5)");
            aValue = new CellAddress((short)1, 3, 4);
            aFormula = new CellAddress((short)1, 3, 5);
        }
        catch(Exception e) {
            throw new StatusException("Could not get set formulas on the sheet.", e);
        }
    }

    public void _seekGoal() {
        boolean result = true;
        double divergence = 0.01;
        GoalResult goal = oObj.seekGoal(aFormula, aValue, "4");
        log.println("Goal Result: " + goal.Result + "   Divergence: " + goal.Divergence);
        result &= goal.Divergence < divergence;
        result &= goal.Result > 16 - divergence || goal.Result < 16 + divergence;

        goal = oObj.seekGoal(aFormula, aValue, "-4");
        log.println("Goal Result: " + goal.Result + "   Divergence: " + goal.Divergence);
        result &= goal.Divergence > 1/divergence;
        result &= goal.Result < divergence || goal.Result > -divergence;

        // just curious: let goal seek find a limiting value
        try {
            xSheet.getCellByPosition(3, 4).setValue(0.8);
            xSheet.getCellByPosition(3, 5).setFormula("= (D5 ^ 2 - 1) / (D5 - 1)");
        }
        catch(Exception e) {
            System.out.println("caught exception: " + e);
        }
        goal = oObj.seekGoal(aFormula, aValue, "2");
        log.println("Goal Result: " + goal.Result + "   Divergence: " + goal.Divergence);
        result &= goal.Divergence < divergence;
        result &= goal.Result > 16 - divergence || goal.Result < 16 + divergence;

        tRes.tested("seekGoal()", result);
    }
}
