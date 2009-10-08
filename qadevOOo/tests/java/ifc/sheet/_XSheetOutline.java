/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XSheetOutline.java,v $
 * $Revision: 1.4 $
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

import com.sun.star.sheet.XCellRangesQuery;
import com.sun.star.sheet.XSheetCellRanges;
import lib.MultiMethodTest;

import com.sun.star.sheet.XSheetOutline;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.TableOrientation;
import com.sun.star.uno.UnoRuntime;
import lib.Status;
import lib.StatusException;

/**
 *
 */
public class _XSheetOutline extends MultiMethodTest {
    public XSheetOutline oObj = null;
    CellRangeAddress address = null;
    CellRangeAddress subaddress = null;

    public void before() {
        address = (CellRangeAddress)tEnv.getObjRelation("CellRangeAddress");
        subaddress = (CellRangeAddress)tEnv.getObjRelation("CellRangeSubAddress");
        if (address == null)
            throw new StatusException(Status.failed("Object relation CellRangeAddress not found"));
        if (subaddress == null)
            throw new StatusException(Status.failed("Object relation CellRangeSubAddress not found"));
    }

    public void _autoOutline() {
        executeMethod("ungroup()");
        boolean result = false;
        oObj.autoOutline(address);
        // initially the range is grouped and shown
        result = isCellShown(subaddress);
        oObj.hideDetail(address);
        // here only a part of the address is hidden: subaddress must be that part
        result &= !isCellShown(subaddress);
        tRes.tested("autoOutline()", result);
    }

    public void _clearOutline() {
        executeMethod("autoOutline()");
        boolean result = false;
        oObj.clearOutline();
        result = isCellShown(subaddress);
        oObj.hideDetail(address);
        result &= isCellShown(subaddress);
        tRes.tested("clearOutline()", result);
    }

    public void _group() {
        oObj.group(address, TableOrientation.COLUMNS);
        oObj.group(address, TableOrientation.ROWS);
        tRes.tested("group()", true);
    }

    public void _ungroup() {
        executeMethod("showDetail()");
        oObj.ungroup(address, TableOrientation.COLUMNS);
        oObj.ungroup(address, TableOrientation.ROWS);
        oObj.hideDetail(address);
        tRes.tested("ungroup()", isCellShown(address));
    }

    public void _hideDetail() {
        requiredMethod("group()");
        oObj.hideDetail(address);
        tRes.tested("hideDetail()", !isCellShown(address));
    }

    public void _showDetail() {
        executeMethod("showLevel()");
        oObj.showDetail(address);
        tRes.tested("showDetail()", isCellShown(address));
    }

    public void _showLevel() {
        executeMethod("hideDetail()");
        boolean result = false;
        oObj.showLevel((short)2, TableOrientation.COLUMNS);
        oObj.showLevel((short)2, TableOrientation.ROWS);
        result = isCellShown(address);
        oObj.showLevel((short)0, TableOrientation.COLUMNS);
        oObj.showLevel((short)0, TableOrientation.ROWS);

        result &= !isCellShown(address);
        tRes.tested("showLevel()", result);
    }

    private boolean isCellShown(CellRangeAddress range) {
        boolean isNotShown = true;
        XCellRangesQuery xCellRangesQuery = (XCellRangesQuery)UnoRuntime.queryInterface(XCellRangesQuery.class, oObj);
        if (xCellRangesQuery != null) {
            XSheetCellRanges xRanges = xCellRangesQuery.queryVisibleCells();
            CellRangeAddress[] visibleRanges = xRanges.getRangeAddresses();
            for (int i=0; i<visibleRanges.length; i++) {
                isNotShown &= dotIsOutsideRange(range.StartRow, range.StartColumn, visibleRanges[i]);
                isNotShown &= dotIsOutsideRange(range.StartRow, range.EndColumn, visibleRanges[i]);
                isNotShown &= dotIsOutsideRange(range.EndRow, range.StartColumn, visibleRanges[i]);
                isNotShown &= dotIsOutsideRange(range.EndRow, range.EndColumn, visibleRanges[i]);
                log.println(isNotShown?"\tisOutSide":"\tisInside");
            }
        }
        return !isNotShown;
    }

    private boolean dotIsOutsideRange(int dotRow, int dotColumn, CellRangeAddress range) {
        log.println("Checking dot(" + dotRow + "," + dotColumn + ") against row["
                    + range.StartRow + ":" + range.EndRow + "]  column["
                    + range.StartColumn + ":" + range.EndColumn + "]");
        boolean isInside = true;
        if (dotRow >= range.StartRow && dotRow <= range.EndRow)
            if (dotColumn >= range.StartColumn && dotColumn <= range.EndColumn)
                isInside = false;
        return isInside;
    }
}
