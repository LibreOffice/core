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
package com.sun.star.wizards.report;

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNamed;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.table.XCellRange;
import com.sun.star.table.XTableColumns;
import com.sun.star.text.XTextTable;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.text.TextTableHandler;
import com.sun.star.wizards.text.ViewHandler;

public class RecordTable
{

    XNamed xTableName;
    XCellRange xCellRange;
    public XTextTable xTextTable;
    private TextTableHandler oTextTableHandler;
    XTableColumns xTableColumns;

    public RecordTable(TextTableHandler _oTextTableHandler)
    {
        try
        {
            this.oTextTableHandler = _oTextTableHandler;
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
            xTextTable.getRows();
            xTableColumns = xTextTable.getColumns();
            xCellRange = UnoRuntime.queryInterface(XCellRange.class, xTextTable);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public void adjustOptimalTableWidths(XMultiServiceFactory _xMSF, ViewHandler oViewHandler)
    {     // setTableColumnSeparators(){
        oTextTableHandler.adjustOptimalTableWidths(_xMSF, xTextTable);
        oViewHandler.collapseViewCursorToStart();
    }
}
