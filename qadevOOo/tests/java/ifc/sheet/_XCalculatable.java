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

import com.sun.star.sheet.XCalculatable;
import com.sun.star.table.XCell;
import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

/**
 *
 */
public class _XCalculatable extends MultiMethodTest {
    public XCalculatable oObj = null;
    private boolean bIsAutomaticCalculationEnabled = false;
    private XCell[] xCells = null;

    /**
     * Get object relation: four cells with values and formulas.
     * @see mod._sc.ScModelObj
     */
    @Override
    public void before() {
        xCells = (XCell[])tEnv.getObjRelation("XCalculatable.Cells");
        if (xCells == null || xCells.length != 3)
            throw new StatusException(Status.failed("Couldn't find correct object relation 'XCalculatable.Cells'"));

    }

    /**
     * Restore begin setting
     */
    @Override
    public void after() {
        // reset to begin value
        oObj.enableAutomaticCalculation(bIsAutomaticCalculationEnabled);
    }


    public void _calculate() {
        requiredMethod("isAutomaticCalculationEnabled()");
        boolean result = true;
        double ergValue1 = xCells[2].getValue();
        double sourceValue1 = xCells[0].getValue();
        xCells[0].setValue(sourceValue1 +1);
        double ergValue2 = xCells[2].getValue();
        result &= ergValue1 == ergValue2;
        oObj.calculate();
        ergValue2 = xCells[2].getValue();
        result &= ergValue1 != ergValue2;
        tRes.tested("calculate()", result);
    }

    public void _calculateAll() {
        requiredMethod("isAutomaticCalculationEnabled()");
        boolean result = true;
        double ergValue1 = xCells[2].getValue();
        double sourceValue1 = xCells[0].getValue();
        xCells[0].setValue(sourceValue1 +1);
        double ergValue2 = xCells[2].getValue();
        result &= ergValue1 == ergValue2;
        oObj.calculateAll();
        ergValue2 = xCells[2].getValue();
        result &= ergValue1 != ergValue2;
        oObj.calculateAll();
        tRes.tested("calculateAll()", result);
    }

    public void _enableAutomaticCalculation() {
        bIsAutomaticCalculationEnabled = oObj.isAutomaticCalculationEnabled();
        oObj.enableAutomaticCalculation(!bIsAutomaticCalculationEnabled);
        tRes.tested("enableAutomaticCalculation()", true);
    }

    public void _isAutomaticCalculationEnabled() {
        requiredMethod("enableAutomaticCalculation()");
        boolean result = oObj.isAutomaticCalculationEnabled();
        oObj.enableAutomaticCalculation(false);
        tRes.tested("isAutomaticCalculationEnabled()", result != bIsAutomaticCalculationEnabled);
    }

}
