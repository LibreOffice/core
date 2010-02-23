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

    protected void before() {
        oSheet = (XSpreadsheet) tEnv.getObjRelation("SHEET");

        if (oSheet == null) {
            log.println("Object relation oSheet is missing");
            log.println("Trying to query the needed Interface");
            oSheet = (XSpreadsheet) UnoRuntime.queryInterface(
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