/*************************************************************************
 *
 *  $RCSfile: _XPrintAreas.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2004-11-02 11:56:56 $
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
