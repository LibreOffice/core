/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XCalculatable.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:42:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
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
    public void before() {
        xCells = (XCell[])tEnv.getObjRelation("XCalculatable.Cells");
        if (xCells == null || xCells.length != 3)
            throw new StatusException(Status.failed("Couldn't find correct object relation 'XCalculatable.Cells'"));

    }

    /**
     * Restore begin setting
     */
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
