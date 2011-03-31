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
package com.sun.star.wizards.report;

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNamed;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.table.XCellRange;
import com.sun.star.table.XTableColumns;
import com.sun.star.table.XTableRows;
import com.sun.star.text.XTextTable;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.text.TextTableHandler;
import com.sun.star.wizards.text.ViewHandler;

public class RecordTable
{

    String CurFieldName;
    String LabelDescription;
    public XNamed xTableName;
    public XCellRange xCellRange;
    public XTextTable xTextTable;
    private TextTableHandler oTextTableHandler;
    public XTableColumns xTableColumns;
    public XTableRows xTableRows;

    public RecordTable(TextTableHandler _oTextTableHandler)
    {
        try
        {
            this.oTextTableHandler = _oTextTableHandler;
            String[] TableNames = oTextTableHandler.xTextTablesSupplier.getTextTables().getElementNames();
            XNameAccess xAllTextTables = oTextTableHandler.xTextTablesSupplier.getTextTables();
            if ((xAllTextTables.hasByName(ReportTextDocument.TBLRECORDSECTION)) || (xAllTextTables.hasByName(ReportTextDocument.COPYOFTBLRECORDSECTION)))
            {
                Object oTable;
                if (xAllTextTables.hasByName(ReportTextDocument.COPYOFTBLRECORDSECTION))
                {
                    oTable = xAllTextTables.getByName(ReportTextDocument.COPYOFTBLRECORDSECTION);
                }
                else
                {
                    oTable = xAllTextTables.getByName(ReportTextDocument.TBLRECORDSECTION);
                }
                xTextTable = UnoRuntime.queryInterface(XTextTable.class, oTable);
                xTableName = UnoRuntime.queryInterface(XNamed.class, xTextTable);
            }
            else
            {
                XIndexAccess xTableIndex = UnoRuntime.queryInterface(XIndexAccess.class, xAllTextTables);
                int n = xTableIndex.getCount() - 1;
                Object x = xTableIndex.getByIndex(n);
                xTextTable = UnoRuntime.queryInterface(XTextTable.class, x);
                xTableName = UnoRuntime.queryInterface(XNamed.class, xTextTable);
                xTableName.setName(ReportTextDocument.TBLRECORDSECTION);
            }
            xTableRows = xTextTable.getRows();
            xTableColumns = xTextTable.getColumns();
            xCellRange = UnoRuntime.queryInterface(XCellRange.class, xTextTable);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    public void adjustOptimalTableWidths(XMultiServiceFactory _xMSF, ViewHandler oViewHandler)
    {     // setTableColumnSeparators(){
        oTextTableHandler.adjustOptimalTableWidths(_xMSF, xTextTable);
        oViewHandler.collapseViewCursorToStart();
    }
}
