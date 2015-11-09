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
import com.sun.star.wizards.common.InvalidQueryException;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.task.XInteractionHandler;
import com.sun.star.wizards.common.PropertyNames;

public class RecordParser extends QueryMetaData
{

    private XNameAccess xColumns;
    private com.sun.star.sdbc.XRow xResultSetRow;
    public XResultSet ResultSet;
    private XInterface xRowSet;
    private XCompletedExecution xExecute;
    private XComponent xRowSetComponent;
    private XInteractionHandler xInteraction;
    public FieldColumn[] GroupFieldColumns;
    public FieldColumn[] RecordFieldColumns;

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
            UnoRuntime.queryInterface(XColumnsSupplier.class, xRowSet);
            xRowSetComponent = UnoRuntime.queryInterface(XComponent.class, xRowSet);
            xExecute = UnoRuntime.queryInterface(XCompletedExecution.class, xRowSet);
            XInterface oInteraction = (XInterface) xMSF.createInstance("com.sun.star.task.InteractionHandler");
            xInteraction = UnoRuntime.queryInterface(XInteractionHandler.class, oInteraction);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
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
            exception.printStackTrace(System.err);
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
                DblValue = new Double(xResultSetRow.getDouble(ColIndex) + super.getNullDateCorrection());
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
            exception.printStackTrace(System.err);
        }
        return Any.VOID;
    }

    private Object getColumnValue(int ColIndex, int iType)
    {
        Object oAny = Any.VOID;
        switch (iType)
        {
            case DataType.BIT: // ==  -7;
            case DataType.BOOLEAN:
            case DataType.TINYINT: // ==  -6;
            case DataType.BIGINT: // ==  -5;
            case DataType.NUMERIC: // ==   2;
            case DataType.INTEGER: // ==   4;
            case DataType.SMALLINT: // ==   5;
            case DataType.DECIMAL: // ==   3;  [mit Nachkommastellen]
            case DataType.FLOAT: // ==   6;
            case DataType.REAL: // ==   7;
            case DataType.DOUBLE: // ==   8;
            case DataType.TIME: // ==  92;
                oAny = getColumnDoubleValue(ColIndex, false);
                break;

            case DataType.DATE: // ==  91;
            case DataType.TIMESTAMP: // ==  93;
                oAny = getColumnDoubleValue(ColIndex, true);
                break;
            case DataType.CHAR: // ==   1;
            case DataType.VARCHAR: // ==  12;
            case DataType.LONGVARCHAR: // ==  -1;
                oAny = getColumnStringValue(ColIndex);
                break;

        }

        // Is the index OK; increment?
        return oAny;
    }

    public boolean executeCommand(int _nCommandType) throws InvalidQueryException
    {
        try
        {
            Helper.setUnoPropertyValue(xRowSet, "DataSourceName", DataSourceName);
            Helper.setUnoPropertyValue(xRowSet, PropertyNames.ACTIVE_CONNECTION, DBConnection);
            Helper.setUnoPropertyValue(xRowSet, PropertyNames.COMMAND, Command);
            Helper.setUnoPropertyValue(xRowSet, PropertyNames.COMMAND_TYPE, Integer.valueOf(_nCommandType)); // CommandType
            xExecute.executeWithCompletion(xInteraction);
            com.sun.star.sdb.XResultSetAccess xResultAccess = UnoRuntime.queryInterface(com.sun.star.sdb.XResultSetAccess.class, xRowSet);
            ResultSet = xResultAccess.createResultSet();
            xResultSetRow = UnoRuntime.queryInterface(com.sun.star.sdbc.XRow.class, ResultSet);
            XColumnsSupplier xDBCols = UnoRuntime.queryInterface(XColumnsSupplier.class, ResultSet);
            xColumns = xDBCols.getColumns();
            setCommandType(_nCommandType);
            return true;
        }
        catch (Exception exception)
        {
            throw new InvalidQueryException(xMSF, Command, exception);
        }
    }

    public boolean getFields(String[] _sFieldNames, boolean binitializeDBColumns)
    {
        try
        {
            if (binitializeDBColumns)
            {
                initializeFieldColumns(_sFieldNames, xColumns);
            }
            String[] AllQueryFieldNames = xColumns.getElementNames();
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
            queryexception.printStackTrace(System.err);
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

    public boolean getcurrentRecordData(java.util.ArrayList<Object[]> DataVector)
    {
        Object[] RecordValueArray = new Object[RecordFieldColumns.length];
        for (int i = 0; i < RecordFieldColumns.length; i++)
        {
            FieldColumn CurDBFieldColumn = this.RecordFieldColumns[i];
            RecordValueArray[i] = getColumnValue(CurDBFieldColumn.ColIndex, CurDBFieldColumn.getFieldType()); //FinalColIndex
        }
        DataVector.add(RecordValueArray);
        return true;
    }

    @Override
    public void dispose()
    {
        if (xRowSetComponent != null)
        {
            xRowSetComponent.dispose();
        }
        super.dispose();
    }
}
