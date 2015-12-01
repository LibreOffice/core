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

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.sdbc.DataType;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.*;

public class FieldColumn
{
    protected int ColIndex;

    private Object DefaultValue;
    private String m_sFieldName;
    private String m_sDisplayFieldName;
    private String FieldTitle;
    private String m_sCommandName;
    private int m_nDBFormatKey;
    private int m_nFieldType;
    private XPropertySet m_xColPropertySet;

    // field meta data
    private int FieldWidth;
    private boolean bIsNumberFormat;

    private static boolean bFormatKeysInitialized = false;
    private static int iDateFormatKey;
    private static int iDateTimeFormatKey;
    private static int iNumberFormatKey;
    private static int iTextFormatKey;
    private static int iTimeFormatKey;
    private static int iLogicalFormatKey;

    private CommandMetaData m_aCommandMetaData;

    public FieldColumn(CommandMetaData oCommandMetaData, String _DisplayFieldName)
    {
        m_sDisplayFieldName = _DisplayFieldName;
        m_sCommandName = oCommandMetaData.getCommandName();
        m_sFieldName = getOnlyFieldName(m_sDisplayFieldName, m_sCommandName);
// TODO: could be wrong here!
//        FieldTitle = _DisplayFieldName; // oCommandMetaData.getFieldTitle(m_sFieldName);
        FieldTitle = m_sFieldName;
        DBMetaData.CommandObject oTable = oCommandMetaData.getTableByName(m_sCommandName);
        initializeFormatKeys(oCommandMetaData, oTable.getColumns());
    }

    public FieldColumn(CommandMetaData oCommandMetaData, String _FieldName, String _CommandName, boolean _bInstantiateByDisplayName)
    {
        m_sCommandName = _CommandName;
        if (_bInstantiateByDisplayName)
        {
            m_sDisplayFieldName = _FieldName;
            m_sFieldName = getOnlyFieldName(_FieldName, _CommandName);
        }
        else
        {
            m_sFieldName = _FieldName;
            m_sDisplayFieldName = composeDisplayFieldName(_CommandName, m_sFieldName);
        }
        FieldTitle = m_sFieldName;
        m_aCommandMetaData = oCommandMetaData;
    }

    public FieldColumn(CommandMetaData oCommandMetaData, XNameAccess _xColumns, String _FieldName)
    {
        m_sFieldName = _FieldName;
//        FieldTitle = m_sFieldName;
        m_sDisplayFieldName = m_sFieldName;
        ColIndex = JavaTools.FieldInList(_xColumns.getElementNames(), m_sFieldName) + 1;
        initializeFormatKeys(oCommandMetaData, _xColumns);
        try
        {
            m_sCommandName = (String)m_xColPropertySet.getPropertyValue("TableName");
        }
        catch (com.sun.star.beans.UnknownPropertyException e)
        {
        }
        catch (com.sun.star.lang.WrappedTargetException e)
        {
        }
    }

    public int getFieldType()
    {
        if (m_nFieldType == 0)
        {
            DBMetaData.CommandObject oTable = m_aCommandMetaData.getTableByName(m_sCommandName);
            initializeFormatKeys(m_aCommandMetaData, oTable.getColumns());
        }
        return m_nFieldType;
    }

    public int getFieldWidth()
    {
        getFieldType(); // will collect meta data 'bout the column, if not already done so
        return FieldWidth;
    }

    public int getDBFormatKey()
    {
        getFieldType(); // will collect meta data 'bout the column, if not already done so
        return m_nDBFormatKey;
    }

    public boolean isNumberFormat()
    {
        getFieldType(); // will collect meta data 'bout the column, if not already done so
        return bIsNumberFormat;
    }

    /**
     * Remove the pre name, we want the name after the 'dot'
     * @param _DisplayFieldName
     * @param _CommandName
     * @return
     */
    private String getOnlyFieldName(String _DisplayFieldName, String _CommandName)
    {
        return _DisplayFieldName.substring(_CommandName.length() + 1);
    }

    public static String composeDisplayFieldName(String _sCommandName, String _sFieldName)
    {
        return _sCommandName + "." + _sFieldName;
    }

    private void initializeFormatKeys(CommandMetaData oCommandMetaData, XNameAccess _xColumns)
    {
        try
        {
            if (!bFormatKeysInitialized)
            {
                final NumberFormatter aNumberFormatter = oCommandMetaData.getNumberFormatter();

                iDateFormatKey = aNumberFormatter.getDateFormatKey();
                iDateTimeFormatKey = aNumberFormatter.getDateTimeFormatKey();
                iNumberFormatKey = aNumberFormatter.getNumberFormatKey();
                iTextFormatKey = aNumberFormatter.getTextFormatKey();
                iTimeFormatKey = aNumberFormatter.getTimeFormatKey();
                iLogicalFormatKey = aNumberFormatter.getLogicalFormatKey();
                bFormatKeysInitialized = true;
            }

            m_xColPropertySet = UnoRuntime.queryInterface(XPropertySet.class, _xColumns.getByName(m_sFieldName));
            ColIndex = JavaTools.FieldInList(_xColumns.getElementNames(), m_sFieldName) + 1;
            m_nFieldType = AnyConverter.toInt(m_xColPropertySet.getPropertyValue("Type"));
            getTyperelatedFieldData();
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
    }

    public XPropertySet getXColumnPropertySet()
    {
        getFieldType(); // will collect meta data 'bout the column, if not already done so
        return m_xColPropertySet;
    }

    public String getDisplayFieldName()
    {
        return m_sDisplayFieldName;
    }

    public String getCommandName()
    {
        return m_sCommandName;
    }

    public String getFieldName()
    {
        return m_sFieldName;
    }

    public String getFieldTitle()
    {
        return FieldTitle;
    }

    public void setFieldTitle(String _sTitle)
    {
        FieldTitle = _sTitle;
    }



    public boolean isBoolean()
    {
        boolean bIsBoolean = false;
        switch ( getFieldType() )
        {
            case DataType.BIT: // ==  -7;
            case DataType.BOOLEAN:
                bIsBoolean = true;
                break;
            default:
                bIsBoolean = false;
        }
        return bIsBoolean;
    }

    private void getTyperelatedFieldData()
    {
        int StandardFormatKey = 0;
        try
        {
            switch ( getFieldType() )
            {
                case DataType.BIT: // ==  -7;
                case DataType.BOOLEAN:
                    // Todo: Look if the defaultvalue has been set in the Datasource
                    StandardFormatKey = iLogicalFormatKey;
                    FieldWidth = 5;
                    bIsNumberFormat = true;
                    break;

                case DataType.TINYINT: // ==  -6;
                case DataType.SMALLINT: // ==   5;
                    StandardFormatKey = iNumberFormatKey;
                    FieldWidth = 5;
                    bIsNumberFormat = true;
                    break;

                case DataType.INTEGER: // ==   4;
                    StandardFormatKey = iNumberFormatKey;
                    FieldWidth = 10;
                    bIsNumberFormat = true;
                    break;

                case DataType.BIGINT: // ==  -5;
                    StandardFormatKey = iNumberFormatKey;
                    FieldWidth = 15;
                    bIsNumberFormat = true;
                    break;

                case DataType.CHAR: // ==   1;
                    StandardFormatKey = iTextFormatKey;
                    getTextFieldWidth(10);
                    bIsNumberFormat = false;
                    break;

                case DataType.VARCHAR: // ==  12;
                    StandardFormatKey = iTextFormatKey;
                    getTextFieldWidth(30);
                    bIsNumberFormat = false;
                    break;

                case DataType.LONGVARCHAR: // ==  -1;
                    StandardFormatKey = iTextFormatKey;
                    getTextFieldWidth(60);
                    bIsNumberFormat = false;
                    break;

                case DataType.NUMERIC: // ==   2;
                    StandardFormatKey = iNumberFormatKey;
                    FieldWidth = 20;
                    bIsNumberFormat = true;
                    break;

                case DataType.DECIMAL: // ==   3;  [mit Nachkommastellen]
                case DataType.FLOAT: // ==   6;
                case DataType.REAL: // ==   7;
                case DataType.DOUBLE: // ==   8;
                    StandardFormatKey = iNumberFormatKey;
                    FieldWidth = 10 + AnyConverter.toInt(m_xColPropertySet.getPropertyValue("Scale")) + 1;
                    bIsNumberFormat = true;
                    break;

                case DataType.DATE: // ==  91;
                    StandardFormatKey = iDateFormatKey;
                    FieldWidth = 10;
                    bIsNumberFormat = true;
                    break;

                case DataType.TIME: // ==  92;
                    StandardFormatKey = iTimeFormatKey;
                    FieldWidth = 10;
                    bIsNumberFormat = true;
                    break;

                case DataType.TIMESTAMP: // ==  93;
                    StandardFormatKey = iDateTimeFormatKey;
                    FieldWidth = 20;
                    bIsNumberFormat = true;
                    break;
            }

            Object oKey = m_xColPropertySet.getPropertyValue("FormatKey");
            if (AnyConverter.isVoid(oKey))
            {
                m_nDBFormatKey = StandardFormatKey;
            }
            else
            {
                m_nDBFormatKey = AnyConverter.toInt(oKey);
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
            m_nDBFormatKey = StandardFormatKey;
        }
    }

    private void getTextFieldWidth(int iWidth)
    {
        try
        {
            FieldWidth = AnyConverter.toInt(m_xColPropertySet.getPropertyValue("Precision"));
            if (FieldWidth > 0)
            {
                if (FieldWidth > (2 * iWidth))
                {
                    FieldWidth = 2 * iWidth;
                }
                else if (FieldWidth == 0)
                {
                    FieldWidth = iWidth;
                }
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    private void initDefaultValue()
    {
        switch (getFieldType())
        {
            case DataType.BIT: // ==  -7;
            case DataType.BOOLEAN:
                DefaultValue = Integer.valueOf("1");
                break;

            case DataType.TINYINT: // ==  -6;
                DefaultValue = Integer.valueOf("98");
                break;

            case DataType.SMALLINT: // ==   5;
                DefaultValue = Integer.valueOf("987");
                break;

            case DataType.INTEGER: // ==   4;
                DefaultValue = Integer.valueOf("9876");
                break;

            case DataType.BIGINT: // ==  -5;
                DefaultValue = Integer.valueOf("98765");
                break;

            case DataType.CHAR: // ==   1;
                DefaultValue = String.valueOf('x');
                break;

            case DataType.VARCHAR: // ==  12;
            case DataType.LONGVARCHAR: // ==  -1;
                DefaultValue = BlindtextCreator.getBlindTextString(FieldTitle, FieldWidth);
                break;

            case DataType.NUMERIC: // ==   2;
            case DataType.DECIMAL: // ==   3;  [mit Nachkommastellen]
            case DataType.FLOAT: // ==   6;
            case DataType.REAL: // ==   7;
            case DataType.DOUBLE: // ==   8;
                DefaultValue = Double.valueOf("9876.54");
                break;

            case DataType.DATE: // ==  91;
                DefaultValue = Double.valueOf("42510");
                break;

            case DataType.TIME: // ==  92;
                DefaultValue = Double.valueOf("10");
                break;

            case DataType.TIMESTAMP: // ==  93;
                DefaultValue = Double.valueOf("5454110");
                break;

            default:
                break;
        }
    }

    public Object getDefaultValue()
    {
        if ( DefaultValue == null )
            initDefaultValue();
        return DefaultValue;
    }

}
