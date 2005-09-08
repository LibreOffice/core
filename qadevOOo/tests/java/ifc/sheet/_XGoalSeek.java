/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XGoalSeek.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:48:46 $
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

    public void before() {
        Exception ex = null;
        // get two sheets
        try {
            XSpreadsheetDocument xSpreadsheetDocument = (XSpreadsheetDocument)
                    UnoRuntime.queryInterface(XSpreadsheetDocument.class, oObj);
            XSpreadsheets oSheets = xSpreadsheetDocument.getSheets();
            XIndexAccess oIndexSheets = (XIndexAccess) UnoRuntime.queryInterface(
                                                XIndexAccess.class, oSheets);
            xSheet = (XSpreadsheet) UnoRuntime.queryInterface(
                                      XSpreadsheet.class, oIndexSheets.getByIndex(1));
        }
        catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            ex = e;
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            ex = e;
        }
        catch(java.lang.NullPointerException e) {
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
        catch(Exception e) {}
        goal = oObj.seekGoal(aFormula, aValue, "2");
        log.println("Goal Result: " + goal.Result + "   Divergence: " + goal.Divergence);
        result &= goal.Divergence < divergence;
        result &= goal.Result > 16 - divergence || goal.Result < 16 + divergence;

        tRes.tested("seekGoal()", result);
    }
}
