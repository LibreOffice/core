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

import com.sun.star.sheet.XCellRangeFormula;

public class _XCellRangeFormula extends lib.MultiMethodTest {

    public XCellRangeFormula oObj = null;

    public String[][] orgValue = null;

    public void _getFormulaArray() {
        Object noArray = tEnv.getObjRelation("noArray");
        if (noArray != null) {
            log.println("Component " + noArray.toString() +
                " doesn't really support this Interface");
            log.println("It doesn't make sense to get an FormulaArray from"
                + " the whole sheet");
            tRes.tested("getFormulaArray()", true);
            return;
        }
        orgValue = oObj.getFormulaArray();
        String[][] newValue = oObj.getFormulaArray();
        newValue[0][0] = "inserted";
        oObj.setFormulaArray(newValue);
        boolean res = oObj.getFormulaArray()[0][0].equals(newValue[0][0]);
        tRes.tested("getFormulaArray()",res);
    }

    public void _setFormulaArray() {
        requiredMethod("getFormulaArray()");
        Object noArray = tEnv.getObjRelation("noArray");
        if (noArray != null) {
            log.println("Component " + noArray.toString() +
                " doesn't really support this Interface");
            log.println("It doesn't make sense to set an FormulaArray over"
                + " the whole sheet");
            tRes.tested("setFormulaArray()", true);
            return;
        }
        oObj.setFormulaArray(orgValue);
        boolean res = oObj.getFormulaArray()[0][0].equals(orgValue[0][0]);
        tRes.tested("setFormulaArray()",res);
    }
}
