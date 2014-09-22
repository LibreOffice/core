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
            e.printStackTrace(System.err);
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
                if (oCommandMetaData.xDBMetaData.supportsCatalogsInDataManipulation())
                { // ...dann Catalog mit in TableName
                    iIndex = _DisplayName.indexOf(sCatalogSep);
                    if (iIndex >= 0)
                    {
                        if (bCatalogAtStart)
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
                if (oCommandMetaData.xDBMetaData.supportsSchemasInDataManipulation())
                {
                    String[] NameList;
                    NameList = new String[0];
                    NameList = JavaTools.ArrayoutofString(_DisplayName, ".");
                    if (NameList.length > 1)
                    {
                        SchemaName = NameList[0];
                        TableName = NameList[1];
                    }
                    else
                    {
                        TableName = _DisplayName;
                    }
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
            exception.printStackTrace(System.err);
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
                    if (bCatalogAtStart)
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
            if ((!bCatalogAtStart) && (CatalogName != null))
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
            e.printStackTrace(System.err);
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
