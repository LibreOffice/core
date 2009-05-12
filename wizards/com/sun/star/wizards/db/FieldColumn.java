/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FieldColumn.java,v $
 * $Revision: 1.13 $
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

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
// import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.sdbc.DataType;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.*;

public class FieldColumn
{

    public Object DefaultValue;
    public String m_sFieldName;
    private String m_sDisplayFieldName;
    private String FieldTitle;
    public int ColIndex;
    private String m_sCommandName;
    public int FieldWidth;
    public int DBFormatKey;
    public int FieldType;
    public int StandardFormatKey;
    public boolean bIsNumberFormat;
    public XPropertySet m_xColPropertySet;
    public int iType;
    protected int iDateFormatKey;
    protected int iDateTimeFormatKey;
    protected int iNumberFormatKey;
    protected int iTextFormatKey;
    protected int iTimeFormatKey;
    protected int iLogicalFormatKey;

    public FieldColumn(CommandMetaData oCommandMetaData, String _DisplayFieldName)
    {
        m_sDisplayFieldName = _DisplayFieldName;
        m_sCommandName = oCommandMetaData.getCommandName();
        m_sFieldName = getOnlyFieldName(m_sDisplayFieldName, m_sCommandName);
// TODO: could be wrong here!
        FieldTitle = _DisplayFieldName; // oCommandMetaData.getFieldTitle(m_sFieldName);
        FieldTitle = m_sFieldName;
        DBMetaData.CommandObject oTable = oCommandMetaData.getTableByName(m_sCommandName);
        setFormatKeys(oCommandMetaData, oTable.xColumns);
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
        DBMetaData.CommandObject oTable = oCommandMetaData.getTableByName(m_sCommandName);
        setFormatKeys(oCommandMetaData, oTable.xColumns);
    }

    public FieldColumn(CommandMetaData oCommandMetaData, XNameAccess _xColumns, String _FieldName)
    {
        m_sFieldName = _FieldName;
        FieldTitle = m_sFieldName;
        m_sDisplayFieldName = m_sFieldName;
        ColIndex = JavaTools.FieldInList(_xColumns.getElementNames(), m_sFieldName) + 1;
        setFormatKeys(oCommandMetaData, _xColumns);
    }

    /**
     * Remove the pre name, we want the name after the 'dot'
     * @param _DisplayFieldName
     * @param _CommandName
     * @return
     */
    private String getOnlyFieldName(String _DisplayFieldName, String _CommandName)
    {
        final String sName = _DisplayFieldName.substring(_CommandName.length() + 1, _DisplayFieldName.length());
        return sName;
    }

    public static String composeDisplayFieldName(String _sCommandName, String _sFieldName)
    {
        return _sCommandName + "." + _sFieldName;
    }

    private void setFormatKeys(CommandMetaData oCommandMetaData, XNameAccess _xColumns)
    {
        try
        {
            m_xColPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, _xColumns.getByName(m_sFieldName));
            ColIndex = JavaTools.FieldInList(_xColumns.getElementNames(), m_sFieldName) + 1;
            iType = AnyConverter.toInt(m_xColPropertySet.getPropertyValue("Type"));

            final NumberFormatter aNumberFormatter = oCommandMetaData.getNumberFormatter();
            iDateFormatKey = aNumberFormatter.getDateFormatKey();
            iDateTimeFormatKey = aNumberFormatter.getDateTimeFormatKey();
            iNumberFormatKey = aNumberFormatter.getNumberFormatKey();
            iTextFormatKey = aNumberFormatter.getTextFormatKey();
            iTimeFormatKey = aNumberFormatter.getTimeFormatKey();
            iLogicalFormatKey = aNumberFormatter.getLogicalFormatKey();

            DefaultValue = getTyperelatedFieldData();
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    public XPropertySet getXColumnPropertySet()
    {
        return m_xColPropertySet;
    }

    public void setCommandName(String _CommandName)
    {
        m_sCommandName = _CommandName;
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

    public static String getCommandName(String _DisplayName)
    {
        String locCommandName = null;
        String[] sFieldMetaData = JavaTools.ArrayoutofString(_DisplayName, ".");
        if (sFieldMetaData.length >= 2)
        {
            String locfieldname = sFieldMetaData[sFieldMetaData.length - 1];
            locCommandName = _DisplayName.substring(0, _DisplayName.length() - locfieldname.length() - 1);
        }
        return locCommandName;
    }

    public int getFormatKey()
    {
        try
        {
            // int iKey;
            int nDBFormatKey;
            Object oKey = m_xColPropertySet.getPropertyValue("FormatKey");
            if (AnyConverter.isVoid(oKey))
            {
                nDBFormatKey = StandardFormatKey;
            }
            else
            {
                nDBFormatKey = AnyConverter.toInt(oKey);
            }
            return nDBFormatKey;
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
            return StandardFormatKey;
        }
    }

    public boolean isBoolean()
    {
        boolean bIsBoolean = false;
        try
        {
            FieldType = AnyConverter.toInt(m_xColPropertySet.getPropertyValue("Type"));
            switch (FieldType)
            {
                case DataType.BIT: // ==  -7;
                case DataType.BOOLEAN:
                    bIsBoolean = true;
                    break;
                default:
                    bIsBoolean = false;
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
        return bIsBoolean;
    }

    private Object getTyperelatedFieldData()
    {
        try
        {
            FieldType = AnyConverter.toInt(m_xColPropertySet.getPropertyValue("Type"));
            switch (FieldType)
            {
                case DataType.BIT: // ==  -7;
                case DataType.BOOLEAN:
                    // Todo: Look if the defaultvalue has been set in the Datasource
                    StandardFormatKey = iLogicalFormatKey;
                    FieldWidth = 5;
                    bIsNumberFormat = true;
                    break;

                case DataType.TINYINT: // ==  -6;
                    StandardFormatKey = iNumberFormatKey;
                    FieldWidth = 5;
                    bIsNumberFormat = true;
                    break;

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
                    StandardFormatKey = iNumberFormatKey;
                    FieldWidth = 10 + AnyConverter.toInt(m_xColPropertySet.getPropertyValue("Scale")) + 1;
                    bIsNumberFormat = true;
                    break;

                case DataType.FLOAT: // ==   6;
                    StandardFormatKey = iNumberFormatKey;
                    FieldWidth = 10 + AnyConverter.toInt(m_xColPropertySet.getPropertyValue("Scale")) + 1;
                    bIsNumberFormat = true;
                    break;

                case DataType.REAL: // ==   7;
                    StandardFormatKey = iNumberFormatKey;
                    FieldWidth = 10 + AnyConverter.toInt(m_xColPropertySet.getPropertyValue("Scale")) + 1;
                    bIsNumberFormat = true;
                    break;

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
            DBFormatKey = getFormatKey();
            // TODO: the DefaultValue is not set in this function!!!
            return DefaultValue;
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
            return null;
        }
    }

    private void getTextFieldWidth(int iWidth)
    {
        // int iNewWidth = iWidth;
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
            exception.printStackTrace(System.out);
        }
    }

    public Object getDefaultValue()
    {
        switch (FieldType)
        {
            case DataType.BIT: // ==  -7;
            case DataType.BOOLEAN:
                DefaultValue = (Object) Integer.valueOf("1");
                break;

            case DataType.TINYINT: // ==  -6;
                DefaultValue = (Object) Integer.valueOf("98");
                break;

            case DataType.SMALLINT: // ==   5;
                DefaultValue = (Object) Integer.valueOf("987");
                break;

            case DataType.INTEGER: // ==   4;
                DefaultValue = (Object) Integer.valueOf("9876");
                break;

            case DataType.BIGINT: // ==  -5;
                DefaultValue = (Object) Integer.valueOf("98765");
                break;

            case DataType.CHAR: // ==   1;
                DefaultValue = (Object) String.valueOf('x');
                break;

            case DataType.VARCHAR: // ==  12;
                DefaultValue = (Object) BlindtextCreator.getBlindTextString(FieldTitle, FieldWidth, FieldWidth);
                break;

            case DataType.LONGVARCHAR: // ==  -1;
                DefaultValue = (Object) BlindtextCreator.getBlindTextString(FieldTitle, FieldWidth, FieldWidth);
                break;

            case DataType.NUMERIC: // ==   2;
                DefaultValue = (Object) Double.valueOf("9876.5");
                break;

            case DataType.DECIMAL: // ==   3;  [mit Nachkommastellen]
                DefaultValue = (Object) Double.valueOf("9876.5");
                break;

            case DataType.FLOAT: // ==   6;
                DefaultValue = (Object) Double.valueOf("9876.5");
                break;

            case DataType.REAL: // ==   7;
                DefaultValue = (Object) Double.valueOf("9876.5");
                break;

            case DataType.DOUBLE: // ==   8;
                DefaultValue = (Object) Double.valueOf("9876.54");
                break;

            case DataType.DATE: // ==  91;
                DefaultValue = (Object) Double.valueOf("42510");
                break;

            case DataType.TIME: // ==  92;
                DefaultValue = (Object) Double.valueOf("10");
                break;

            case DataType.TIMESTAMP: // ==  93;
                DefaultValue = (Object) Double.valueOf("5454110");
                break;

            default:
                break;
        }
        return DefaultValue;
    }
}
