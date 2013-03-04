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

import java.util.Vector;

import com.sun.star.beans.XPropertySet;
import com.sun.star.sdbc.ColumnSearch;
import com.sun.star.sdbc.ColumnValue;
import com.sun.star.sdbc.DataType;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XRow;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.PropertyNames;

public class TypeInspector
{

    private String[] sDataTypeNames;
    private int[] nDataTypeInfos;
    private int[] nPrecisionInfos;
    private int[] nNullableInfos;
    private int[] nSearchables;
    private boolean[] bisAutoIncrementableInfos;
    private int[] nMinScaleInfos;
    private int[] nMaxScaleInfos;
    private int[] nNumericFallBackList = new int[]
    {
        DataType.INTEGER, DataType.FLOAT, DataType.REAL, DataType.DOUBLE, DataType.NUMERIC, DataType.DECIMAL
    };
    final int INVALID = 999999;
    XResultSet xResultSet;

    public class TypeInfo
    {

        public int nDataType;
        public String sDataTypeName;
        public boolean bisAutoIncrementable;

        public TypeInfo(int _nDataType, String _sDataTypeName, boolean _bisAutoIncrementable)
        {
            nDataType = _nDataType;
            sDataTypeName = _sDataTypeName;
            bisAutoIncrementable = _bisAutoIncrementable;
        }
    }

    public TypeInspector(XResultSet _xResultSet)
    {
        try
        {
            xResultSet = _xResultSet;
            Vector<String> aTypeNameVector = new Vector<String>();
            Vector<Integer> aTypeVector = new Vector<Integer>();
            Vector<Integer> aNullableVector = new Vector<Integer>();
            Vector<Boolean> aAutoIncrementVector = new Vector<Boolean>();
            Vector<Integer> aPrecisionVector = new Vector<Integer>();
            Vector<Integer> aMinScaleVector = new Vector<Integer>();
            Vector<Integer> aMaxScaleVector = new Vector<Integer>();
            Vector<Integer> aSearchableVector = new Vector<Integer>();
            // Integer[] aIntegerDataTypes = null;
//      XResultSet xResultSet = xDBMetaDagetTypeInfo();
            XRow xRow = UnoRuntime.queryInterface(XRow.class, xResultSet);
            while (xResultSet.next())
            {
                aTypeNameVector.addElement(xRow.getString(1));
                aTypeVector.addElement(new Integer(xRow.getShort(2)));
                aPrecisionVector.addElement(new Integer(xRow.getInt(3)));
                aNullableVector.addElement(new Integer(xRow.getShort(7)));
                aSearchableVector.addElement(new Integer(xRow.getShort(9)));
                aAutoIncrementVector.addElement(Boolean.valueOf(xRow.getBoolean(12)));
                aMinScaleVector.addElement(new Integer(xRow.getShort(14)));
                aMaxScaleVector.addElement(new Integer(xRow.getShort(15)));

            }
            sDataTypeNames = new String[aTypeNameVector.size()];
            aTypeNameVector.toArray(sDataTypeNames);
            nDataTypeInfos = JavaTools.IntegerTointList(aTypeVector);
            nNullableInfos = JavaTools.IntegerTointList(aNullableVector);
            nSearchables = JavaTools.IntegerTointList(aSearchableVector);
            bisAutoIncrementableInfos = JavaTools.BooleanTobooleanList(aAutoIncrementVector);
            nPrecisionInfos = JavaTools.IntegerTointList(aPrecisionVector);
            nMinScaleInfos = JavaTools.IntegerTointList(aMinScaleVector);
            nMaxScaleInfos = JavaTools.IntegerTointList(aMaxScaleVector);
        }
        catch (SQLException e)
        {
            e.printStackTrace(System.err);
        }
    }

    public int getScale(XPropertySet _xColPropertySet)
    {
        try
        {
            int i = getDataTypeIndex(_xColPropertySet, false);
            int nScale = AnyConverter.toInt(_xColPropertySet.getPropertyValue("Scale"));
            if (i == -1)
            {
                return nScale;
            }
            if (nScale > nMaxScaleInfos[i])
            {
                return nMaxScaleInfos[i];
            }
            else if (nScale < nMinScaleInfos[i])
            {
                return nMinScaleInfos[i];
            }
            else
            {
                return nScale;
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
            return 0;
        }
    }

    public int getNullability(XPropertySet _xColPropertySet, int _nNullable)
    {
        int i = getDataTypeIndex(_xColPropertySet, false);
        if (i == -1)
        {
            return ColumnValue.NO_NULLS;
        }
        if (_nNullable == ColumnValue.NULLABLE)
        {
            return nNullableInfos[i];           //probably nullability is not allowed
        }
        return _nNullable;
    }

    public int getNullability(XPropertySet _xColPropertySet)
    {
        try
        {
            int i = getDataTypeIndex(_xColPropertySet, false);
            if (i == -1)
            {
                return ColumnValue.NO_NULLS;
            }
            int nNullable = AnyConverter.toInt(_xColPropertySet.getPropertyValue("IsNullable"));
            if (nNullable == ColumnValue.NULLABLE)
            {
                return nNullableInfos[i];
            }
            return nNullable;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
        return ColumnValue.NO_NULLS;
    }

    public boolean isColumnOrderable(XPropertySet _xColPropertySet)
    {
        int i = getDataTypeIndex(_xColPropertySet, false);
        if (i > -1)
        {
            return (nSearchables[i] != ColumnSearch.NONE);
        }
        else
        {
            return false;
        }
    }

    public int isNullable(XPropertySet _xColPropertySet)
    {
        int i = getDataTypeIndex(_xColPropertySet, false);
        if (i > -1)
        {
            return nNullableInfos[i];
        }
        else
        {
            return ColumnValue.NO_NULLS;
        }
    }

    private int getDataTypeIndex(XPropertySet _xColPropertySet, boolean _bCheckNumericAttributes)
    {
        try
        {
            int nPrecision = -1;
            int nScale = -1;
            int nDataType = AnyConverter.toInt(_xColPropertySet.getPropertyValue("Type"));
            String sTypeName = AnyConverter.toString(_xColPropertySet.getPropertyValue("TypeName"));
            if (_bCheckNumericAttributes)
            {
                nPrecision = AnyConverter.toInt(_xColPropertySet.getPropertyValue("Precision"));
                nScale = AnyConverter.toInt(_xColPropertySet.getPropertyValue("Scale"));
            }
            boolean bleaveloop = false;
            int startindex = 0;
            while (!bleaveloop)
            {
                int i = JavaTools.FieldInIntTable(nDataTypeInfos, nDataType, startindex);
                startindex = i + 1;
                bleaveloop = (i < 0);
                if (!bleaveloop && sTypeName.equals(sDataTypeNames[i]))
                {
                    if (_bCheckNumericAttributes)
                    {
                        if (nPrecision <= nPrecisionInfos[i] && (nScale >= nMinScaleInfos[i]) && (nScale <= nMinScaleInfos[i]) )
                        {
                            return i;
                        }
                    }
                    else
                    {
                        return i;
                    }
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
        return -1;
    }

    public boolean supportsDataType(int _curDataType)
    {
        return (JavaTools.FieldInIntTable(nDataTypeInfos, _curDataType) > -1);
    }

    public int getLastConversionFallbackDataType()
    {
        if (supportsDataType(DataType.VARCHAR))
        {
            return DataType.VARCHAR;
        }
        else
        {
            return DataType.LONGVARCHAR;
        }
    }

    /**
     * an empty string is returned when no appropriate Typename can be found
     * finds the first TypeName of the passed datatype.
     * @param _curDataType
     * @return
     */
    public String getDefaultTypeName(int _curDataType, Integer precision)
    {
        String ret = PropertyNames.EMPTY_STRING;
        for (int i = 0; i < nDataTypeInfos.length; i++)
        {
            if (nDataTypeInfos[i] == _curDataType)
            {
//                 System.out.println( "Desired prec " + precision + ",nPrecisionInfos[i]="+nPrecisionInfos[i] + ",sDataTypeNames[i]="+sDataTypeNames[i] );

                if (precision == null || nPrecisionInfos[i] >= precision.intValue())
                {
                    ret = sDataTypeNames[i]; // this fits best !
                    break;
                }
                else if (ret.length() == 0)
                {
                    // in case we dont find anything else, we at return a typename
                    // with the correct class
                    ret = sDataTypeNames[i];
                }
            }
        }
//         System.out.println( "_curDataType="+_curDataType+",precision="+precision+",ret="+
        return ret;
    }

    public int getDataType(String _sTypeName)
    {
        int i = JavaTools.FieldInList(sDataTypeNames, _sTypeName);
        if (i > -1)
        {
            return nDataTypeInfos[i];
        }
        else
        {
            return getLastConversionFallbackDataType();
        }
    }

    public int convertDataType(int _curDataType)
    {
        int retDataType = _curDataType;
        if (!supportsDataType(_curDataType))
        {
            switch (_curDataType)
            {
                case DataType.BIT:
                    retDataType = convertDataType(DataType.BOOLEAN);
                    break;
                case DataType.BOOLEAN:
                    retDataType = convertDataType(DataType.BIT);
                    break;
                case DataType.TINYINT:
                    retDataType = convertDataType(DataType.SMALLINT);
                    break;
                case DataType.SMALLINT:
                    retDataType = convertDataType(DataType.INTEGER);
                    break;
                case DataType.INTEGER:
                    retDataType = convertDataType(DataType.FLOAT);
                    break;
                case DataType.FLOAT:
                    retDataType = convertDataType(DataType.REAL);
                    break;
                case DataType.DATE:
                case DataType.TIME:
                    retDataType = convertDataType(DataType.TIMESTAMP);
                    break;
                case DataType.TIMESTAMP:
                case DataType.REAL:
                case DataType.BIGINT:
                    retDataType = convertDataType(DataType.DOUBLE);
                    break;
                case DataType.DOUBLE:
                    retDataType = convertDataType(DataType.NUMERIC);
                    break;
                case DataType.NUMERIC:
                    retDataType = convertDataType(DataType.DECIMAL);
                    break;
                case DataType.DECIMAL:
                    if (supportsDataType(DataType.DOUBLE))
                    {
                        retDataType = convertDataType(DataType.DOUBLE);
                    }
                    else if (supportsDataType(DataType.NUMERIC))
                    {
                        retDataType = DataType.NUMERIC;
                    }
                    else
                    {
                        retDataType = getLastConversionFallbackDataType();
                    }
                    break;
                case DataType.VARCHAR:
                    retDataType = getLastConversionFallbackDataType();
                    break;
                default:
                    retDataType = getLastConversionFallbackDataType();
            }
        }
        return retDataType;
    }

    public int getAutoIncrementIndex(XPropertySet _xColPropertySet)
    {
        try
        {
            boolean bleaveloop = false;
            int startindex = 0;
            int curDataType = ((Integer) _xColPropertySet.getPropertyValue("Type")).intValue();
            while (!bleaveloop)
            {
                int i = JavaTools.FieldInIntTable(nDataTypeInfos, curDataType, startindex);
                startindex = i + 1;
                bleaveloop = (i == -1);
                if (!bleaveloop)
                {
                    if (bisAutoIncrementableInfos[i])
                    {
                        return nDataTypeInfos[i];
                    }
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
        return INVALID;

    }

    public boolean isAutoIncrementable(XPropertySet _xColPropertySet)
    {
        return (getAutoIncrementIndex(_xColPropertySet) != INVALID);
    }

    /** finds the first available DataType that can be used as a primary key in a table. 
     * @return The first datatype that also supports Autoincrmentation is taken according to the following list:
     *1) INTEGER
     *2) FLOAT
     *3) REAL
     *4) DOUBLE
     *5) NUMERIC
     *6) DECIMAL         * 
     * If no appropriate datatype is found ther first available numeric type after DataType.INTEGER
     * according to the 'convertDataType' method is returned
     */
    /**TODO the fallback order is the same as implemented in the method 'convertDataType'. 
     * It's not very elegant to have the same intelligence
     * on several spots in the class!!
     * 
     */
    public TypeInfo findAutomaticPrimaryKeyType()
    {
        int nDataType;
        for (int n = 0; n < this.nNumericFallBackList.length; n++)
        {
            nDataType = nNumericFallBackList[n];
            boolean bleaveloop = false;
            int startindex = 0;
            while (!bleaveloop)
            {
                int i = JavaTools.FieldInIntTable(nDataTypeInfos, nDataType, startindex);
                bleaveloop = (i < 0);
                if (!bleaveloop)
                {
                    if (this.bisAutoIncrementableInfos[i])
                    {
                        return new TypeInfo(nDataType, this.sDataTypeNames[i], true);
                    }
                    startindex = i + 1;
                }
                startindex = i + 1;
            }
        }
        // As Autoincrementation is not supported for any numeric datatype we take the first available numeric Type;
        nDataType = convertDataType(DataType.INTEGER);
        return new TypeInfo(nDataType, getDefaultTypeName(nDataType, null), false);
    }
}
