/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RecordParser.java,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-06 14:15:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/package com.sun.star.wizards.db;

import com.sun.star.awt.XWindow;
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

public class RecordParser extends QueryMetaData {
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

    public RecordParser(XMultiServiceFactory _xMSF, Locale _aLocale, NumberFormatter _oNumberFormatter) {
        super(_xMSF, _aLocale, _oNumberFormatter);
        getInterfaces();
    }

    /** Creates a new instance of RecordParser */
    public RecordParser(XMultiServiceFactory _xMSF) {
        super(_xMSF);
        getInterfaces();
    }

    private void getInterfaces() {
        try {
            xRowSet = (XInterface) xMSF.createInstance("com.sun.star.sdb.RowSet");
            xRowSetColumnsSupplier = (XColumnsSupplier) UnoRuntime.queryInterface(XColumnsSupplier.class, xRowSet);
            xRowSetComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, xRowSet);
            xExecute = (com.sun.star.sdb.XCompletedExecution) UnoRuntime.queryInterface(com.sun.star.sdb.XCompletedExecution.class, xRowSet);
            XInterface oInteraction = (XInterface) xMSF.createInstance("com.sun.star.sdb.InteractionHandler");
            xInteraction = (XInteractionHandler) UnoRuntime.queryInterface(XInteractionHandler.class, oInteraction);
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    private Object getColumnStringValue(int ColIndex) {
        try {
            com.sun.star.uno.Type CurType;
            Object oAny;
            String sValue = xResultSetRow.getString(ColIndex); //???
            CurType = new com.sun.star.uno.Type(String.class);
            oAny = AnyConverter.toObject(CurType, sValue);
            return oAny;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            return null;
        }
    }

    private Object getColumnDoubleValue(int ColIndex, boolean bisDate) {
        try {
            Double DblValue;
            if (bisDate)
                DblValue = new Double(xResultSetRow.getDouble(ColIndex) + (double) super.getNullDateCorrection());
            else
                DblValue = new Double(xResultSetRow.getDouble(ColIndex));
            if (!xResultSetRow.wasNull())
                return DblValue;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
        return Any.VOID;
    }

    public Object getColumnValue(int ColIndex, int iType) {
        boolean bResult;
        Object oAny = Any.VOID;
        switch (iType) {
            case DataType.BIT : // ==  -7;
            case DataType.BOOLEAN :
                oAny = getColumnDoubleValue(ColIndex, false);
                break;

            case DataType.TINYINT : // ==  -6;
                oAny = getColumnDoubleValue(ColIndex, false);
                break;

            case DataType.BIGINT : // ==  -5;
                oAny = getColumnDoubleValue(ColIndex, false);
                break;

            case DataType.CHAR : // ==   1;
                oAny = getColumnStringValue(ColIndex);
                break;

            case DataType.VARCHAR : // ==  12;
                oAny = getColumnStringValue(ColIndex);
                //      double sResult = xResultSetRow.getClob(ColIndex);       //???
                break;

            case DataType.LONGVARCHAR : // ==  -1;
                oAny = getColumnStringValue(ColIndex);
                //      double sResult = xResultSetRow.getCharacterStream(ColIndex);
                break;

            case DataType.NUMERIC : // ==   2;
                oAny = getColumnDoubleValue(ColIndex, false);
                break;

            case DataType.INTEGER : // ==   4;
                oAny = getColumnDoubleValue(ColIndex, false);
                break;

            case DataType.SMALLINT : // ==   5;
                oAny = getColumnDoubleValue(ColIndex, false);
                break;

            case DataType.DECIMAL : // ==   3;  [mit Nachkommastellen]
                oAny = getColumnDoubleValue(ColIndex, false);
                break;

            case DataType.FLOAT : // ==   6;
                oAny = getColumnDoubleValue(ColIndex, false);
                break;

            case DataType.REAL : // ==   7;
                oAny = getColumnDoubleValue(ColIndex, false);
                break;

            case DataType.DOUBLE : // ==   8;
                oAny = getColumnDoubleValue(ColIndex, false);
                break;

            case DataType.DATE : // ==  91;
                oAny = getColumnDoubleValue(ColIndex, true);
                break;

            case DataType.TIME : // ==  92;
                oAny = getColumnDoubleValue(ColIndex, false);
                break;

            case DataType.TIMESTAMP : // ==  93;
                oAny = getColumnDoubleValue(ColIndex, true);
                break;
        }

        // Is the index OK; increment?
        //  String sValue =  xResultSetRow.getString(ColIndex);
        return oAny;
    }

    public boolean executeCommand(String sMessage, String[] _sFieldNames, boolean binitializeDBColumns) throws InvalidQueryException {
        try {
            Helper.setUnoPropertyValue(xRowSet, "DataSourceName", DataSourceName);
            Helper.setUnoPropertyValue(xRowSet, "ActiveConnection", DBConnection);
            Helper.setUnoPropertyValue(xRowSet, "Command", Command);
            Helper.setUnoPropertyValue(xRowSet, "CommandType", new Integer(com.sun.star.sdb.CommandType.COMMAND)); // CommandType
            xExecute.executeWithCompletion(xInteraction);
            com.sun.star.sdb.XResultSetAccess xResultAccess = (com.sun.star.sdb.XResultSetAccess) UnoRuntime.queryInterface(com.sun.star.sdb.XResultSetAccess.class, xRowSet);
            ResultSet = xResultAccess.createResultSet();
            xResultSetRow = (com.sun.star.sdbc.XRow) UnoRuntime.queryInterface(com.sun.star.sdbc.XRow.class, ResultSet);
            XColumnsSupplier xDBCols = (XColumnsSupplier) UnoRuntime.queryInterface(XColumnsSupplier.class, ResultSet);
            xColumns = xDBCols.getColumns();
            setCommandType(com.sun.star.sdb.CommandType.COMMAND);
            if (binitializeDBColumns == true){
                    initializeFieldColumns(_sFieldNames, xColumns);
            }
            String[] AllQueryFieldNames = xColumns.getElementNames();
            String[] sFieldNames = getFieldNames();
            for (int i = 0; i < FieldColumns.length; i++) {
                    FieldColumns[i].ColIndex = JavaTools.FieldInList(AllQueryFieldNames, FieldColumns[i].FieldName) + 1;
                    if (FieldColumns[i].ColIndex == -1)
                            throw new InvalidQueryException(xMSF, Command);
            }
            GroupFieldColumns = getFieldColumnList(GroupFieldNames);
            RecordFieldColumns = getFieldColumnList(RecordFieldNames);
            return true;
        } catch (InvalidQueryException queryexception) {
            queryexception.printStackTrace(System.out);
            return false;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            throw new InvalidQueryException(xMSF, Command);
        }
    }

    private FieldColumn[] getFieldColumnList(String[] _FieldNames) {
        FieldColumn[] LocFieldColumns = new FieldColumn[_FieldNames.length];
        for (int i = 0; i < _FieldNames.length; i++) {
            LocFieldColumns[i] = super.getFieldColumnByFieldName(_FieldNames[i]);
        }
        return LocFieldColumns;
    }

    public Object getGroupColumnValue(int ColIndex) {
        FieldColumn CurDBFieldColumn = this.GroupFieldColumns[ColIndex];
        return getColumnValue(CurDBFieldColumn.ColIndex, CurDBFieldColumn.FieldType);
    }

    public boolean getcurrentRecordData(java.util.Vector DataVector) {
        Object[] RecordValueArray = new Object[RecordFieldColumns.length];
        for (int i = 0; i < RecordFieldColumns.length; i++) {
            FieldColumn CurDBFieldColumn = this.RecordFieldColumns[i];
            RecordValueArray[i] = getColumnValue(CurDBFieldColumn.ColIndex, CurDBFieldColumn.FieldType); //FinalColIndex
        }
        DataVector.addElement(RecordValueArray);
        return true;
    }


    public void dispose() {
        if (xRowSetComponent != null)
            xRowSetComponent.dispose();
        super.dispose();
    }
}
