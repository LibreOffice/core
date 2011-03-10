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

import com.sun.star.beans.PropertyState;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertyState;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNamed;
// import com.sun.star.lang.IndexOutOfBoundsException;
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
import com.sun.star.util.XNumberFormats;
import com.sun.star.wizards.db.*;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.text.TextDocument;
import com.sun.star.wizards.text.TextTableHandler;
import com.sun.star.wizards.text.TextFieldHandler;

public class DBColumn
{

    public XCell xValCell;
    public XTextRange xValTextCell;
    public XTextCursor xValCellCursor;
    public XNumberFormats xValCellNumberFormats;
    public XCell xNameCell;
    public XTextRange xNameTextCell;
    public boolean bAlignLeft;
    public Object DefaultValue;
    public String CharFontName;
    public int FormatKey;
    public PropertyState PropertyState;
    public int ValColumn = 1;
    public int ValRow = 0;
    public FieldColumn CurDBField;
    private XTextTable xTextTable;
    private XTableColumns xTableColumns;
    private XCellRange xCellRange;
    public XNamed xTableName;
    private boolean bIsGroupColumn;
    TextDocument oTextDocument;
    RecordParser CurDBMetaData;
    RecordTable CurRecordTable;
    TextTableHandler oTextTableHandler;

    public DBColumn(TextTableHandler _oTextTableHandler, RecordParser _CurDBMetaData, int i) throws Exception
    {
        CurRecordTable = new RecordTable(_oTextTableHandler);
        initializeRecordTableMembers(CurRecordTable, _oTextTableHandler, _CurDBMetaData, i, false);
    }

    public DBColumn(RecordTable _CurRecordTable, TextTableHandler _oTextTableHandler, RecordParser _CurDBMetaData, int i, boolean _bforce) throws Exception
    {
        initializeRecordTableMembers(_CurRecordTable, _oTextTableHandler, _CurDBMetaData, i, _bforce);
    }

    public DBColumn(RecordTable _CurRecordTable, TextTableHandler _oTextTableHandler, RecordParser _CurDBMetaData, int i) throws Exception
    {
        initializeRecordTableMembers(_CurRecordTable, _oTextTableHandler, _CurDBMetaData, i, false);
    }

    private void initializeRecordTableMembers(RecordTable _CurRecordTable, TextTableHandler _oTextTableHandler, RecordParser _CurDBMetaData, int i, boolean _bForce) throws Exception
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
            XTextRange xTextCell = (XTextRange) UnoRuntime.queryInterface(XTextRange.class, xCell);
            String CompString = "Column";
            XTextCursor xLocCellCursor = TextDocument.createTextCursor(xCell);
            if (isNameCell(xLocCellCursor, CurDBField.getFieldName(), CompString) || (_bforce))
            {
                xNameCell = xCell;
                xNameTextCell = xTextCell;
                xValCell = CurRecordTable.xCellRange.getCellByPosition(_nColumn, 1);
                xValTextCell = (XTextRange) UnoRuntime.queryInterface(XTextRange.class, xValCell);
                xValCellCursor = TextDocument.createTextCursor(xValCell);
                ValColumn = _nColumn;
                return true;
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
        return false;
    }

    public DBColumn(TextTableHandler _oTextTableHandler, RecordParser _CurDBMetaData, String _FieldName, int GroupIndex, String TableName, DBColumn OldDBColumn) throws Exception
    {
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

    public DBColumn(TextTableHandler _oTextTableHandler, RecordParser _CurDBMetaData, String _FieldName, int GroupIndex, String TableName) throws Exception
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
                xTextCell = (XTextRange) UnoRuntime.queryInterface(XTextRange.class, xCell);
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
            if (xAllTextTables.hasByName(TableName) == true)
            {
                Object oTextTable = xAllTextTables.getByName(TableName);
                xCellRange = (XCellRange) UnoRuntime.queryInterface(XCellRange.class, oTextTable);
                xTextTable = (XTextTable) UnoRuntime.queryInterface(XTextTable.class, oTextTable);
                xTableName = (XNamed) UnoRuntime.queryInterface(XNamed.class, oTextTable);
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

    public void insertUserFieldToTableCell(TextFieldHandler oTextFieldHandler)
    {
        XTextCursor xTextCursor = TextDocument.createTextCursor(xNameCell);
        xTextCursor.gotoStart(false);
        xTextCursor.gotoEnd(true);
        xTextCursor.setString(PropertyNames.EMPTY_STRING);
        oTextFieldHandler.insertUserField(xTextCursor, CurDBField.getFieldName(), CurDBField.getFieldTitle());
    }

    public void insertUserFieldToTableCell(TextFieldHandler oTextFieldHandler, XCell xCell)
    {
        XTextCursor xTextCursor = TextDocument.createTextCursor(xCell);
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
            xValTextCell = (XTextRange) UnoRuntime.queryInterface(XTextRange.class, xValCell);
            modifyCellContent(CurGroupValue);
            if (bAlignLeft)
            {
                xValCellCursor = TextDocument.createTextCursor(xValCell);
                Helper.setUnoPropertyValue(xValCellCursor, "ParaAdjust", new Integer(com.sun.star.style.ParagraphAdjust.LEFT_value));
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    public void modifyCellContent(Object CurGroupValue)
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
                        dblValue = (double) AnyConverter.toInt(CurGroupValue);
                    }
                    if (AnyConverter.isByte(CurGroupValue))
                    {
                        dblValue = (double) AnyConverter.toByte(CurGroupValue);
                    }
                    else if (AnyConverter.isDouble(CurGroupValue))
                    {
                        dblValue = AnyConverter.toDouble(CurGroupValue);
                    }
                    else if (AnyConverter.isFloat(CurGroupValue))
                    {
                        dblValue = (double) AnyConverter.toFloat(CurGroupValue);
                    }
                    else if (AnyConverter.isInt(CurGroupValue))
                    {
                        dblValue = (double) AnyConverter.toInt(CurGroupValue);
                    }
                    else if (AnyConverter.isLong(CurGroupValue))
                    {
                        dblValue = (double) AnyConverter.toLong(CurGroupValue);
                    }
                    else if (AnyConverter.isShort(CurGroupValue))
                    {
                        dblValue = (double) AnyConverter.toShort(CurGroupValue);
                    }
                    xValCell.setValue(dblValue);
                }
            }
        }
        catch (Exception exception)
        {
            System.err.println(exception);
        }
    }
    // If the parameter CurGroupValue is null the placeholders are inserted
    public void replaceValueCellofTable(boolean _bIsLandscape)
    {
        try
        {
            Object CurGroupValue;
            if (bIsGroupColumn == false && CurDBField.isNumberFormat() == false)
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
                Helper.setUnoPropertyValue(xValCellCursor, "ParaAdjust", new Integer(ParagraphAdjust.LEFT_value));
            }

            int nFieldType = CurDBField.getFieldType();
            if ((nFieldType == com.sun.star.sdbc.DataType.BIT) ||
                (nFieldType == com.sun.star.sdbc.DataType.BOOLEAN))
            {
                CharFontName = "StarSymbol";
                Helper.setUnoPropertyValue(xValCellCursor, "CharFontName", CharFontName);
                if (bIsGroupColumn == false)
                {
                    Helper.setUnoPropertyValue(xValCellCursor, "ParaAdjust", new Integer(ParagraphAdjust.CENTER_value));
                }
            }
            else
            {
                if (PropertyState == com.sun.star.beans.PropertyState.DEFAULT_VALUE)
                {
                    XPropertyState xPropState = (XPropertyState) UnoRuntime.queryInterface(XPropertyState.class, xValCellCursor);
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
            exception.printStackTrace(System.out);
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
                xPropertyState = (XPropertyState) UnoRuntime.queryInterface(XPropertyState.class, xValCellCursor);
                PropertyState = xPropertyState.getPropertyState("CharFontName");
                CharFontName = AnyConverter.toString(Helper.getUnoPropertyValue(xValCellCursor, "CharFontName"));
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
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
                XDependentTextField xDependent = (XDependentTextField) UnoRuntime.queryInterface(XDependentTextField.class, oTextField);
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
//      sMsgInvalidTextField = oResource.getResText(UIConsts.RID_REPORT + 73);
//      SystemDialog.showMessageBox(oTextTableHandler.xMSFDoc, "ErrorBox", VclWindowPeerAttribute.OK, sMsgInvalidTextField);
            exception.printStackTrace(System.out);
            return true;    //most probably this is really the Namecell!!!!
        }
    }
}
