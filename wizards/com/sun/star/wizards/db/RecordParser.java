/*************************************************************************
*
*  $RCSfile: RecordParser.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: kz $ $Date: 2004-05-19 12:41:14 $
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
*/

package com.sun.star.wizards.db;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.sdbc.DataType;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.wizards.common.Helper;
import com.sun.star.sdb.XCompletedExecution;
import com.sun.star.lang.Locale;
import com.sun.star.util.XNumberFormats;
import com.sun.star.wizards.common.InvalidQueryException;
import com.sun.star.wizards.common.JavaTools;
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
    long lDateCorrection;
    //    int[] SelColIndices = null;
    //    int[] GroupColIndices = null;
    public FieldColumn[] GroupFieldColumns;
    public FieldColumn[] RecordFieldColumns;

    public RecordParser(XMultiServiceFactory _xMSF, Locale _CharLocale, XNumberFormats _NumberFormats) {
        super(_xMSF, _CharLocale, _NumberFormats);
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
                DblValue = new Double(xResultSetRow.getDouble(ColIndex) + (double) lDateCorrection);
            else
                DblValue = new Double(xResultSetRow.getDouble(ColIndex));
            return DblValue;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            return null;
        }
    }

    public Object getColumnValue(int ColIndex, int iType) {
        boolean bResult;
        Object oAny = null;
        switch (iType) {
            case DataType.BIT : // ==  -7;
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

    public boolean executeCommand(String sMessage, boolean binitializeDBColumns) throws InvalidQueryException {
        try {
            Helper.setUnoPropertyValue(xRowSet, "DataSourceName", DataSourceName);
            Helper.setUnoPropertyValue(xRowSet, "ActiveConnection", DBConnection);
            Helper.setUnoPropertyValue(xRowSet, "Command", Command);
            Helper.setUnoPropertyValue(xRowSet, "CommandType", new Integer(com.sun.star.sdb.CommandType.COMMAND)); // CommandType
            xExecute.executeWithCompletion(xInteraction);
            // create the ResultSet to access the data

            com.sun.star.sdb.XResultSetAccess xResultAccess = (com.sun.star.sdb.XResultSetAccess) UnoRuntime.queryInterface(com.sun.star.sdb.XResultSetAccess.class, xRowSet);
            ResultSet = xResultAccess.createResultSet();
            xResultSetRow = (com.sun.star.sdbc.XRow) UnoRuntime.queryInterface(com.sun.star.sdbc.XRow.class, ResultSet);
            XColumnsSupplier xDBCols = (XColumnsSupplier) UnoRuntime.queryInterface(XColumnsSupplier.class, ResultSet);
            xColumns = xDBCols.getColumns();
            String[] sElementNames = xColumns.getElementNames();
            CommandType = com.sun.star.sdb.CommandType.COMMAND;

            if (binitializeDBColumns == true)
                setFieldNames(FieldNames);
//              super.setFieldColumns(false);
            this.setColindices();
            GroupFieldColumns = getFieldColumnList(GroupFieldNames);
            RecordFieldColumns = getFieldColumnList(RecordFieldNames);

            //      GroupColIndices = getSelectedQueryFields(GroupFieldNames);
            //      SelColIndices = getSelectedQueryFields(RecordFieldNames);
            // todo: the following Methods call directly from Dataimport object
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
            LocFieldColumns[i] = super.getFieldColumnByDisplayName(_FieldNames[i]);
        }
        return LocFieldColumns;
    }

    public Object getGroupColumnValue(int ColIndex) {
        FieldColumn CurDBFieldColumn = this.GroupFieldColumns[ColIndex];
        return getColumnValue(CurDBFieldColumn.ColIndex, CurDBFieldColumn.FieldType);
    }

    public boolean getcurrentRecordData(java.util.Vector DataVector) {
        Object[] RecordValueArray = new Object[RecordFieldNames.length];
        for (int i = 0; i < RecordFieldNames.length; i++) {
            FieldColumn CurDBFieldColumn = this.RecordFieldColumns[i];
            RecordValueArray[i] = getColumnValue(CurDBFieldColumn.ColIndex, CurDBFieldColumn.FieldType); //FinalColIndex
        }
        DataVector.addElement(RecordValueArray);
        return true;
    }


    private void setColindices() throws InvalidQueryException {
        String[] AllQueryFieldNames = xColumns.getElementNames();
        for (int i = 0; i < FieldNames.length; i++) {
            FieldColumn CurFieldColumn = getFieldColumnByDisplayName(FieldNames[i]);
            CurFieldColumn.ColIndex = JavaTools.FieldInList(AllQueryFieldNames, CurFieldColumn.FieldName) + 1;
            if (CurFieldColumn.ColIndex == -1)
                throw new InvalidQueryException(xMSF, Command);
        }

    }

    /*    public int[] getSelectedQueryFields(String[] FieldNames) throws InvalidQueryException{
            String CurFieldName;
            String[] AllQueryFieldNames = xColumns.getElementNames();
            int FieldCount = FieldNames.length;
            int[] SelColIndices = JavaTools.initializeintArray(FieldCount, -1);
            for (int i=0; i<FieldCount; i++){
                CurFieldName = FieldNames[i];
                SelColIndices[i] = JavaTools.FieldInList(AllQueryFieldNames, CurFieldName)+1;
                if (SelColIndices[i] == -1)
                    throw new InvalidQueryException(xMSF, Command);
            }
            return SelColIndices;
        } */

    public void dispose() {
        if (xRowSetComponent != null)
            xRowSetComponent.dispose();
    }
}
