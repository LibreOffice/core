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
