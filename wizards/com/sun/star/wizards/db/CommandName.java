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

import com.sun.star.sdbc.SQLException;
import com.sun.star.uno.Exception;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.PropertyNames;

public class CommandName
{

    protected CommandMetaData oCommandMetaData;
    protected String CatalogName = PropertyNames.EMPTY_STRING;
    protected String SchemaName = PropertyNames.EMPTY_STRING;
    protected String TableName = PropertyNames.EMPTY_STRING;
    protected String DisplayName = PropertyNames.EMPTY_STRING;
    protected String ComposedName = PropertyNames.EMPTY_STRING;
    protected String AliasName = PropertyNames.EMPTY_STRING;
    protected boolean bCatalogAtStart;
    protected String sCatalogSep;
    protected String sIdentifierQuote;
    protected boolean baddQuotation = true;

    public CommandName(CommandMetaData _CommandMetaData, String _DisplayName)
    {
        oCommandMetaData = _CommandMetaData;
        setComposedCommandName(_DisplayName);
    }

    public CommandName(CommandMetaData _CommandMetaData, String _CatalogName, String _SchemaName, String _TableName, boolean _baddQuotation)
    {
        try
        {
            baddQuotation = _baddQuotation;
            oCommandMetaData = _CommandMetaData;
            if ((_CatalogName != null) && (oCommandMetaData.xDBMetaData.supportsCatalogsInTableDefinitions()))
            {
                if (!_CatalogName.equals(PropertyNames.EMPTY_STRING))
                {
                    CatalogName = _CatalogName;
                }
            }
            if ((_SchemaName != null) && (oCommandMetaData.xDBMetaData.supportsSchemasInTableDefinitions()))
            {
                if (!_SchemaName.equals(PropertyNames.EMPTY_STRING))
                {
                    SchemaName = _SchemaName;
                }
            }
            if (_TableName != null)
            {
                if (!_TableName.equals(PropertyNames.EMPTY_STRING))
                {
                    TableName = _TableName;
                }
            }
            setComposedCommandName();
        }
        catch (SQLException e)
        {
            e.printStackTrace(System.out);
        }
    }

    private void setComposedCommandName(String _DisplayName)
    {
        try
        {
            if (this.setMetaDataAttributes())
            {
                this.DisplayName = _DisplayName;
                int iIndex;
                if (oCommandMetaData.xDBMetaData.supportsCatalogsInDataManipulation() == true)
                { // ...dann Catalog mit in TableName
                    iIndex = _DisplayName.indexOf(sCatalogSep);
                    if (iIndex >= 0)
                    {
                        if (bCatalogAtStart == true)
                        {
                            CatalogName = _DisplayName.substring(0, iIndex);
                            _DisplayName = _DisplayName.substring(iIndex + 1, _DisplayName.length());
                        }
                        else
                        {
                            CatalogName = _DisplayName.substring(iIndex + 1, _DisplayName.length());
                            _DisplayName = _DisplayName.substring(0, iIndex);
                        }
                    }
                }
                if (oCommandMetaData.xDBMetaData.supportsSchemasInDataManipulation() == true)
                {
                    String[] NameList;
                    NameList = new String[0];
                    NameList = JavaTools.ArrayoutofString(_DisplayName, ".");
                    SchemaName = NameList[0];
                    TableName = NameList[1]; // TODO Was ist mit diesem Fall: CatalogSep = "." und CatalogName = PropertyNames.EMPTY_STRING
                }
                else
                {
                    TableName = _DisplayName;
                }
                setComposedCommandName();
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    public void setComposedCommandName()
    {
        if (this.setMetaDataAttributes())
        {
            if (CatalogName != null)
            {
                if (!CatalogName.equals(PropertyNames.EMPTY_STRING))
                {
                    if (bCatalogAtStart == true)
                    {
                        ComposedName = quoteName(CatalogName) + sCatalogSep;
                    }
                }
            }
            if (SchemaName != null)
            {
                if (!SchemaName.equals(PropertyNames.EMPTY_STRING))
                {
                    ComposedName += quoteName(SchemaName) + ".";
                }
            }
            if (ComposedName.equals(PropertyNames.EMPTY_STRING))
            {
                ComposedName = quoteName(TableName);
            }
            else
            {
                ComposedName += quoteName(TableName);
            }
            if ((bCatalogAtStart == false) && (CatalogName != null))
            {
                if (!CatalogName.equals(PropertyNames.EMPTY_STRING))
                {
                    ComposedName += sCatalogSep + quoteName(CatalogName);
                }
            }
        }
    }

    private boolean setMetaDataAttributes()
    {
        try
        {
            bCatalogAtStart = oCommandMetaData.xDBMetaData.isCatalogAtStart();
            sCatalogSep = oCommandMetaData.xDBMetaData.getCatalogSeparator();
            sIdentifierQuote = oCommandMetaData.xDBMetaData.getIdentifierQuoteString();
            return true;
        }
        catch (SQLException e)
        {
            e.printStackTrace(System.out);
            return false;
        }
    }

    public String quoteName(String _sName)
    {
        if (baddQuotation)
        {
            return quoteName(_sName, this.oCommandMetaData.getIdentifierQuote());
        }
        else
        {
            return _sName;
        }
    }

    public static String quoteName(String sName, String _sIdentifierQuote)
    {
        if (sName == null)
        {
            sName = PropertyNames.EMPTY_STRING;
        }
        return new StringBuilder(_sIdentifierQuote).append(sName).append(_sIdentifierQuote).toString();
    }

    public void setAliasName(String _AliasName)
    {
        AliasName = _AliasName;
    }

    public String getAliasName()
    {
        return AliasName;
    }

    /**
     * @return Returns the catalogName.
     */
    public String getCatalogName()
    {
        return CatalogName;
    }

    /**
     * @return Returns the composedName.
     */
    public String getComposedName()
    {
        return ComposedName;
    }

    /**
     * @return Returns the displayName.
     */
    public String getDisplayName()
    {
        return DisplayName;
    }

    /**
     * @return Returns the schemaName.
     */
    public String getSchemaName()
    {
        return SchemaName;
    }

    /**
     * @return Returns the tableName.
     */
    public String getTableName()
    {
        return TableName;
    }

    public CommandMetaData getCommandMetaData()
    {
        return oCommandMetaData;
    }
}
