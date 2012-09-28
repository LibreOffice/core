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

import com.sun.star.sheet.XFormulaQuery;
import com.sun.star.sheet.XSheetCellRanges;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XCell;
import com.sun.star.uno.UnoRuntime;

public class _XFormulaQuery extends MultiMethodTest {

    public XFormulaQuery oObj;

    protected XSpreadsheet oSheet = null;
    private XCell mxCell;
    private int miQueryThisDependentRange = 1;
    private int miQueryThisPrecedentRange = 1;
    private int[] miExpectedDependentValues;
    private int[] miExpectedPrecedentValues;

    protected void before() {
        oSheet = (XSpreadsheet)tEnv.getObjRelation("SHEET");

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
        Object o = tEnv.getObjRelation("MAKEENTRYINCELL");
        if (o != null) {
            mxCell = (XCell)o;
        }
        else {
            try {
                mxCell = oSheet.getCellByPosition(15, 15);
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                throw new StatusException(
                            Status.failed("Couldn't get initial cell"));
            }
        }

        o = tEnv.getObjRelation("RANGEINDICES");
        if (o != null) {
            int[]index = (int[])o;
            miQueryThisDependentRange = index[0];
            miQueryThisPrecedentRange = index[1];
        }

        o = tEnv.getObjRelation("EXPECTEDDEPENDENTVALUES");
        if (o != null) {
            miExpectedDependentValues = (int[])o;
        }
        else {
            miExpectedDependentValues = new int[4];
            miExpectedDependentValues[0] = 15;
            miExpectedDependentValues[1] = 15;
            miExpectedDependentValues[2] = 15;
            miExpectedDependentValues[3] = 15;
        }

        o = tEnv.getObjRelation("EXPECTEDPRECEDENTVALUES");
        if (o != null) {
            miExpectedPrecedentValues = (int[])o;
        }
        else {
            miExpectedPrecedentValues = new int[4];
            miExpectedPrecedentValues[0] = 0;
            miExpectedPrecedentValues[1] = 0;
            miExpectedPrecedentValues[2] = 15;
            miExpectedPrecedentValues[3] = 15;
        }
    }

    public void _queryDependents() {
        boolean res = true;

        try {
            mxCell.setFormula("=sum(A1:D1)");
            oSheet.getCellByPosition(0, 0).setValue(1);
            oSheet.getCellByPosition(1, 0).setValue(1);
            oSheet.getCellByPosition(2, 0).setValue(1);
            oSheet.getCellByPosition(3, 0).setValue(1);

            log.println(
                    "calling oObj.queryDependents(false)");
            XSheetCellRanges getting = oObj.queryDependents(false);
            CellRangeAddress[] range = getting.getRangeAddresses();

            res = ((range[miQueryThisDependentRange].StartColumn==miExpectedDependentValues[0]) &&
                    (range[miQueryThisDependentRange].EndColumn==miExpectedDependentValues[1]) &&
                   (range[miQueryThisDependentRange].StartRow==miExpectedDependentValues[2]) &&
                   (range[miQueryThisDependentRange].EndRow==miExpectedDependentValues[3]));

            if (!res) {
                log.println("Getting ("
                        +(range[miQueryThisDependentRange]).StartColumn+","
                        +(range[miQueryThisDependentRange]).EndColumn+","
                        +(range[miQueryThisDependentRange]).StartRow+","
                        +(range[miQueryThisDependentRange]).EndRow+")");
                log.println("Expected (" + miExpectedDependentValues[0] + "," +
                                           miExpectedDependentValues[1] + "," +
                                           miExpectedDependentValues[2] + "," +
                                           miExpectedDependentValues[3] + ")");
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
            mxCell.setFormula("=sum(A1:D1)");
            oSheet.getCellByPosition(0, 0).setValue(1);
            oSheet.getCellByPosition(1, 0).setValue(1);
            oSheet.getCellByPosition(2, 0).setValue(1);
            oSheet.getCellByPosition(3, 0).setValue(1);
            oSheet.getCellByPosition(1, 2).setFormula("=A16*2");

            log.println(
                    "calling oObj.queryPrecedents(false)");
            XSheetCellRanges getting = oObj.queryPrecedents(false);
            CellRangeAddress[] range = getting.getRangeAddresses();

            res = ((range[miQueryThisPrecedentRange].StartColumn==miExpectedPrecedentValues[0]) &&
                    (range[miQueryThisPrecedentRange].EndColumn==miExpectedPrecedentValues[1]) &&
                   (range[miQueryThisPrecedentRange].StartRow==miExpectedPrecedentValues[2]) &&
                   (range[miQueryThisPrecedentRange].EndRow==miExpectedPrecedentValues[3]));

            if (!res) {
                log.println("Getting ("
                        +(range[miQueryThisPrecedentRange]).StartColumn+","
                        +(range[miQueryThisPrecedentRange]).EndColumn+","
                        +(range[miQueryThisPrecedentRange]).StartRow+","
                        +(range[miQueryThisPrecedentRange]).EndRow+")");
                log.println("Expected (" + miExpectedPrecedentValues[0] + "," +
                                           miExpectedPrecedentValues[1] + "," +
                                           miExpectedPrecedentValues[2] + "," +
                                           miExpectedPrecedentValues[3] + ")");
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
