/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XCellRangeFormula.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:43:35 $
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
