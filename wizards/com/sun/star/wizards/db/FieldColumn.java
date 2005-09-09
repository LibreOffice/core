/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FieldColumn.java,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:24:15 $
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

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.sdbc.DataType;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.*;

public class FieldColumn {
    public Object DefaultValue;
    public String FieldName;
    public String DisplayFieldName;
    public String FieldTitle;
    public String AliasName;
    public int ColIndex;
    private String CommandName;
    public int FieldWidth;
    public int DBFormatKey;
    public int FieldType;
    public int StandardFormatKey;
    public boolean bIsNumberFormat;
    public XPropertySet xColPropertySet;
    public int iType;
    protected int iDateFormatKey;
    protected int iDateTimeFormatKey;
    protected int iNumberFormatKey;
    protected int iTextFormatKey;
    protected int iTimeFormatKey;
    protected int iLogicalFormatKey;

    public FieldColumn(CommandMetaData oCommandMetaData, String _DisplayFieldName) {
        // TODO: xColumns has to be retrieved from the respective table
        setFieldNameAndCommandName(_DisplayFieldName);
        if (CommandName == null){
            DisplayFieldName = FieldName;
            CommandName = oCommandMetaData.getCommandName();
        }
        else
            DisplayFieldName = CommandName + "." + FieldName;
        FieldTitle = FieldName; // oCommandMetaData.getFieldTitle(FieldName);
        //TODO check if the aliasname doesn't occur twice in query
        AliasName = FieldName;
        DBMetaData.CommandObject oTable = oCommandMetaData.getTableByName(CommandName);
        setFormatKeys(oCommandMetaData, oTable.xColumns);
    }

    public FieldColumn(CommandMetaData oCommandMetaData, String _FieldName, String _CommandName) {
        CommandName = _CommandName;
        FieldName = _FieldName;
        DisplayFieldName = FieldName;
        AliasName = FieldName;
        FieldTitle = FieldName;
        //TODO check if the aliasname doesn't occur twice in query
        AliasName = FieldName;
        DBMetaData.CommandObject oTable = oCommandMetaData.getTableByName(CommandName);
        setFormatKeys(oCommandMetaData, oTable.xColumns);
    }





    private void setFormatKeys(CommandMetaData oCommandMetaData, XNameAccess _xColumns){
        try {
            xColPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, _xColumns.getByName(FieldName));
            ColIndex = JavaTools.FieldInList(_xColumns.getElementNames(), FieldName) + 1;
            iType = AnyConverter.toInt(xColPropertySet.getPropertyValue("Type"));
            iDateFormatKey = oCommandMetaData.getNumberFormatter().getDateFormatKey();
            iDateTimeFormatKey = oCommandMetaData.getNumberFormatter().getDateTimeFormatKey();
            iNumberFormatKey = oCommandMetaData.getNumberFormatter().getNumberFormatKey();
            iTextFormatKey = oCommandMetaData.getNumberFormatter().getTextFormatKey();
            iTimeFormatKey = oCommandMetaData.getNumberFormatter().getTimeFormatKey();
            iLogicalFormatKey = oCommandMetaData.getNumberFormatter().getLogicalFormatKey();
            DefaultValue = getTyperelatedFieldData();
        } catch (Exception e) {
            e.printStackTrace(System.out);
        }
    }

    public FieldColumn(CommandMetaData oCommandMetaData, XNameAccess _xColumns, String _DisplayFieldName) {
        FieldName = _DisplayFieldName;
        DisplayFieldName = FieldName;
        ColIndex = JavaTools.FieldInList(_xColumns.getElementNames(), FieldName) + 1;
        setFormatKeys(oCommandMetaData, _xColumns);
    }

    public void setCommandName(String _CommandName) {
        CommandName = _CommandName;
    }

    public String getCommandName() {
        return CommandName;
    }

    public void setFieldNameAndCommandName(String _DisplayName) {
        String[] sFieldMetaData = JavaTools.ArrayoutofString(_DisplayName, ".");
        if (sFieldMetaData.length >= 2) {
            FieldName = sFieldMetaData[sFieldMetaData.length - 1];
            CommandName = "";
            for (int i = 0; i < sFieldMetaData.length - 1; i++) {
                CommandName += sFieldMetaData[i];
                if (i < sFieldMetaData.length - 2)
                    CommandName += ".";
            }
        } else
            FieldName = _DisplayName;
    }

    public static String getFieldName(String _DisplayName) {
        String LocFieldName;
        String[] sFieldMetaData = JavaTools.ArrayoutofString(_DisplayName, ".");
        if (sFieldMetaData.length >= 2)
            LocFieldName = sFieldMetaData[sFieldMetaData.length - 1];
        else
            LocFieldName = _DisplayName;
        return LocFieldName;
    }

    public static String getCommandName(String _DisplayName) {
        String locCommandName = null;
        String[] sFieldMetaData = JavaTools.ArrayoutofString(_DisplayName, ".");
        if (sFieldMetaData.length >= 2) {
            String locfieldname = sFieldMetaData[sFieldMetaData.length - 1];
            locCommandName = _DisplayName.substring(0, _DisplayName.length() - locfieldname.length() - 1);
        }
        return locCommandName;
    }

    public int getFormatKey() {
        try {
            int iKey;
            Object oKey = this.xColPropertySet.getPropertyValue( "FormatKey");
            if (AnyConverter.isVoid(oKey))
                DBFormatKey = StandardFormatKey;
            else
                DBFormatKey = AnyConverter.toInt(oKey);
            return DBFormatKey;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            return StandardFormatKey;
        }
    }

    private Object getTyperelatedFieldData() {
        try {
            FieldType = AnyConverter.toInt(xColPropertySet.getPropertyValue("Type"));
            switch (FieldType) {
                case DataType.BIT : // ==  -7;
                    // Todo: Look if the defaultvalue has been set in the Datasource
                    StandardFormatKey = iLogicalFormatKey;
                    FieldWidth = 5;
                    bIsNumberFormat = true;
                    break;

                case DataType.TINYINT : // ==  -6;
                    StandardFormatKey = iNumberFormatKey;
                    FieldWidth = 5;
                    bIsNumberFormat = true;
                    break;

                case DataType.SMALLINT : // ==   5;
                    StandardFormatKey = iNumberFormatKey;
                    FieldWidth = 5;
                    bIsNumberFormat = true;
                    break;

                case DataType.INTEGER : // ==   4;
                    StandardFormatKey = iNumberFormatKey;
                    FieldWidth = 10;
                    bIsNumberFormat = true;
                    break;

                case DataType.BIGINT : // ==  -5;
                    StandardFormatKey = iNumberFormatKey;
                    FieldWidth = 15;
                    bIsNumberFormat = true;
                    break;

                case DataType.CHAR : // ==   1;
                    StandardFormatKey = iTextFormatKey;
                    getTextFieldWidth(10);
                    bIsNumberFormat = false;
                    break;

                case DataType.VARCHAR : // ==  12;
                    StandardFormatKey = iTextFormatKey;
                    getTextFieldWidth(30);
                    bIsNumberFormat = false;
                    break;

                case DataType.LONGVARCHAR : // ==  -1;
                    StandardFormatKey = iTextFormatKey;
                    getTextFieldWidth(60);
                    bIsNumberFormat = false;
                    break;

                case DataType.NUMERIC : // ==   2;
                    StandardFormatKey = iNumberFormatKey;
                    FieldWidth = 20;
                    bIsNumberFormat = true;
                    break;

                case DataType.DECIMAL : // ==   3;  [mit Nachkommastellen]
                    StandardFormatKey = iNumberFormatKey;
                    FieldWidth = 10 + AnyConverter.toInt(xColPropertySet.getPropertyValue("Scale")) + 1;
                    bIsNumberFormat = true;
                    break;

                case DataType.FLOAT : // ==   6;
                    StandardFormatKey = iNumberFormatKey;
                    FieldWidth = 10 + AnyConverter.toInt(xColPropertySet.getPropertyValue("Scale")) + 1;
                    bIsNumberFormat = true;
                    break;

                case DataType.REAL : // ==   7;
                    StandardFormatKey = iNumberFormatKey;
                    FieldWidth = 10 + AnyConverter.toInt(xColPropertySet.getPropertyValue("Scale")) + 1;
                    bIsNumberFormat = true;
                    break;

                case DataType.DOUBLE : // ==   8;
                    StandardFormatKey = iNumberFormatKey;
                    FieldWidth = 10 + AnyConverter.toInt(xColPropertySet.getPropertyValue("Scale")) + 1;
                    bIsNumberFormat = true;
                    break;

                case DataType.DATE : // ==  91;
                    StandardFormatKey = iDateFormatKey;
                    FieldWidth = 10;
                    bIsNumberFormat = true;
                    break;

                case DataType.TIME : // ==  92;
                    StandardFormatKey = iTimeFormatKey;
                    FieldWidth = 10;
                    bIsNumberFormat = true;
                    break;

                case DataType.TIMESTAMP : // ==  93;
                    StandardFormatKey = iDateTimeFormatKey;
                    FieldWidth = 20;
                    bIsNumberFormat = true;
                    break;
            }
            DBFormatKey = getFormatKey();
            return DefaultValue;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            return null;
        }
    }

    private void getTextFieldWidth(int iWidth) {
        int iNewWidth = iWidth;
        try {
            FieldWidth = AnyConverter.toInt(xColPropertySet.getPropertyValue("Precision"));
            if (FieldWidth > 0) {
                if (FieldWidth > (2 * iWidth))
                    FieldWidth = 2 * iWidth;
                else if (FieldWidth == 0)
                    FieldWidth = iWidth;
            }
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public Object getDefaultValue() {
        switch (FieldType) {
            case DataType.BIT : // ==  -7;
                DefaultValue = (Object) Integer.valueOf("1");
                break;

            case DataType.TINYINT : // ==  -6;
                DefaultValue = (Object) Integer.valueOf("98");
                break;

            case DataType.SMALLINT : // ==   5;
                DefaultValue = (Object) Integer.valueOf("987");
                break;

            case DataType.INTEGER : // ==   4;
                DefaultValue = (Object) Integer.valueOf("9876");
                break;

            case DataType.BIGINT : // ==  -5;
                DefaultValue = (Object) Integer.valueOf("98765");
                break;

            case DataType.CHAR : // ==   1;
                DefaultValue = (Object) String.valueOf('x');
                break;

            case DataType.VARCHAR : // ==  12;
                DefaultValue = (Object) BlindtextCreator.getBlindTextString(FieldTitle, FieldWidth, FieldWidth);
                break;

            case DataType.LONGVARCHAR : // ==  -1;
                DefaultValue = (Object) BlindtextCreator.getBlindTextString(FieldTitle, FieldWidth, FieldWidth);
                break;

            case DataType.NUMERIC : // ==   2;
                DefaultValue = (Object) Double.valueOf("9876.5");
                break;

            case DataType.DECIMAL : // ==   3;  [mit Nachkommastellen]
                DefaultValue = (Object) Double.valueOf("9876.5");
                break;

            case DataType.FLOAT : // ==   6;
                DefaultValue = (Object) Double.valueOf("9876.5");
                break;

            case DataType.REAL : // ==   7;
                DefaultValue = (Object) Double.valueOf("9876.5");
                break;

            case DataType.DOUBLE : // ==   8;
                DefaultValue = (Object) Double.valueOf("9876.54");
                break;

            case DataType.DATE : // ==  91;
                DefaultValue = (Object) Double.valueOf("42510");
                break;

            case DataType.TIME : // ==  92;
                DefaultValue = (Object) Double.valueOf("10");
                break;

            case DataType.TIMESTAMP : // ==  93;
                DefaultValue = (Object) Double.valueOf("5454110");
                break;

            default :
                break;
        }
        return DefaultValue;
    }
}
