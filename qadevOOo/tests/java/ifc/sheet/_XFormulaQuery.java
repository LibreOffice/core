/*************************************************************************
 *
 *  $RCSfile: _XFormulaQuery.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:01:07 $
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

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.sheet.XFormulaQuery;
import com.sun.star.sheet.XSheetCellRanges;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.uno.UnoRuntime;

/**
 *
 * @author  sw93809
 */
public class _XFormulaQuery extends MultiMethodTest {

    public XFormulaQuery oObj;

     protected XSpreadsheet oSheet = null;

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

    public void _queryDependents() {
        boolean res = true;

        try {
            oSheet.getCellByPosition(15, 15).setFormula("=sum(A1:D1)");
            oSheet.getCellByPosition(0, 0).setValue(1);
            oSheet.getCellByPosition(1, 0).setValue(1);
            oSheet.getCellByPosition(2, 0).setValue(1);
            oSheet.getCellByPosition(3, 0).setValue(1);

            log.println(
                    "calling oObj.queryDependents(false)");
            XSheetCellRanges getting = oObj.queryDependents(false);

            res = ((getting.getRangeAddresses()[1].StartColumn==15) && (getting.getRangeAddresses()[1].EndColumn==15) && (getting.getRangeAddresses()[1].StartRow==15) && (getting.getRangeAddresses()[1].EndRow==15));

            if (!res) {
            log.println("Getting ("+(getting.getRangeAddresses()[1]).StartColumn
            +","+(getting.getRangeAddresses()[1]).EndColumn
            +","+(getting.getRangeAddresses()[1]).StartRow
            +","+(getting.getRangeAddresses()[1]).EndRow+")"                       );
            log.println("Expected (15,15,15,15)");
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Couldn't set initial version to cell");
            res = false;
        }

        tRes.tested("queryDependents()", res);
    }

    public void _queryPrecedents() {
        boolean res = true;

        try {
            oSheet.getCellByPosition(0, 15).setFormula("=sum(A1:D1)");
            oSheet.getCellByPosition(0, 0).setValue(1);
            oSheet.getCellByPosition(1, 0).setValue(1);
            oSheet.getCellByPosition(2, 0).setValue(1);
            oSheet.getCellByPosition(3, 0).setValue(1);
            oSheet.getCellByPosition(1, 2).setFormula("=A16*2");

            log.println(
                    "calling oObj.queryPrecedents(false)");
            XSheetCellRanges getting = oObj.queryPrecedents(false);

            res = ((getting.getRangeAddresses()[1].StartColumn==0) && (getting.getRangeAddresses()[1].EndColumn==0) && (getting.getRangeAddresses()[1].StartRow==15) && (getting.getRangeAddresses()[1].EndRow==15));

            if (!res) {
            log.println("Getting ("+(getting.getRangeAddresses()[1]).StartColumn
            +","+(getting.getRangeAddresses()[1]).EndColumn
            +","+(getting.getRangeAddresses()[1]).StartRow
            +","+(getting.getRangeAddresses()[1]).EndRow+")"                       );
            log.println("Expected (0,0,15,15)");
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Couldn't set initial version to cell");
            res = false;
        }

        tRes.tested("queryPrecedents()", res);
    }

    /**
    * Forces environment recreation.
    */
    protected void after() {
        disposeEnvironment();
    }

}
