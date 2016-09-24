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

import com.sun.star.sheet.XPrintAreas;
import com.sun.star.table.CellRangeAddress;
import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;
import util.ValueComparer;

/**
 *
 */
public class _XPrintAreas extends MultiMethodTest {
    public XPrintAreas oObj = null;
    CellRangeAddress address = null;
    CellRangeAddress subaddress = null;
    CellRangeAddress titleColumns;
    CellRangeAddress titleRows;

    @Override
    public void before() {
        address = (CellRangeAddress)tEnv.getObjRelation("CellRangeAddress");
        subaddress = (CellRangeAddress)tEnv.getObjRelation("CellRangeSubAddress");
        if (address == null)
            throw new StatusException(Status.failed("Object relation CellRangeAddress not found"));
        if (subaddress == null)
            throw new StatusException(Status.failed("Object relation CellRangeSubAddress not found"));
    }

    public void _getPrintAreas() {
        requiredMethod("getPrintTitleColumns()");
        requiredMethod("getPrintTitleRows()");
        executeMethod("getTitleColumns()");
        executeMethod("getTitleRows()");
        CellRangeAddress[] printAreas = oObj.getPrintAreas();
        CellRangeAddress[] setValue = new CellRangeAddress[]{address};
        boolean ret = ValueComparer.equalValue(printAreas, setValue);
        // delete the print area
        oObj.setPrintAreas(null);
        printAreas = oObj.getPrintAreas();
        ret &= printAreas.length == 0;

        tRes.tested("getPrintAreas()", ret);
    }

    public void _getPrintTitleColumns() {
        requiredMethod("setPrintTitleColumns()");
        tRes.tested("getPrintTitleColumns()", !oObj.getPrintTitleColumns());
    }

    public void _getPrintTitleRows() {
        requiredMethod("setPrintTitleRows()");
        tRes.tested("getPrintTitleRows()", !oObj.getPrintTitleRows());
    }

    public void _getTitleColumns() {
        requiredMethod("setTitleColumns()");
        CellRangeAddress setValue = oObj.getTitleColumns();
        tRes.tested("getTitleColumns()", ValueComparer.equalValue(setValue,titleColumns));
    }

    public void _getTitleRows() {
        requiredMethod("setTitleRows()");
        CellRangeAddress setValue = oObj.getTitleRows();
        tRes.tested("getTitleRows()", ValueComparer.equalValue(setValue,titleRows));
    }

    public void _setPrintAreas() {
        boolean ret = false;
        CellRangeAddress[]setValue = new CellRangeAddress[]{subaddress};
        oObj.setPrintAreas(setValue);
        CellRangeAddress[]newVal = oObj.getPrintAreas();
        ret = ValueComparer.equalValue(newVal, setValue);
        setValue = new CellRangeAddress[]{address};
        oObj.setPrintAreas(setValue);
        newVal = oObj.getPrintAreas();
        ret &= ValueComparer.equalValue(newVal, setValue);
        tRes.tested("setPrintAreas()", ret);
    }

    public void _setPrintTitleColumns() {
        requiredMethod("setTitleColumns()");
        boolean ret = false;
        boolean value = oObj.getPrintTitleColumns();
        oObj.setPrintTitleColumns(!value);
        ret = value != oObj.getPrintTitleColumns();
        oObj.setPrintTitleColumns(false);
        tRes.tested("setPrintTitleColumns()", ret);
    }

    public void _setPrintTitleRows() {
        requiredMethod("setTitleRows()");
        boolean ret = false;
        boolean value = oObj.getPrintTitleRows();
        oObj.setPrintTitleRows(!value);
        ret = value != oObj.getPrintTitleRows();
        oObj.setPrintTitleRows(false);
        tRes.tested("setPrintTitleRows()", ret);
    }

    public void _setTitleColumns() {
        requiredMethod("setPrintAreas()");
        boolean ret = false;
        CellRangeAddress newVal = oObj.getTitleColumns();
        ret = ValueComparer.equalValue(newVal, new CellRangeAddress((short)0, 0, 0, 0, 0));
        // use first row of range as title column
        titleColumns = new CellRangeAddress();
        titleColumns.Sheet = address.Sheet;
        titleColumns.StartColumn = address.StartColumn;
        titleColumns.StartRow = address.StartRow;
        titleColumns.EndColumn = address.EndColumn;
        titleColumns.EndRow = address.StartRow;
        oObj.setTitleColumns(titleColumns);
        tRes.tested("setTitleColumns()", ret);
    }

    public void _setTitleRows() {
        requiredMethod("setPrintAreas()");
        boolean ret = false;
        CellRangeAddress newVal = oObj.getTitleRows();
        ret = ValueComparer.equalValue(newVal, new CellRangeAddress((short)0, 0, 0, 0, 0));
        // use first column of range as title row
        titleRows = new CellRangeAddress();
        titleRows.Sheet = address.Sheet;
        titleRows.StartColumn = address.StartColumn;
        titleRows.StartRow = address.StartRow;
        titleRows.EndColumn = address.StartColumn;
        titleRows.EndRow = address.EndRow;
        oObj.setTitleColumns(titleRows);
        tRes.tested("setTitleRows()", ret);
    }

}
