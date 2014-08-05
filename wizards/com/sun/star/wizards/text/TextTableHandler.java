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
package com.sun.star.wizards.text;

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNamed;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.Locale;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.style.BreakType;
import com.sun.star.table.XCellRange;
import com.sun.star.text.XSimpleText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextTable;
import com.sun.star.text.XTextTablesSupplier;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XNumberFormatsSupplier;
import com.sun.star.view.XSelectionSupplier;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.NumberFormatter;

public class TextTableHandler
{

    public XTextTablesSupplier xTextTablesSupplier;
    public XMultiServiceFactory xMSFDoc;
    public XTextDocument xTextDocument;
    public XSimpleText xSimpleText;
    private NumberFormatter oNumberFormatter;
    private Locale aCharLocale;

    /** Creates a new instance of TextTableHandler */
    public TextTableHandler(XMultiServiceFactory xMSF, XTextDocument xTextDocument)
    {
        try
        {
            this.xMSFDoc = xMSF;
            this.xTextDocument = xTextDocument;
            xTextTablesSupplier = UnoRuntime.queryInterface(XTextTablesSupplier.class, xTextDocument);
            xSimpleText = UnoRuntime.queryInterface(XSimpleText.class, xTextDocument.getText());
            XNumberFormatsSupplier xNumberFormatsSupplier = UnoRuntime.queryInterface(XNumberFormatsSupplier.class, xTextDocument);
            aCharLocale = (Locale) Helper.getUnoStructValue(xTextDocument, "CharLocale");
            oNumberFormatter = new NumberFormatter(xNumberFormatsSupplier, aCharLocale);
        }
        catch (java.lang.Exception e)
        {
            e.printStackTrace(System.err);
        }
    }

    public NumberFormatter getNumberFormatter()
    {
        return oNumberFormatter;
    }

    public XTextTable getByName(String _sTableName)
    {
        XTextTable xTextTable = null;
        try
        {
            XNameAccess xAllTextTables = xTextTablesSupplier.getTextTables();
            if (xAllTextTables.hasByName(_sTableName))
            {
                Object oTable = xAllTextTables.getByName(_sTableName);
                xTextTable = UnoRuntime.queryInterface(XTextTable.class, oTable);
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
        return xTextTable;
    }

    public com.sun.star.text.XTextTable getlastTextTable()
    {
        try
        {
            XIndexAccess xAllTextTables = UnoRuntime.queryInterface(XIndexAccess.class, xTextTablesSupplier.getTextTables());
            int MaxIndex = xAllTextTables.getCount() - 1;
            Object oTable = xAllTextTables.getByIndex(MaxIndex);
            return UnoRuntime.queryInterface(XTextTable.class, oTable);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
            return null;
        }
    }

    public void insertTextTable(com.sun.star.text.XTextCursor xTextCursor)
    {
        try
        {
            com.sun.star.uno.XInterface xTextTable = (XInterface) xMSFDoc.createInstance("com.sun.star.text.TextTable");
            XTextContent xTextContentTable = UnoRuntime.queryInterface(XTextContent.class, xTextTable);
            if (xTextCursor == null)
            {
                xTextCursor = xTextDocument.getText().createTextCursor();
                xTextCursor.gotoEnd(false);
            }
            xTextCursor.getText().insertTextContent(xTextCursor, xTextContentTable, false);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public void removeAllTextTables()
    {
        try
        {
            XIndexAccess xAllTextTables = UnoRuntime.queryInterface(XIndexAccess.class, xTextTablesSupplier.getTextTables());
            int TextTableCount = xAllTextTables.getCount();
            for (int i = TextTableCount - 1; i >= 0; i--)
            {
                removeTextTable(xAllTextTables.getByIndex(i));
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public void removeLastTextTable()
    {
        try
        {
            XIndexAccess xAllTextTables = UnoRuntime.queryInterface(XIndexAccess.class, xTextTablesSupplier.getTextTables());
            Object oTextTable = xAllTextTables.getByIndex(xAllTextTables.getCount() - 1);
            removeTextTable(oTextTable);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public void removeTextTable(Object oTextTable)
    {
        try
        {
            XTextContent xTextContentTable = UnoRuntime.queryInterface(XTextContent.class, oTextTable);
            xTextDocument.getText().removeTextContent(xTextContentTable);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public void removeTextTablebyName(String TableName)
    {
        try
        {
            XNameAccess xAllTextTables = xTextTablesSupplier.getTextTables();
            if (xAllTextTables.hasByName(TableName))
            {
                removeTextTable(xAllTextTables.getByName(TableName));
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public void renameTextTable(String OldTableName, String NewTableName)
    {
        try
        {
            XNameAccess xTextTableNames = xTextTablesSupplier.getTextTables();
            if (xTextTableNames.hasByName(OldTableName))
            {
                Object oTextTable = xTextTableNames.getByName(OldTableName);
                XNamed xTextTableName = UnoRuntime.queryInterface(XNamed.class, oTextTable);
                xTextTableName.setName(NewTableName);
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public static BreakType resetBreakTypeofTextTable(Object oTextTable)
    {
        BreakType BreakValue = (BreakType) com.sun.star.wizards.common.Helper.getUnoStructValue(oTextTable, "BreakType");
        Helper.setUnoPropertyValue(oTextTable, "BreakType", BreakType.NONE);
        return BreakType.NONE;
    }

    public void adjustOptimalTableWidths(XMultiServiceFactory _xMSF, XTextTable xTextTable)
    {
        try
        {
            XFrame xFrame = this.xTextDocument.getCurrentController().getFrame();
            int ColCount = xTextTable.getColumns().getCount();
            XCellRange xCellRange = UnoRuntime.queryInterface(XCellRange.class, xTextTable);
            XCellRange xLocCellRange = xCellRange.getCellRangeByPosition(0, 0, ColCount - 1, 1);
            short iHoriOrient = AnyConverter.toShort(Helper.getUnoPropertyValue(xTextTable, "HoriOrient"));
            XSelectionSupplier xSelection = UnoRuntime.queryInterface(XSelectionSupplier.class, xTextDocument.getCurrentController());
            xSelection.select(xLocCellRange);
            Desktop.dispatchURL(_xMSF, ".Uno:DistributeColumns", xFrame);
            Desktop.dispatchURL(_xMSF, ".Uno:SetOptimalColumnWidth", xFrame);
            Helper.setUnoPropertyValue(xTextTable, "HoriOrient", new Short(iHoriOrient));
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }
}
