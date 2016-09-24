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

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.sheet.SubTotalColumn;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSubTotalCalculatable;
import com.sun.star.sheet.XSubTotalDescriptor;
import com.sun.star.uno.UnoRuntime;


public class _XSubTotalCalculatable extends MultiMethodTest {
    public XSubTotalCalculatable oObj;
    protected XSubTotalDescriptor desc;
    protected XSpreadsheet oSheet;

    @Override
    protected void before() {
        oSheet = (XSpreadsheet) tEnv.getObjRelation("SHEET");

        if (oSheet == null) {
            log.println("Object relation oSheet is missing");
            log.println("Trying to query the needed Interface");
            oSheet = UnoRuntime.queryInterface(
                             XSpreadsheet.class, tEnv.getTestObject());

            if (oSheet == null) {
                throw new StatusException(Status.failed(
                                                  "Object relation oSheet is missing"));
            }
        }
    }

    public void _applySubTotals() {
        requiredMethod("createSubTotalDescriptor()");

        boolean res = true;

        try {
            oSheet.getCellByPosition(0, 0).setFormula("first");
            oSheet.getCellByPosition(1, 0).setFormula("second");
            oSheet.getCellByPosition(0, 3).setFormula("");
            oSheet.getCellByPosition(0, 1).setValue(5);
            oSheet.getCellByPosition(0, 2).setValue(5);
            oSheet.getCellByPosition(1, 1).setValue(17);
            oSheet.getCellByPosition(1, 2).setValue(25);
            oObj.applySubTotals(desc, true);

            String formula = oSheet.getCellByPosition(0, 3).getFormula();
            String expected = "=SUBTOTAL(9;$A$2:$A$3)";
            res = formula.equals(expected);

            if (!res) {
                log.println("getting: " + formula);
                log.println("expected: " + expected);
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Couldn't fill cells" + e.getLocalizedMessage());
            res = false;
        }

        tRes.tested("applySubTotals()", res);
    }

    public void _createSubTotalDescriptor() {
        desc = oObj.createSubTotalDescriptor(true);

        SubTotalColumn[] columns = new SubTotalColumn[1];
        columns[0] = new SubTotalColumn();
        columns[0].Column = 0;
        columns[0].Function = com.sun.star.sheet.GeneralFunction.SUM;
        desc.addNew(columns, 0);
        tRes.tested("createSubTotalDescriptor()", true);
    }

    public void _removeSubTotals() {
        requiredMethod("applySubTotals()");

        boolean res = true;

        try {
            oObj.removeSubTotals();

            String formula = oSheet.getCellByPosition(0, 3).getFormula();
            String expected = "";
            res = formula.equals(expected);

            if (!res) {
                log.println("getting: " + formula);
                log.println("expected: " + expected);
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Couldn't get cell" + e.getLocalizedMessage());
        }

        tRes.tested("removeSubTotals()", res);
    }
}