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
import com.sun.star.beans.*;
import com.sun.star.container.XIndexAccess;
import com.sun.star.sdb.XSingleSelectQueryComposer;
import com.sun.star.sdb.XSingleSelectQueryAnalyzer;
import com.sun.star.ui.dialogs.XExecutableDialog;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.sdbc.SQLException;
import com.sun.star.lang.XInitialization;
import com.sun.star.awt.XWindow;
import com.sun.star.sdb.SQLFilterOperator;

import com.sun.star.wizards.common.*;
import java.util.ArrayList;

public class SQLQueryComposer
{

    private QueryMetaData CurDBMetaData;
    public XSingleSelectQueryAnalyzer m_xQueryAnalyzer;
    private ArrayList<CommandName> composedCommandNames = new ArrayList<CommandName>(1);
    private XSingleSelectQueryComposer m_queryComposer;
    private XMultiServiceFactory xMSF;
    private boolean bincludeGrouping = true;

    public SQLQueryComposer(QueryMetaData _CurDBMetaData)
    {
        try
        {
            setDBMetaData(_CurDBMetaData);
            xMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, CurDBMetaData.DBConnection);
            final Object oQueryComposer = xMSF.createInstance("com.sun.star.sdb.SingleSelectQueryComposer");
            m_xQueryAnalyzer = UnoRuntime.queryInterface(XSingleSelectQueryAnalyzer.class, oQueryComposer);
            m_queryComposer = UnoRuntime.queryInterface(XSingleSelectQueryComposer.class, m_xQueryAnalyzer);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    private boolean addtoSelectClause(String DisplayFieldName) throws SQLException
    {
        return !(bincludeGrouping && CurDBMetaData.xDBMetaData.supportsGroupByUnrelated() && CurDBMetaData.GroupFieldNames != null && JavaTools.FieldInList(CurDBMetaData.GroupFieldNames, DisplayFieldName) > -1);
        }

    public String getSelectClause(boolean _baddAliasFieldNames) throws SQLException
    {
        // getFromClause() must be called first to populate composedCommandNames,
        // but it's idempotent, so let's call it now in case the caller didn't already:
        getFromClause();

        String sSelectBaseClause = "SELECT ";
        StringBuilder sb = new StringBuilder(sSelectBaseClause);
        for (int i = 0; i < CurDBMetaData.FieldColumns.length; i++)
        {
            if (addtoSelectClause(CurDBMetaData.FieldColumns[i].getDisplayFieldName()))
            {
                int iAggregate = CurDBMetaData.getAggregateIndex(CurDBMetaData.FieldColumns[i].getDisplayFieldName());
                if (iAggregate > -1)
                {
                    sb.append(CurDBMetaData.AggregateFieldNames[iAggregate][1]).append("(").append(getComposedAliasDisplayName(CurDBMetaData.AggregateFieldNames[iAggregate][0])).append(")");
                    if (_baddAliasFieldNames)
                    {
                        sb.append(getAliasFieldNameClause(CurDBMetaData.AggregateFieldNames[iAggregate][0]));
                    }
                }
                else
                {
                    sb.append(getComposedAliasDisplayName(CurDBMetaData.FieldColumns[i].getDisplayFieldName()));
                    if (_baddAliasFieldNames)
                    {
                        sb.append(getAliasFieldNameClause(CurDBMetaData.FieldColumns[i].getDisplayFieldName()));
                    }
                }
                sb.append(", ");
            }
        }
        String sSelectClause = sb.toString();
        // TODO: little bit unhandy version of remove the append 'comma' at the end
        if (sSelectClause.equals(sSelectBaseClause))
        {
            sSelectClause = sSelectClause.substring(0, sSelectClause.length() - 1);
        }
        else
        {
            sSelectClause = sSelectClause.substring(0, sSelectClause.length() - 2);
        }
        return sSelectClause;
    }

    private String getAliasFieldNameClause(String _FieldName)
    {
        String FieldTitle = CurDBMetaData.getFieldTitle(_FieldName);
        if (!FieldTitle.equals(_FieldName))
        {
            return " AS " + CommandName.quoteName(FieldTitle, CurDBMetaData.getIdentifierQuote());
        }
        else
        {
            return "";
        }
    }



    public void prependSortingCriteria() throws SQLException
    {
        prependSortingCriteria(false);
    }

    private void prependSortingCriteria(boolean _baddAliasFieldNames) throws SQLException
    {
        XIndexAccess xColumnIndexAccess = m_xQueryAnalyzer.getOrderColumns();
        m_queryComposer.setOrder("");
        for (int i = 0; i < CurDBMetaData.getSortFieldNames().length; i++)
        {
            appendSortingCriterion(i, _baddAliasFieldNames);
        }
        for (int i = 0; i < xColumnIndexAccess.getCount(); i++)
        {
            try
            {
                XPropertySet xColumnPropertySet = UnoRuntime.queryInterface(XPropertySet.class, xColumnIndexAccess.getByIndex(i));
                String sName = (String) xColumnPropertySet.getPropertyValue(PropertyNames.PROPERTY_NAME);
                if (JavaTools.FieldInTable(CurDBMetaData.getSortFieldNames(), sName) == -1)
                {
                    boolean bascend = AnyConverter.toBoolean(xColumnPropertySet.getPropertyValue("IsAscending"));
                    m_queryComposer.appendOrderByColumn(xColumnPropertySet, bascend);
                }
            }
            catch (Exception e)
            {
                e.printStackTrace(System.err);
            }
        }
    }

    private void appendSortingCriterion(int _SortIndex, boolean _baddAliasFieldNames) throws SQLException
    {
        String sSortValue = CurDBMetaData.getSortFieldNames()[_SortIndex][0];
        XPropertySet xColumn = CurDBMetaData.getColumnObjectByFieldName(sSortValue, _baddAliasFieldNames);

        String sSort = "ASC";
        if(CurDBMetaData.getSortFieldNames()[_SortIndex].length > 1)
        {
            sSort = CurDBMetaData.getSortFieldNames()[_SortIndex][1];
        }
        boolean bascend = !(sSort.equals("DESC"));
        m_queryComposer.appendOrderByColumn(xColumn, bascend);
    }

    private void appendSortingcriteria(boolean _baddAliasFieldNames) throws SQLException
    {
        m_queryComposer.setOrder("");
        for (int i = 0; i < CurDBMetaData.getSortFieldNames().length; i++)
        {
            String sSortValue = CurDBMetaData.getSortFieldNames()[i][0];
            int iAggregate = CurDBMetaData.getAggregateIndex(sSortValue);
            if (iAggregate > -1)
            {
                String sOrder = m_xQueryAnalyzer.getOrder();
                if (sOrder.length() > 0)
                {
                    sOrder += ", ";
                }
                sOrder += CurDBMetaData.AggregateFieldNames[iAggregate][1] + "(" + getComposedAliasDisplayName(CurDBMetaData.AggregateFieldNames[iAggregate][0]) + ")";
                sOrder += " " + CurDBMetaData.getSortFieldNames()[i][1];
                m_queryComposer.setOrder(sOrder);
            }
            else
            {
                appendSortingCriterion(i, _baddAliasFieldNames);
            }
        }
    }

    private void appendGroupByColumns(boolean _baddAliasFieldNames) throws SQLException
    {
        for (int i = 0; i < CurDBMetaData.GroupFieldNames.length; i++)
        {
            XPropertySet xColumn = CurDBMetaData.getColumnObjectByFieldName(CurDBMetaData.GroupFieldNames[i], _baddAliasFieldNames);
            m_queryComposer.appendGroupByColumn(xColumn);
        }
    }

    private void setDBMetaData(QueryMetaData _oDBMetaData)
    {
        this.CurDBMetaData = _oDBMetaData;
        updateComposedCommandNames();
    }

    private PropertyValue[][] replaceConditionsByAlias(PropertyValue _filterconditions[][])
    {
        for (int n = 0; n < _filterconditions.length; n++)
        {
            for (int m = 0; m < _filterconditions[n].length; m++)
            {
                final String aliasName = getComposedAliasFieldName(_filterconditions[n][m].Name);
                _filterconditions[n][m].Name = aliasName;
            }
        }
        return _filterconditions;
    }

    public String getQuery()
    {
        return m_xQueryAnalyzer.getQuery();
    }

    private void updateComposedCommandNames()
    {
        composedCommandNames.clear();
        String[] sCommandNames = CurDBMetaData.getIncludedCommandNames();
        for (int i = 0; i < sCommandNames.length; i++)
        {
            CommandName curCommandName = new CommandName(CurDBMetaData, sCommandNames[i]);
            curCommandName.setAliasName(getuniqueAliasName(curCommandName.getTableName()));
            composedCommandNames.add(curCommandName);
        }
    }

    public StringBuilder getFromClause() throws SQLException
    {
        StringBuilder sFromClause = new StringBuilder("FROM");
        String[] sCommandNames = CurDBMetaData.getIncludedCommandNames();
        for (int i = 0; i < sCommandNames.length; i++)
        {
            CommandName curCommandName = getComposedCommandByDisplayName(sCommandNames[i]);
            if (curCommandName == null) {
                throw new SQLException("Error: CommandName unavailable");
            }
            sFromClause.append(" ").append(curCommandName.getComposedName()).append(" ").append(quoteName(curCommandName.getAliasName()));
            if (i < sCommandNames.length - 1)
            {
                sFromClause.append(", ");
            }
        }
        return sFromClause;
    }

    public boolean setQueryCommand(XWindow _xParentWindow, boolean _bincludeGrouping, boolean _baddAliasFieldNames)
    {
        return setQueryCommand(_xParentWindow, _bincludeGrouping, _baddAliasFieldNames, true);
    }

    private boolean setQueryCommand(XWindow _xParentWindow, boolean _bincludeGrouping, boolean _baddAliasFieldNames, boolean addQuery)
    {
        return setQueryCommand(_xParentWindow, _bincludeGrouping, _baddAliasFieldNames, addQuery, false);
    }

    private boolean setQueryCommand(XWindow _xParentWindow, boolean _bincludeGrouping, boolean _baddAliasFieldNames, boolean addQuery, boolean prependSortingCriteria)
    {
        try
        {
            bincludeGrouping = _bincludeGrouping;
            if (addQuery)
            {
                StringBuilder fromClause = getFromClause();
                String sSelectClause = getSelectClause(_baddAliasFieldNames);
                StringBuilder queryclause = new StringBuilder(sSelectClause).append(" ").append(fromClause);
                // TDF#122461: Clean SQL string
                m_xQueryAnalyzer.setQuery(queryclause.toString().replace("\n", "").replace("\r", ""));
                if (CurDBMetaData.getFilterConditions() != null && CurDBMetaData.getFilterConditions().length > 0)
                {
                    CurDBMetaData.setFilterConditions(replaceConditionsByAlias(CurDBMetaData.getFilterConditions()));
                    m_queryComposer.setStructuredFilter(CurDBMetaData.getFilterConditions());
                }
            }
            if (_bincludeGrouping)
            {
                appendGroupByColumns(_baddAliasFieldNames);
                if (CurDBMetaData.GroupByFilterConditions.length > 0)
                {
                    m_queryComposer.setStructuredHavingClause(CurDBMetaData.GroupByFilterConditions);
                }
            }
            if (prependSortingCriteria)
            {
                prependSortingCriteria(_baddAliasFieldNames);
            }
            else
            {
                appendSortingcriteria(_baddAliasFieldNames);
            }

            return true;
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
            displaySQLErrorDialog(exception, _xParentWindow);
            return false;
        }
    }

    private String getComposedAliasDisplayName(String _fieldname)
    {
        FieldColumn CurFieldColumn = CurDBMetaData.getFieldColumnByDisplayName(_fieldname);
        final String curCommandName = CurFieldColumn.getCommandName();
        final String curFieldName = CurFieldColumn.getFieldName();
        CommandName curComposedCommandName = getComposedCommandByDisplayName(curCommandName);
        if (curComposedCommandName == null)
        {
            //return _fieldname;
            if ( curCommandName.length() > 0 )
                return quoteName(curCommandName) + "." + quoteName(curFieldName);
            else
                return quoteName(curFieldName);
        }
        String curAliasName = curComposedCommandName.getAliasName();
        return quoteName(curAliasName) + "." + quoteName(curFieldName);
    }

    private String getComposedAliasFieldName(String _fieldname)
    {
        FieldColumn CurFieldColumn = CurDBMetaData.getFieldColumnByFieldName(_fieldname);
        final String curCommandName = CurFieldColumn.getCommandName();
        final String curFieldName = CurFieldColumn.getFieldName();
        CommandName curComposedCommandName = getComposedCommandByDisplayName(curCommandName);
        if (curComposedCommandName == null)
        {
            //return _fieldname;
            if ( curCommandName.length() > 0 )
                return quoteName(curCommandName) + "." + quoteName(curFieldName);
            else
                return quoteName(curFieldName);
        }
        String curAliasName = curComposedCommandName.getAliasName();
        return quoteName(curAliasName) + "." + quoteName(curFieldName);
    }

    private CommandName getComposedCommandByAliasName(String _AliasName)
    {
        if (composedCommandNames != null)
        {
            for (CommandName commandName : composedCommandNames)
            {
                if (commandName.getAliasName().equals(_AliasName))
                {
                    return commandName;
                }
            }
        }
        return null;
    }

    private CommandName getComposedCommandByDisplayName(String _DisplayName)
    {
        if (composedCommandNames != null)
        {
            for (CommandName commandName : composedCommandNames)
            {
                if (commandName.getDisplayName().equals(_DisplayName))
                {
                    return commandName;
                }
            }
        }
        return null;
    }

    private String getuniqueAliasName(String _TableName)
    {
        int a = 0;
        String AliasName = "";
        boolean bAliasNameexists = true;
        String locAliasName = _TableName;
        while (bAliasNameexists)
        {
            bAliasNameexists = (getComposedCommandByAliasName(locAliasName) != null);
            if (bAliasNameexists)
            {
                a++;
                locAliasName = _TableName + "_" + a;
            }
            else
            {
                AliasName = locAliasName;
            }
        }
        return AliasName;
    }

    private String quoteName(String _sname)
    {
        return CommandName.quoteName(_sname, CurDBMetaData.getIdentifierQuote());
    }

    private void displaySQLErrorDialog(Exception _exception, XWindow _xParentWindow)
    {
        try
        {
            Object oErrorDialog = CurDBMetaData.xMSF.createInstance("com.sun.star.sdb.ErrorMessageDialog");
            XInitialization xInitialize = UnoRuntime.queryInterface(XInitialization.class, oErrorDialog);
            XExecutableDialog xExecute = UnoRuntime.queryInterface(XExecutableDialog.class, oErrorDialog);
            PropertyValue[] rDispatchArguments = new PropertyValue[3];
            rDispatchArguments[0] = Properties.createProperty(PropertyNames.PROPERTY_TITLE, Configuration.getProductName(CurDBMetaData.xMSF) + " Base");
            rDispatchArguments[1] = Properties.createProperty("ParentWindow", _xParentWindow);
            rDispatchArguments[2] = Properties.createProperty("SQLException", _exception);
            xInitialize.initialize(rDispatchArguments);
            xExecute.execute();
            //TODO dispose???
        }
        catch (Exception typeexception)
        {
            typeexception.printStackTrace(System.err);
        }
    }

    public XSingleSelectQueryComposer getQueryComposer()
    {
        return m_queryComposer;
    }
}
