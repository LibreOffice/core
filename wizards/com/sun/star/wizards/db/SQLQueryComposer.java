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


// import com.sun.star.lang.IllegalArgumentException;
// import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.beans.*;
// import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.sdbcx.XColumnsSupplier;
// import com.sun.star.sdb.XColumn;
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

    public XColumnsSupplier xColSuppl;
    // XSQLQueryComposer xSQLQueryComposer;
    QueryMetaData CurDBMetaData;
    // String m_sSelectClause;
    // String m_sFromClause;
    public XSingleSelectQueryAnalyzer m_xQueryAnalyzer;
    ArrayList<CommandName> composedCommandNames = new ArrayList<CommandName>(1);
    private XSingleSelectQueryComposer m_queryComposer;
    XMultiServiceFactory xMSF;
    boolean bincludeGrouping = true;

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
        String sSelectBaseClause = "SELECT ";
        String sSelectClause = sSelectBaseClause;
        for (int i = 0; i < CurDBMetaData.FieldColumns.length; i++)
        {
            if (addtoSelectClause(CurDBMetaData.FieldColumns[i].getDisplayFieldName()))
            {
                int iAggregate = CurDBMetaData.getAggregateIndex(CurDBMetaData.FieldColumns[i].getDisplayFieldName());
                if (iAggregate > -1)
                {
                    sSelectClause += CurDBMetaData.AggregateFieldNames[iAggregate][1] + "(" + getComposedAliasFieldName(CurDBMetaData.AggregateFieldNames[iAggregate][0]) + ")";
                    if (_baddAliasFieldNames)
                    {
                        sSelectClause += getAliasFieldNameClause(CurDBMetaData.AggregateFieldNames[iAggregate][0]);
                    }
                }
                else
                {
                    sSelectClause += getComposedAliasFieldName(CurDBMetaData.FieldColumns[i].getDisplayFieldName());
                    if (_baddAliasFieldNames)
                    {
                        sSelectClause += getAliasFieldNameClause(CurDBMetaData.FieldColumns[i].getDisplayFieldName());
                    }
                }
                sSelectClause += ", ";
            }
        }
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

    public String getAliasFieldNameClause(String _FieldName)
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

    public void appendFilterConditions() throws SQLException
    {
        try
        {
            for (int i = 0; i < CurDBMetaData.getFilterConditions().length; i++)
            {
                m_queryComposer.setStructuredFilter(CurDBMetaData.getFilterConditions());
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public void prependSortingCriteria() throws SQLException
    {
        prependSortingCriteria(false);
    }

    public void prependSortingCriteria(boolean _baddAliasFieldNames) throws SQLException
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

    public void appendSortingcriteria(boolean _baddAliasFieldNames) throws SQLException
    {
        String sOrder = "";
        m_queryComposer.setOrder("");
        for (int i = 0; i < CurDBMetaData.getSortFieldNames().length; i++)
        {
            String sSortValue = CurDBMetaData.getSortFieldNames()[i][0];
            int iAggregate = CurDBMetaData.getAggregateIndex(sSortValue);
            if (iAggregate > -1)
            {
                sOrder = m_xQueryAnalyzer.getOrder();
                if (sOrder.length() > 0)
                {
                    sOrder += ", ";
                }
                sOrder += CurDBMetaData.AggregateFieldNames[iAggregate][1] + "(" + getComposedAliasFieldName(CurDBMetaData.AggregateFieldNames[iAggregate][0]) + ")";
                sOrder += " " + CurDBMetaData.getSortFieldNames()[i][1];
                m_queryComposer.setOrder(sOrder);
            }
            else
            {
                appendSortingCriterion(i, _baddAliasFieldNames);
            }
            sOrder = m_xQueryAnalyzer.getOrder();
        }
        // just for debug!
        sOrder = m_queryComposer.getOrder();
    }

    public void appendGroupByColumns(boolean _baddAliasFieldNames) throws SQLException
    {
        for (int i = 0; i < CurDBMetaData.GroupFieldNames.length; i++)
        {
            XPropertySet xColumn = CurDBMetaData.getColumnObjectByFieldName(CurDBMetaData.GroupFieldNames[i], _baddAliasFieldNames);
            m_queryComposer.appendGroupByColumn(xColumn);
        }
    }

    public void setDBMetaData(QueryMetaData _oDBMetaData)
    {
        this.CurDBMetaData = _oDBMetaData;
        updateComposedCommandNames();
    }

    private PropertyValue[][] replaceConditionsByAlias(PropertyValue _filterconditions[][])
    {
        XColumnsSupplier columnSup = UnoRuntime.queryInterface(XColumnsSupplier.class, m_xQueryAnalyzer);
        XNameAccess columns = columnSup.getColumns();
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

    public StringBuilder getFromClause()
    {
        StringBuilder sFromClause = new StringBuilder("FROM");
        String[] sCommandNames = CurDBMetaData.getIncludedCommandNames();
        for (int i = 0; i < sCommandNames.length; i++)
        {
            CommandName curCommandName = getComposedCommandByDisplayName(sCommandNames[i]);
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

    public boolean setQueryCommand(XWindow _xParentWindow, boolean _bincludeGrouping, boolean _baddAliasFieldNames, boolean addQuery)
    {
        return setQueryCommand(_xParentWindow, _bincludeGrouping, _baddAliasFieldNames, addQuery, false);
    }

    public boolean setQueryCommand(XWindow _xParentWindow, boolean _bincludeGrouping, boolean _baddAliasFieldNames, boolean addQuery, boolean prependSortingCriteria)
    {
        try
        {
            bincludeGrouping = _bincludeGrouping;
            if (addQuery)
            {
                StringBuilder fromClause = getFromClause();
                String sSelectClause = getSelectClause(_baddAliasFieldNames);
                StringBuilder queryclause = new StringBuilder(sSelectClause).append(" ").append(fromClause);
                m_xQueryAnalyzer.setQuery(queryclause.toString());
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

    private String getComposedAliasFieldName(String _fieldname)
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

    public CommandName getComposedCommandByDisplayName(String _DisplayName)
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

    public String getuniqueAliasName(String _TableName)
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
                locAliasName = _TableName + "_" + String.valueOf(a);
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

    public void displaySQLErrorDialog(Exception _exception, XWindow _xParentWindow)
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

    /**
     * retrieves a normalized structured filter
     *
     * <p>XSingleSelectQueryComposer.getStructuredFilter has a strange habit of returning the predicate (equal, not equal, etc)
     * effectively twice: Once as SQLFilterOperator, and once in the value. That is, if you have a term "column <> 3", then
     * you'll get an SQLFilterOperator.NOT_EQUAL (which is fine), <strong>and</strong> the textual value of the condition
     * will read "<> 3". The latter is strange enough, but even more strange is that this behavior is not even consistent:
     * for SQLFilterOperator.EQUAL, the "=" sign is not include in the textual value.</p>
     *
     * <p>To abstract from this weirdness, use this function here, which strips the unwanted tokens from the textual value
     * representation.</p>
     */
    public PropertyValue[][] getNormalizedStructuredFilter()
    {
        final PropertyValue[][] structuredFilter = m_queryComposer.getStructuredFilter();
        for (int i = 0; i < structuredFilter.length; ++i)
        {
            for (int j = 0; j < structuredFilter[i].length; ++j)
            {
                if (!(structuredFilter[i][j].Value instanceof String))
                {
                    continue;
                }
                final StringBuffer textualValue = new StringBuffer((String) structuredFilter[i][j].Value);
                switch (structuredFilter[i][j].Handle)
                {
                    case SQLFilterOperator.EQUAL:
                        break;
                    case SQLFilterOperator.NOT_EQUAL:
                    case SQLFilterOperator.LESS_EQUAL:
                    case SQLFilterOperator.GREATER_EQUAL:
                        textualValue.delete(0, 2);
                        break;
                    case SQLFilterOperator.LESS:
                    case SQLFilterOperator.GREATER:
                        textualValue.delete(0, 1);
                        break;
                    case SQLFilterOperator.NOT_LIKE:
                        textualValue.delete(0, 8);
                        break;
                    case SQLFilterOperator.LIKE:
                        textualValue.delete(0, 4);
                        break;
                    case SQLFilterOperator.SQLNULL:
                        textualValue.delete(0, 7);
                        break;
                    case SQLFilterOperator.NOT_SQLNULL:
                        textualValue.delete(0, 11);
                        break;
                }
                structuredFilter[i][j].Value = textualValue.toString().trim();
            }
        }
        return structuredFilter;
    }

    public XSingleSelectQueryComposer getQueryComposer()
    {
        return m_queryComposer;
    }
}
