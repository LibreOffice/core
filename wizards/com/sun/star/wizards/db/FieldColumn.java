/*************************************************************************
*
*  $RCSfile: FieldColumn.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: kz $ $Date: 2004-05-19 12:40:39 $
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

import com.sun.star.sdbc.DataType;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
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
    public Object oField;
    public int iType;
    protected int iDateFormatKey;
    protected int iDateTimeFormatKey;
    protected int iNumberFormatKey;
    protected int iTextFormatKey;
    protected int iTimeFormatKey;
    protected int iLogicalFormatKey;

    public FieldColumn(CommandMetaData oCommandMetaData, String _DisplayFieldName) {
        try {
            // TODO: xColumns has to be retrieved from the respective table
            setFieldNameAndCommandName(_DisplayFieldName);
            FieldTitle = FieldName; // oCommandMetaData.getFieldTitle(FieldName);
            //TODO check if the aliasname doesn't occur twice in query
            AliasName = FieldName;
            DBMetaData.CommandObject oTable = oCommandMetaData.getTableByName(CommandName);
            oField = oTable.xColumns.getByName(FieldName);
            ColIndex = JavaTools.FieldInList(oTable.xColumns.getElementNames(), FieldName) + 1;
            iType = AnyConverter.toInt(Helper.getUnoPropertyValue(oField, "Type"));
            DisplayFieldName = CommandName + "." + FieldName;
            iDateFormatKey = oCommandMetaData.iDateFormatKey;
            iDateTimeFormatKey = oCommandMetaData.iDateTimeFormatKey;
            iNumberFormatKey = oCommandMetaData.iNumberFormatKey;
            iTextFormatKey = oCommandMetaData.iTextFormatKey;
            iTimeFormatKey = oCommandMetaData.iTimeFormatKey;
            iLogicalFormatKey = oCommandMetaData.iLogicalFormatKey;
            DefaultValue = getTyperelatedFieldData();
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
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
            Object oKey = Helper.getUnoPropertyValue(oField, "FormatKey");
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
            FieldType = AnyConverter.toInt(Helper.getUnoPropertyValue(oField, "Type"));
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
                    FieldWidth = 10 + AnyConverter.toInt(Helper.getUnoPropertyValue(oField, "Scale")) + 1;
                    bIsNumberFormat = true;
                    break;

                case DataType.FLOAT : // ==   6;
                    StandardFormatKey = iNumberFormatKey;
                    FieldWidth = 10 + AnyConverter.toInt(Helper.getUnoPropertyValue(oField, "Scale")) + 1;
                    bIsNumberFormat = true;
                    break;

                case DataType.REAL : // ==   7;
                    StandardFormatKey = iNumberFormatKey;
                    FieldWidth = 10 + AnyConverter.toInt(Helper.getUnoPropertyValue(oField, "Scale")) + 1;
                    bIsNumberFormat = true;
                    break;

                case DataType.DOUBLE : // ==   8;
                    StandardFormatKey = iNumberFormatKey;
                    FieldWidth = 10 + AnyConverter.toInt(Helper.getUnoPropertyValue(oField, "Scale")) + 1;
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
        } catch (com.sun.star.lang.IllegalArgumentException exception) {
            exception.printStackTrace(System.out);
            return null;
        }
    }

    private void getTextFieldWidth(int iWidth) {
        int iNewWidth = iWidth;
        try {
            FieldWidth = AnyConverter.toInt(Helper.getUnoPropertyValue(oField, "Precision"));
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
