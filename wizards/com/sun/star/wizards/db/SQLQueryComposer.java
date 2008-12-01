/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SQLQueryComposer.java,v $
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

import java.util.Vector;

// import com.sun.star.lang.IllegalArgumentException;
// import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.beans.*;
// import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XIndexAccess;
// import com.sun.star.container.XNameAccess;
import com.sun.star.sdbcx.XColumnsSupplier;
// import com.sun.star.sdb.XColumn;
import com.sun.star.sdb.XSQLQueryComposer;
import com.sun.star.sdb.XSQLQueryComposerFactory;
import com.sun.star.sdb.XSingleSelectQueryComposer;
import com.sun.star.sdb.XSingleSelectQueryAnalyzer;
import com.sun.star.ui.dialogs.XExecutableDialog;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.sdbc.SQLException;
import com.sun.star.lang.XInitialization;
import com.sun.star.awt.XWindow;

import com.sun.star.wizards.common.*;

public class SQLQueryComposer
{

    public XColumnsSupplier xColSuppl;
    // XSQLQueryComposer xSQLQueryComposer;
    QueryMetaData CurDBMetaData;
    // String m_sSelectClause;
    // String m_sFromClause;
    public XSingleSelectQueryAnalyzer m_xQueryAnalyzer;
    Vector composedCommandNames = new Vector(1);
    public XSingleSelectQueryComposer m_xQueryComposer;
    XMultiServiceFactory xMSF;
    boolean bincludeGrouping = true;

    public SQLQueryComposer(QueryMetaData _CurDBMetaData)
    {
        try
        {
            this.CurDBMetaData = _CurDBMetaData;
            xMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, CurDBMetaData.DBConnection);
            final Object oQueryComposer = xMSF.createInstance("com.sun.star.sdb.SingleSelectQueryComposer");
            m_xQueryAnalyzer = (XSingleSelectQueryAnalyzer) UnoRuntime.queryInterface(XSingleSelectQueryAnalyzer.class, oQueryComposer);
            m_xQueryComposer = (XSingleSelectQueryComposer) UnoRuntime.queryInterface(XSingleSelectQueryComposer.class, m_xQueryAnalyzer);
            XSQLQueryComposerFactory xSQLComposerFactory;
            xSQLComposerFactory = (XSQLQueryComposerFactory) UnoRuntime.queryInterface(XSQLQueryComposerFactory.class, CurDBMetaData.DBConnection);
        // /* XSQLQueryComposer */ xSQLQueryComposer = xSQLComposerFactory.createQueryComposer();
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    private boolean addtoSelectClause(String DisplayFieldName) throws SQLException
    {
        if (bincludeGrouping)
        {
            if (CurDBMetaData.xDBMetaData.supportsGroupByUnrelated())
            {
                if (CurDBMetaData.GroupFieldNames != null)
                {
                    if (JavaTools.FieldInList(CurDBMetaData.GroupFieldNames, DisplayFieldName) > -1)
                    {
                        return false;
                    }
                }
            }
        }
        return true;
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
                String sCurFieldName = CurDBMetaData.getFilterConditions()[i][0].Name;
                m_xQueryComposer.setStructuredFilter(CurDBMetaData.getFilterConditions());
            }
            String s = m_xQueryAnalyzer.getQuery();
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    public void prependSortingCriteria() throws SQLException
    {
        XIndexAccess xColumnIndexAccess = m_xQueryAnalyzer.getOrderColumns();
        m_xQueryComposer.setOrder("");
        for (int i = 0; i < CurDBMetaData.getSortFieldNames().length; i++)
        {
            appendSortingCriterion(i, false);
        }
        for (int i = 0; i < xColumnIndexAccess.getCount(); i++)
        {
            try
            {
                XPropertySet xColumnPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xColumnIndexAccess.getByIndex(i));
                String sName = (String) xColumnPropertySet.getPropertyValue("Name");
                if (JavaTools.FieldInTable(CurDBMetaData.getSortFieldNames(), sName) == -1)
                {
                    boolean bascend = AnyConverter.toBoolean(xColumnPropertySet.getPropertyValue("IsAscending"));
                    m_xQueryComposer.appendOrderByColumn(xColumnPropertySet, bascend);
                }
            }
            catch (Exception e)
            {
                e.printStackTrace(System.out);
            }
        }
    }

    private void appendSortingCriterion(int _SortIndex, boolean _baddAliasFieldNames) throws SQLException
    {
        String sSortValue = CurDBMetaData.getSortFieldNames()[_SortIndex][0];
        XPropertySet xColumn = CurDBMetaData.getColumnObjectByFieldName(sSortValue, _baddAliasFieldNames);

        String sSort = CurDBMetaData.getSortFieldNames()[_SortIndex][1];
        boolean bascend = (sSort.equals("ASC"));
        m_xQueryComposer.appendOrderByColumn(xColumn, bascend);
    }

    public void appendSortingcriteria(boolean _baddAliasFieldNames) throws SQLException
    {
        String sOrder = "";
        m_xQueryComposer.setOrder("");
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
                sOrder += CurDBMetaData.AggregateFieldNames[iAggregate][1] + "(" + CurDBMetaData.AggregateFieldNames[iAggregate][0] + ")";
                sOrder += " " + CurDBMetaData.getSortFieldNames()[i][1];
                m_xQueryComposer.setOrder(sOrder);
            }
            else
            {
                appendSortingCriterion(i, _baddAliasFieldNames);
            }
            sOrder = m_xQueryAnalyzer.getOrder();
        }
        // just for debug!
        sOrder = m_xQueryComposer.getOrder();
        int dummy = 0;
    }

    public void appendGroupByColumns(boolean _baddAliasFieldNames) throws SQLException
    {
        for (int i = 0; i < CurDBMetaData.GroupFieldNames.length; i++)
        {
            XPropertySet xColumn = CurDBMetaData.getColumnObjectByFieldName(CurDBMetaData.GroupFieldNames[i], _baddAliasFieldNames);
            m_xQueryComposer.appendGroupByColumn(xColumn);
        }
        String s = m_xQueryAnalyzer.getQuery();
    }

    public void setDBMetaData(QueryMetaData _oDBMetaData)
    {
        this.CurDBMetaData = _oDBMetaData;
    }

    private PropertyValue[][] replaceConditionsByAlias(PropertyValue _filterconditions[][])
    {
        for (int n = 0; n < _filterconditions.length; n++)
        {
            for (int m = 0; m < _filterconditions[n].length; m++)
            {
                _filterconditions[n][m].Name = getComposedAliasFieldName(_filterconditions[n][m].Name);
            }
        }
        return _filterconditions;
    }

    public String getQuery()
    {
        return m_xQueryAnalyzer.getQuery();
    }

    private String getFromClause()
    {
        String sFromClause = "FROM";
        String[] sCommandNames = CurDBMetaData.getIncludedCommandNames();
        for (int i = 0; i < sCommandNames.length; i++)
        {
            CommandName curCommandName = new CommandName(CurDBMetaData, sCommandNames[i]); //(setComposedCommandName)
            curCommandName.setAliasName(getuniqueAliasName(curCommandName.getTableName()));
            sFromClause += " " + curCommandName.getComposedName() + " " + quoteName(curCommandName.getAliasName());
            if (i < sCommandNames.length - 1)
            {
                sFromClause += ", ";
            }
            // fill composedCommandNames
            composedCommandNames.add(curCommandName);
        }
        return sFromClause;
    }

    public boolean setQueryCommand(String QueryName, XWindow _xParentWindow, boolean _bincludeGrouping, boolean _baddAliasFieldNames)
    {
        try
        {
            String s;
            bincludeGrouping = _bincludeGrouping;
            String sFromClause = getFromClause();
            String sSelectClause = getSelectClause(_baddAliasFieldNames);
            String queryclause = sSelectClause + " " + sFromClause;
            m_xQueryAnalyzer.setQuery(queryclause);
            if (CurDBMetaData.getFilterConditions() != null)
            {
                if (CurDBMetaData.getFilterConditions().length > 0)
                {
                    CurDBMetaData.setFilterConditions(replaceConditionsByAlias(CurDBMetaData.getFilterConditions()));
                    m_xQueryComposer.setStructuredFilter(CurDBMetaData.getFilterConditions());
                }
            }
            s = m_xQueryAnalyzer.getQuery();
            if (_bincludeGrouping)
            {
                appendGroupByColumns(_baddAliasFieldNames);
                if (CurDBMetaData.GroupByFilterConditions.length > 0)
                {
                    m_xQueryComposer.setStructuredHavingClause(CurDBMetaData.GroupByFilterConditions);
                }
            }
            appendSortingcriteria(_baddAliasFieldNames);

            s = m_xQueryAnalyzer.getQuery();
            return true;
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
            displaySQLErrorDialog(exception, _xParentWindow);
            return false;
        }
    }

    private String getComposedAliasFieldName(String _fieldname)
    {
        FieldColumn CurFieldColumn = CurDBMetaData.getFieldColumnByDisplayName(_fieldname);
        CommandName curComposedCommandName = getComposedCommandByDisplayName(CurFieldColumn.getCommandName());
        String curAliasName = curComposedCommandName.getAliasName();
        return quoteName(curAliasName) + "." + quoteName(CurFieldColumn.m_sFieldName);
    }

    private CommandName getComposedCommandByAliasName(String _AliasName)
    {
        if (composedCommandNames != null)
        {
            CommandName curComposedName;
            for (int i = 0; i < composedCommandNames.size(); i++)
            {
                curComposedName = (CommandName) composedCommandNames.elementAt(i);
                if (curComposedName.getAliasName().equals(_AliasName))
                {
                    return curComposedName;
                }
            }
        }
        return null;
    }

    public CommandName getComposedCommandByDisplayName(String _DisplayName)
    {
        if (composedCommandNames != null)
        {
            CommandName curComposedName;
            for (int i = 0; i < composedCommandNames.size(); i++)
            {
                curComposedName = (CommandName) composedCommandNames.elementAt(i);
                if (curComposedName.getDisplayName().equals(_DisplayName))
                {
                    return curComposedName;
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
        while (bAliasNameexists == true)
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
            XInitialization xInitialize = (XInitialization) UnoRuntime.queryInterface(XInitialization.class, oErrorDialog);
            XExecutableDialog xExecute = (XExecutableDialog) UnoRuntime.queryInterface(XExecutableDialog.class, oErrorDialog);
            PropertyValue[] rDispatchArguments = new PropertyValue[3];
            rDispatchArguments[0] = Properties.createProperty("Title", Configuration.getProductName(CurDBMetaData.xMSF) + " Base");
            rDispatchArguments[1] = Properties.createProperty("ParentWindow", _xParentWindow);
            rDispatchArguments[2] = Properties.createProperty("SQLException", _exception);
            xInitialize.initialize(rDispatchArguments);
            xExecute.execute();
        //TODO dispose???
        }
        catch (Exception typeexception)
        {
            typeexception.printStackTrace(System.out);
        }
    }
}
