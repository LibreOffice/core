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
import com.sun.star.text.XText;
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
    private XText xText;
    private NumberFormatter oNumberFormatter;
    private Locale aCharLocale;

    /** Creates a new instance of TextTableHandler */
    public TextTableHandler(XMultiServiceFactory xMSF, XTextDocument xTextDocument)
    {
        try
        {
            this.xMSFDoc = xMSF;
            this.xTextDocument = xTextDocument;
            xText = xTextDocument.getText();
            xTextTablesSupplier = UnoRuntime.queryInterface(XTextTablesSupplier.class, xTextDocument);
            xSimpleText = UnoRuntime.queryInterface(XSimpleText.class, xTextDocument.getText());
            XNumberFormatsSupplier xNumberFormatsSupplier = UnoRuntime.queryInterface(XNumberFormatsSupplier.class, xTextDocument);
            aCharLocale = (Locale) Helper.getUnoStructValue((Object) xTextDocument, "CharLocale");
            oNumberFormatter = new NumberFormatter(xNumberFormatsSupplier, aCharLocale);
        }
        catch (java.lang.Exception e)
        {
            e.printStackTrace(System.out);
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
            exception.printStackTrace(System.out);
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
            exception.printStackTrace(System.out);
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
            exception.printStackTrace(System.out);
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
            exception.printStackTrace(System.out);
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
            exception.printStackTrace(System.out);
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
            exception.printStackTrace(System.out);
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
            exception.printStackTrace(System.out);
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
            exception.printStackTrace(System.out);
        }
    }

    public static BreakType resetBreakTypeofTextTable(Object oTextTable)
    {
        BreakType CorrBreakValue = null;
        BreakType BreakValue = (BreakType) com.sun.star.wizards.common.Helper.getUnoStructValue(oTextTable, "BreakType");
        //  if (BreakValue.equals(BreakType.NONE) == false){
        //      CorrBreakValue = BreakValue;
        Helper.setUnoPropertyValue(oTextTable, "BreakType", BreakType.NONE);
        //  }
        return BreakType.NONE;
    }

    public void adjustOptimalTableWidths(XMultiServiceFactory _xMSF, XTextTable xTextTable)
    {        // setTableColumnSeparators(){
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
            exception.printStackTrace(System.out);
        }
    }
}
