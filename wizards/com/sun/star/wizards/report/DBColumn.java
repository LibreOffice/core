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

import com.sun.star.beans.PropertyState;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertyState;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNamed;
import com.sun.star.style.ParagraphAdjust;
import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;
import com.sun.star.table.XTableColumns;
import com.sun.star.table.XTableRows;
import com.sun.star.text.XDependentTextField;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextRange;
import com.sun.star.text.XTextTable;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.db.*;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.text.TextDocument;
import com.sun.star.wizards.text.TextTableHandler;
import com.sun.star.wizards.text.TextFieldHandler;

public class DBColumn
{

    private XCell xValCell;
    private XTextRange xValTextCell;
    private XTextCursor xValCellCursor;
    public XCell xNameCell;
    private XTextRange xNameTextCell;
    private boolean bAlignLeft;
    private String CharFontName;
    private PropertyState PropertyState;
    public int ValColumn = 1;
    private int ValRow = 0;
    public FieldColumn CurDBField;
    private XTextTable xTextTable;
    private XTableColumns xTableColumns;
    private XCellRange xCellRange;
    public XNamed xTableName;
    private boolean bIsGroupColumn;
    private RecordParser CurDBMetaData;
    private RecordTable CurRecordTable;
    private TextTableHandler oTextTableHandler;

    public DBColumn(RecordTable _CurRecordTable, TextTableHandler _oTextTableHandler, RecordParser _CurDBMetaData, int i, boolean _bforce)
    {
        initializeRecordTableMembers(_CurRecordTable, _oTextTableHandler, _CurDBMetaData, i, _bforce);
    }

    public DBColumn(RecordTable _CurRecordTable, TextTableHandler _oTextTableHandler, RecordParser _CurDBMetaData, int i)
    {
        initializeRecordTableMembers(_CurRecordTable, _oTextTableHandler, _CurDBMetaData, i, false);
    }

    private void initializeRecordTableMembers(RecordTable _CurRecordTable, TextTableHandler _oTextTableHandler, RecordParser _CurDBMetaData, int i, boolean _bForce)
    {
        this.oTextTableHandler = _oTextTableHandler;
        this.CurDBMetaData = _CurDBMetaData;
        this.CurRecordTable = _CurRecordTable;
        bIsGroupColumn = false;
        if (CurDBMetaData.RecordFieldColumns != null)
        {
            CurDBField = CurDBMetaData.getFieldColumnByFieldName(CurDBMetaData.RecordFieldColumns[i].getFieldName());
        }
        else
        {
            CurDBField = CurDBMetaData.getFieldColumnByFieldName(CurDBMetaData.getRecordFieldName(i));
        }
        if (_bForce)
        {
            assignCells(i, true);
        }
        else
        {
            for (int n = 0; n < CurRecordTable.xTableColumns.getCount(); n++)
            {
                assignCells(n, false);
            }
        }
    }

    private boolean assignCells(int _nColumn, boolean _bforce)
    {
        try
        {
            XCell xCell = CurRecordTable.xCellRange.getCellByPosition(_nColumn, 0);
            XTextRange xTextCell = UnoRuntime.queryInterface(XTextRange.class, xCell);
            String CompString = "Column";
            XTextCursor xLocCellCursor = TextDocument.createTextCursor(xCell);
            if (isNameCell(xLocCellCursor, CurDBField.getFieldName(), CompString) || (_bforce))
            {
                xNameCell = xCell;
                xNameTextCell = xTextCell;
                xValCell = CurRecordTable.xCellRange.getCellByPosition(_nColumn, 1);
                xValTextCell = UnoRuntime.queryInterface(XTextRange.class, xValCell);
                xValCellCursor = TextDocument.createTextCursor(xValCell);
                ValColumn = _nColumn;
                return true;
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
        return false;
    }

    public DBColumn(TextTableHandler _oTextTableHandler, RecordParser _CurDBMetaData, String _FieldName, String TableName, DBColumn OldDBColumn) {
        this.oTextTableHandler = _oTextTableHandler;
        this.CurDBMetaData = _CurDBMetaData;
        CurDBField = CurDBMetaData.getFieldColumnByDisplayName(_FieldName);
        bIsGroupColumn = true;
        getTableColumns(TableName);
        xNameCell = OldDBColumn.xNameCell;
        xNameTextCell = OldDBColumn.xNameTextCell;
        xValCell = OldDBColumn.xValCell;
        xValTextCell = OldDBColumn.xValTextCell;
        xValCellCursor = TextDocument.createTextCursor(xValCell);
        ValColumn = OldDBColumn.ValColumn;
        ValRow = OldDBColumn.ValRow;
        initializeNumberFormat();
    }

    public DBColumn(TextTableHandler _oTextTableHandler, RecordParser _CurDBMetaData, String _FieldName, String TableName) throws Exception
    {
        this.oTextTableHandler = _oTextTableHandler;
        this.CurDBMetaData = _CurDBMetaData;
        CurDBField = CurDBMetaData.getFieldColumnByFieldName(_FieldName);
        bIsGroupColumn = true;
        XTextRange xTextCell;
        XCell xCell;
        getTableColumns(TableName);

        XTableRows xRows = null;
        try
        {
            xRows = xTextTable.getRows();
        }
        catch (java.lang.NullPointerException e)
        {
            e.printStackTrace();
// TODO: handle the nullpointer right
//                return;
        }
        for (int n = 0; n < xTableColumns.getCount(); n++)
        {
            for (int m = 0; m < xRows.getCount(); m++)
            {
                xCell = xCellRange.getCellByPosition(n, m);
                xTextCell = UnoRuntime.queryInterface(XTextRange.class, xCell);
                String CompString = TableName.substring(4);
                XTextCursor xLocCellCursor = TextDocument.createTextCursor(xCell);
                if (isNameCell(xLocCellCursor, CurDBField.getFieldName(), CompString))
                {
                    xNameCell = xCell;
                    xNameTextCell = xTextCell;
                }
                else
                {   //In Grouping Sections only two cells are allowed ' if (CellString.equals(CurFieldString)){
                    xValCell = xCell;
                    xValTextCell = xTextCell;
                    xValCellCursor = xLocCellCursor;
                    ValColumn = n;
                    ValRow = m;
                    checkforLeftAlignment();
                }
            }
        }
    }

    private void getTableColumns(String TableName)
    {
        try
        {
            XNameAccess xAllTextTables = oTextTableHandler.xTextTablesSupplier.getTextTables();
            if (xAllTextTables.hasByName(TableName))
            {
                Object oTextTable = xAllTextTables.getByName(TableName);
                xCellRange = UnoRuntime.queryInterface(XCellRange.class, oTextTable);
                xTextTable = UnoRuntime.queryInterface(XTextTable.class, oTextTable);
                xTableName = UnoRuntime.queryInterface(XNamed.class, oTextTable);
                xTableColumns = xTextTable.getColumns();
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    public void initializeNumberFormat()
    {
        if (CurDBField.isBoolean())
        {
            NumberFormatter oNumberFormatter = oTextTableHandler.getNumberFormatter();
            int iLogicalFormatKey = oNumberFormatter.setBooleanReportDisplayNumberFormat();
            oNumberFormatter.setNumberFormat(xValCell, iLogicalFormatKey, oNumberFormatter);
        }
        else
        {
            oTextTableHandler.getNumberFormatter().setNumberFormat(xValCell, CurDBField.getDBFormatKey(), CurDBMetaData.getNumberFormatter());
        }
        setCellFont();
    }

    public void insertColumnData(TextFieldHandler oTextFieldHandler, boolean _bIsLandscape)
    {
        insertUserFieldToTableCell(oTextFieldHandler);
        replaceValueCellofTable(_bIsLandscape);
    }

    private void insertUserFieldToTableCell(TextFieldHandler oTextFieldHandler)
    {
        XTextCursor xTextCursor = TextDocument.createTextCursor(xNameCell);
        xTextCursor.gotoStart(false);
        xTextCursor.gotoEnd(true);
        xTextCursor.setString(PropertyNames.EMPTY_STRING);
        oTextFieldHandler.insertUserField(xTextCursor, CurDBField.getFieldName(), CurDBField.getFieldTitle());
    }



    public void formatValueCell()
    {
        initializeNumberFormat();
        if (checkforLeftAlignment())
        {
            bAlignLeft = true;
        }
    }

    private boolean checkforLeftAlignment()
    {
        bAlignLeft = ((CurDBField.isNumberFormat()) && (ValColumn == xTableColumns.getCount() - 1));
        return bAlignLeft;
    }

    public void modifyCellContent(XCellRange xCellRange, Object CurGroupValue)
    {
        try
        {
            xValCell = xCellRange.getCellByPosition(ValColumn, ValRow);
            xValTextCell = UnoRuntime.queryInterface(XTextRange.class, xValCell);
            modifyCellContent(CurGroupValue);
            if (bAlignLeft)
            {
                xValCellCursor = TextDocument.createTextCursor(xValCell);
                Helper.setUnoPropertyValue(xValCellCursor, "ParaAdjust", Integer.valueOf(com.sun.star.style.ParagraphAdjust.LEFT_value));
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    private void modifyCellContent(Object CurGroupValue)
    {
        double dblValue = 0;
        try
        {
            if (xValCell != null)
            {
                if (AnyConverter.isString(CurGroupValue))
                {
                    String sValue = AnyConverter.toString(CurGroupValue);
                    xValTextCell.setString(sValue);
                }
                else
                {
                    if (AnyConverter.isBoolean(CurGroupValue))
                    {
                        dblValue = AnyConverter.toInt(CurGroupValue);
                    }
                    if (AnyConverter.isByte(CurGroupValue))
                    {
                        dblValue = AnyConverter.toByte(CurGroupValue);
                    }
                    else if (AnyConverter.isDouble(CurGroupValue))
                    {
                        dblValue = AnyConverter.toDouble(CurGroupValue);
                    }
                    else if (AnyConverter.isFloat(CurGroupValue))
                    {
                        dblValue = AnyConverter.toFloat(CurGroupValue);
                    }
                    else if (AnyConverter.isInt(CurGroupValue))
                    {
                        dblValue = AnyConverter.toInt(CurGroupValue);
                    }
                    else if (AnyConverter.isLong(CurGroupValue))
                    {
                        dblValue = AnyConverter.toLong(CurGroupValue);
                    }
                    else if (AnyConverter.isShort(CurGroupValue))
                    {
                        dblValue = AnyConverter.toShort(CurGroupValue);
                    }
                    xValCell.setValue(dblValue);
                }
            }
        }
        catch (IllegalArgumentException exception)
        {
            System.err.println(exception);
        }
    }
    // If the parameter CurGroupValue is null the placeholders are inserted
    private void replaceValueCellofTable(boolean _bIsLandscape)
    {
        try
        {
            Object CurGroupValue;
            if (!bIsGroupColumn && !CurDBField.isNumberFormat())
            {
                CurGroupValue = BlindtextCreator.adjustBlindTextlength(CurDBField.getFieldTitle(), CurDBField.getFieldWidth(), _bIsLandscape, bIsGroupColumn, CurDBMetaData.getRecordFieldNames());
            }
            else
            {
                CurGroupValue = CurDBField.getDefaultValue();
            }
            modifyCellContent(CurGroupValue);
            if (bAlignLeft)
            {
                Helper.setUnoPropertyValue(xValCellCursor, "ParaAdjust", Integer.valueOf(ParagraphAdjust.LEFT_value));
            }

            int nFieldType = CurDBField.getFieldType();
            if ((nFieldType == com.sun.star.sdbc.DataType.BIT) ||
                (nFieldType == com.sun.star.sdbc.DataType.BOOLEAN))
            {
                CharFontName = "StarSymbol";
                Helper.setUnoPropertyValue(xValCellCursor, "CharFontName", CharFontName);
                if (!bIsGroupColumn)
                {
                    Helper.setUnoPropertyValue(xValCellCursor, "ParaAdjust", Integer.valueOf(ParagraphAdjust.CENTER_value));
                }
            }
            else
            {
                if (PropertyState == com.sun.star.beans.PropertyState.DEFAULT_VALUE)
                {
                    XPropertyState xPropState = UnoRuntime.queryInterface(XPropertyState.class, xValCellCursor);
                    xPropState.setPropertyToDefault("CharFontName");
                }
                else
                {
                    if (PropertyState == com.sun.star.beans.PropertyState.DIRECT_VALUE)
                    {
                        Helper.setUnoPropertyValue(xValCellCursor, "CharFontName", CharFontName);
                    }
                }
            }
        }
        catch (com.sun.star.beans.UnknownPropertyException exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public void setCellFont()
    {
        try
        {
            XPropertyState xPropertyState;
            int nFieldType = CurDBField.getFieldType();
            if ((nFieldType == com.sun.star.sdbc.DataType.BIT) ||
                (nFieldType == com.sun.star.sdbc.DataType.BOOLEAN))
            {
                CharFontName = "StarSymbol";
                PropertyState = com.sun.star.beans.PropertyState.DIRECT_VALUE;
                xValCellCursor.gotoStart(false);
                xValCellCursor.gotoEnd(true);
                Helper.setUnoPropertyValue(xValCellCursor, "CharFontName", CharFontName);
            }
            else
            {
                xPropertyState = UnoRuntime.queryInterface(XPropertyState.class, xValCellCursor);
                PropertyState = xPropertyState.getPropertyState("CharFontName");
                CharFontName = AnyConverter.toString(Helper.getUnoPropertyValue(xValCellCursor, "CharFontName"));
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    private boolean isNameCell(XTextCursor xCellCursor, String CurFieldName, String CompString)
    {
        try
        {
            xCellCursor.gotoStart(false);
            XTextRange xTextRange = xCellCursor.getEnd();
            Object oTextField = Helper.getUnoPropertyValue(xTextRange, "TextField");
            if (AnyConverter.isVoid(oTextField))
            {
                return false;
            }
            else
            {
                XDependentTextField xDependent = UnoRuntime.queryInterface(XDependentTextField.class, oTextField);
                XPropertySet xMaster = xDependent.getTextFieldMaster();
                String UserFieldName = (String) xMaster.getPropertyValue(PropertyNames.PROPERTY_NAME);
                boolean bIsNameCell = false;
                if ((UserFieldName.startsWith(CompString)) || (UserFieldName.equals(CurFieldName)))
                {
                    bIsNameCell = true;
                }
                else
                {
                    // stupid hack, 'Title' is not a real good Table-Cell-Name
                    // take a look at xmloff/source/text/txtvfldi.txt, there exists 2 '_renamed_' strings
                    String sLocalCurFieldName = CurFieldName + "_renamed_";
                    if (UserFieldName.startsWith(sLocalCurFieldName))
                    {
                        bIsNameCell = true;
                    }
                }
                return bIsNameCell;
            }
        }
        // Todo: Insert a  resource; Exception should be thrown to the calling routine
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
            return true;    //most probably this is really the Namecell!!!!
        }
    }
}
