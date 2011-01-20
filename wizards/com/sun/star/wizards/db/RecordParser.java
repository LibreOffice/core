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
package com.sun.star.wizards.db;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.sdbc.DataType;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.wizards.common.Helper;
import com.sun.star.sdb.XCompletedExecution;
import com.sun.star.lang.Locale;
import com.sun.star.wizards.common.InvalidQueryException;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.NumberFormatter;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.task.XInteractionHandler;
import com.sun.star.wizards.common.PropertyNames;

public class RecordParser extends QueryMetaData
{

    XNameAccess xColumns;
    com.sun.star.sdbc.XRow xRow;
    com.sun.star.sdbc.XRow xResultSetRow;
    public XResultSet ResultSet;
    XInterface xRowSet;
    XCompletedExecution xExecute;
    XColumnsSupplier xRowSetColumnsSupplier;
    XComponent xRowSetComponent;
    XInteractionHandler xInteraction;
    public FieldColumn[] GroupFieldColumns;
    public FieldColumn[] RecordFieldColumns;

    public RecordParser(XMultiServiceFactory _xMSF, Locale _aLocale, NumberFormatter _oNumberFormatter)
    {
        super(_xMSF, _aLocale, _oNumberFormatter);
        getInterfaces();
    }

    /** Creates a new instance of RecordParser */
    public RecordParser(XMultiServiceFactory _xMSF)
    {
        super(_xMSF);
        getInterfaces();
    }

    private void getInterfaces()
    {
        try
        {
            xRowSet = (XInterface) xMSF.createInstance("com.sun.star.sdb.RowSet");
            xRowSetColumnsSupplier = (XColumnsSupplier) UnoRuntime.queryInterface(XColumnsSupplier.class, xRowSet);
            xRowSetComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, xRowSet);
            xExecute = (com.sun.star.sdb.XCompletedExecution) UnoRuntime.queryInterface(com.sun.star.sdb.XCompletedExecution.class, xRowSet);
            XInterface oInteraction = (XInterface) xMSF.createInstance("com.sun.star.task.InteractionHandler");
            xInteraction = (XInteractionHandler) UnoRuntime.queryInterface(XInteractionHandler.class, oInteraction);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    private Object getColumnStringValue(int ColIndex)
    {
        try
        {
            com.sun.star.uno.Type CurType;
            Object oAny;
            String sValue = xResultSetRow.getString(ColIndex); //???
            CurType = new com.sun.star.uno.Type(String.class);
            oAny = AnyConverter.toObject(CurType, sValue);
            return oAny;
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
            return null;
        }
    }

    private Object getColumnDoubleValue(int ColIndex, boolean bisDate)
    {
        try
        {
            Double DblValue;
            if (bisDate)
            {
                DblValue = new Double(xResultSetRow.getDouble(ColIndex) + (double) super.getNullDateCorrection());
            }
            else
            {
                DblValue = new Double(xResultSetRow.getDouble(ColIndex));
            }
            if (!xResultSetRow.wasNull())
            {
                return DblValue;
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
        return Any.VOID;
    }

    public Object getColumnValue(int ColIndex, int iType)
    {
        boolean bResult;
        Object oAny = Any.VOID;
        switch (iType)
        {
            case DataType.BIT: // ==  -7;
            case DataType.BOOLEAN:
                oAny = getColumnDoubleValue(ColIndex, false);
                break;

            case DataType.TINYINT: // ==  -6;
                oAny = getColumnDoubleValue(ColIndex, false);
                break;

            case DataType.BIGINT: // ==  -5;
                oAny = getColumnDoubleValue(ColIndex, false);
                break;

            case DataType.CHAR: // ==   1;
                oAny = getColumnStringValue(ColIndex);
                break;

            case DataType.VARCHAR: // ==  12;
                oAny = getColumnStringValue(ColIndex);
                //      double sResult = xResultSetRow.getClob(ColIndex);       //???
                break;

            case DataType.LONGVARCHAR: // ==  -1;
                oAny = getColumnStringValue(ColIndex);
                //      double sResult = xResultSetRow.getCharacterStream(ColIndex);
                break;

            case DataType.NUMERIC: // ==   2;
                oAny = getColumnDoubleValue(ColIndex, false);
                break;

            case DataType.INTEGER: // ==   4;
                oAny = getColumnDoubleValue(ColIndex, false);
                break;

            case DataType.SMALLINT: // ==   5;
                oAny = getColumnDoubleValue(ColIndex, false);
                break;

            case DataType.DECIMAL: // ==   3;  [mit Nachkommastellen]
                oAny = getColumnDoubleValue(ColIndex, false);
                break;

            case DataType.FLOAT: // ==   6;
                oAny = getColumnDoubleValue(ColIndex, false);
                break;

            case DataType.REAL: // ==   7;
                oAny = getColumnDoubleValue(ColIndex, false);
                break;

            case DataType.DOUBLE: // ==   8;
                oAny = getColumnDoubleValue(ColIndex, false);
                break;

            case DataType.DATE: // ==  91;
                oAny = getColumnDoubleValue(ColIndex, true);
                break;

            case DataType.TIME: // ==  92;
                oAny = getColumnDoubleValue(ColIndex, false);
                break;

            case DataType.TIMESTAMP: // ==  93;
                oAny = getColumnDoubleValue(ColIndex, true);
                break;
        }

        // Is the index OK; increment?
        //  String sValue =  xResultSetRow.getString(ColIndex);
        return oAny;
    }

    public boolean executeCommand(int _nCommandType) throws InvalidQueryException
    {
        try
        {
            Helper.setUnoPropertyValue(xRowSet, "DataSourceName", DataSourceName);
            Helper.setUnoPropertyValue(xRowSet, PropertyNames.ACTIVE_CONNECTION, DBConnection);
            Helper.setUnoPropertyValue(xRowSet, PropertyNames.COMMAND, Command);
            Helper.setUnoPropertyValue(xRowSet, PropertyNames.COMMAND_TYPE, new Integer(_nCommandType)); // CommandType
            xExecute.executeWithCompletion(xInteraction);
            com.sun.star.sdb.XResultSetAccess xResultAccess = (com.sun.star.sdb.XResultSetAccess) UnoRuntime.queryInterface(com.sun.star.sdb.XResultSetAccess.class, xRowSet);
            ResultSet = xResultAccess.createResultSet();
            xResultSetRow = (com.sun.star.sdbc.XRow) UnoRuntime.queryInterface(com.sun.star.sdbc.XRow.class, ResultSet);
            XColumnsSupplier xDBCols = (XColumnsSupplier) UnoRuntime.queryInterface(XColumnsSupplier.class, ResultSet);
            xColumns = xDBCols.getColumns();
            setCommandType(_nCommandType);
            return true;
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
            throw new InvalidQueryException(xMSF, Command);
        }
    }

    public boolean getFields(String[] _sFieldNames, boolean binitializeDBColumns)
    {
        try
        {
            if (binitializeDBColumns == true)
            {
                initializeFieldColumns(_sFieldNames, xColumns);
            }
            String[] AllQueryFieldNames = xColumns.getElementNames();
            String[] sFieldNames = getFieldNames();
            for (int i = 0; i < FieldColumns.length; i++)
            {
                String sFieldName = FieldColumns[i].getFieldName();
                int nColIndex = JavaTools.FieldInList(AllQueryFieldNames, sFieldName) + 1;
                FieldColumns[i].ColIndex = nColIndex;
                if (nColIndex == -1)
                {
                    throw new InvalidQueryException(xMSF, Command);
                }
            }
            GroupFieldColumns = getFieldColumnList(GroupFieldNames);
            RecordFieldColumns = getFieldColumnList(getRecordFieldNames());
            return true;
        }
        catch (InvalidQueryException queryexception)
        {
            queryexception.printStackTrace(System.out);
            return false;
        }
    }

    private FieldColumn[] getFieldColumnList(String[] _FieldNames)
    {
        FieldColumn[] LocFieldColumns = new FieldColumn[_FieldNames.length];
        for (int i = 0; i < _FieldNames.length; i++)
        {
            LocFieldColumns[i] = super.getFieldColumnByFieldName(_FieldNames[i]);
        }
        return LocFieldColumns;
    }

    public Object getGroupColumnValue(int ColIndex)
    {
        FieldColumn CurDBFieldColumn = this.GroupFieldColumns[ColIndex];
        return getColumnValue(CurDBFieldColumn.ColIndex, CurDBFieldColumn.getFieldType());
    }

    public boolean getcurrentRecordData(java.util.Vector DataVector)
    {
        Object[] RecordValueArray = new Object[RecordFieldColumns.length];
        for (int i = 0; i < RecordFieldColumns.length; i++)
        {
            FieldColumn CurDBFieldColumn = this.RecordFieldColumns[i];
            RecordValueArray[i] = getColumnValue(CurDBFieldColumn.ColIndex, CurDBFieldColumn.getFieldType()); //FinalColIndex
        }
        DataVector.addElement(RecordValueArray);
        return true;
    }

    public void dispose()
    {
        if (xRowSetComponent != null)
        {
            xRowSetComponent.dispose();
        }
        super.dispose();
    }
}
