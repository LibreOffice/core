/*************************************************************************
 *
 *  $RCSfile: _XGoalSeek.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2004-11-02 11:56:44 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
