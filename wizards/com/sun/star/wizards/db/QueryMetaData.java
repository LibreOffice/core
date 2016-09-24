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
import com.sun.star.beans.PropertyValue;

import java.util.*;
import com.sun.star.wizards.common.*;

public class QueryMetaData extends CommandMetaData
{

    private SQLQueryComposer oSQLQueryComposer = null;
    public String Command;
    // Vector CommandNamesV;
    private PropertyValue[][] m_aFilterConditions; /* = new PropertyValue[][] {}; */

    public PropertyValue[][] GroupByFilterConditions = new PropertyValue[][]
    {
    };
    public int Type = QueryType.SODETAILQUERY;

    public interface QueryType
    {
       int SOSUMMARYQUERY = 0;
       int SODETAILQUERY = 1;
    }

    public QueryMetaData(XMultiServiceFactory _xMSF)
    {
        super(_xMSF);
    }

    public void setFilterConditions(PropertyValue[][] _FilterConditions)
    {
        this.m_aFilterConditions = _FilterConditions;
    }

    public PropertyValue[][] getFilterConditions()
    {
        if (m_aFilterConditions == null)
        {
            m_aFilterConditions = new PropertyValue[][]
                    {
                    };
        }
        return m_aFilterConditions;
    }

    public void setGroupByFilterConditions(PropertyValue[][] _GroupByFilterConditions)
    {
        this.GroupByFilterConditions = _GroupByFilterConditions;
    }

    public PropertyValue[][] getGroupByFilterConditions()
    {
        return this.GroupByFilterConditions;
    }

    public void addSeveralFieldColumns(String[] _FieldNames, String _sCommandName)
    {
        ArrayList<FieldColumn> oToBeAddedFieldColumns = new ArrayList<FieldColumn>();
        for (int i = 0; i < _FieldNames.length; i++)
        {
            FieldColumn oFieldColumn = getFieldColumn(_FieldNames[i], _sCommandName);
            if (oFieldColumn == null)
            {
                oToBeAddedFieldColumns.add(new FieldColumn(this, _FieldNames[i], _sCommandName, false));
            }
        }
        if (oToBeAddedFieldColumns.size() > 0)
        {
            int nOldFieldCount = FieldColumns.length;
            FieldColumn[] LocFieldColumns = new FieldColumn[nOldFieldCount + oToBeAddedFieldColumns.size()];
            System.arraycopy(FieldColumns, 0, LocFieldColumns, 0, nOldFieldCount);
            for (int i = 0; i < oToBeAddedFieldColumns.size(); i++)
            {
                LocFieldColumns[nOldFieldCount + i] = oToBeAddedFieldColumns.get(i);
            }
            FieldColumns = LocFieldColumns;
        }
    }

    public void reorderFieldColumns(String[] _sDisplayFieldNames)
    {
        FieldColumn[] LocFieldColumns = new FieldColumn[FieldColumns.length];
        for (int i = 0; i < _sDisplayFieldNames.length; i++)
        {
            FieldColumn LocFieldColumn = this.getFieldColumnByDisplayName(_sDisplayFieldNames[i]);
            LocFieldColumns[i] = LocFieldColumn;
        }
        System.arraycopy(LocFieldColumns, 0, FieldColumns, 0, LocFieldColumns.length);
    }

    public void removeSeveralFieldColumnsByDisplayFieldName(String[] _DisplayFieldNames)
    {
        ArrayList<FieldColumn> oRemainingFieldColumns = new ArrayList<FieldColumn>();
        for (int n = 0; n < FieldColumns.length; n++)
        {
            String sDisplayFieldName = FieldColumns[n].getDisplayFieldName();
            if (JavaTools.FieldInList(_DisplayFieldNames, sDisplayFieldName) <= -1)
            {
                oRemainingFieldColumns.add(FieldColumns[n]);
            }
        }
        FieldColumns = new FieldColumn[oRemainingFieldColumns.size()];
        oRemainingFieldColumns.toArray(FieldColumns);
    }



    public String[] getIncludedCommandNames()
    {
        ArrayList<String> CommandNamesV = new ArrayList<String>(1);
        for (int i = 0; i < FieldColumns.length; i++)
        {
            final FieldColumn CurQueryField = FieldColumns[i];
            final String CurCommandName = CurQueryField.getCommandName();
            if (!CommandNamesV.contains(CurCommandName))
            {
                CommandNamesV.add(CurCommandName);
            }
        }
        String[] sIncludedCommandNames = new String[CommandNamesV.size()];
        CommandNamesV.toArray(sIncludedCommandNames);
        return sIncludedCommandNames;
    }

    public static String[] getIncludedCommandNames(String[] _FieldNames)
    {
        ArrayList<String> CommandNames = new ArrayList<String>(1);
        for (int i = 0; i < _FieldNames.length; i++)
        {
            String[] MetaList = JavaTools.ArrayoutofString(_FieldNames[i], ".");
            if (MetaList.length > 1)
            {
                StringBuilder sb = new StringBuilder(PropertyNames.EMPTY_STRING);
                for (int a = 0; a < MetaList.length - 1; a++)
                {
                    sb.append(MetaList[a]);
                }
                String CurCommandName = sb.toString();
                if (!CommandNames.contains(CurCommandName))
                {
                    CommandNames.add(CurCommandName);
                }
            }
        }
        String[] sIncludedCommandNames = new String[CommandNames.size()];
        CommandNames.toArray(sIncludedCommandNames);
        return sIncludedCommandNames;
    }

    public void initializeFieldTitleSet()
    {
        try
        {
            if (FieldTitleSet == null)
            {
                FieldTitleSet = new HashMap<String, String>();
            }
            String[] aCommandNames = getIncludedCommandNames();
            for (int i = 0; i < aCommandNames.length; i++)
            {
                String sCommandName = aCommandNames[i];
                CommandObject oTable = getTableByName(sCommandName);
                String sTableName = oTable.getName();
                String[] LocFieldNames = oTable.getColumns().getElementNames();
                for (int a = 0; a < LocFieldNames.length; a++)
                {
                    String sDisplayFieldName = FieldColumn.composeDisplayFieldName(sTableName, LocFieldNames[a]);
                    if (!FieldTitleSet.containsKey(sDisplayFieldName))
                    {
                        FieldTitleSet.put(sDisplayFieldName, LocFieldNames[a]);
                    }
                }
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public String[] getUniqueAggregateFieldNames()
    {
        ArrayList<String> UniqueAggregateFieldVector = new ArrayList<String>();
        for (int i = 0; i < AggregateFieldNames.length; i++)
        {
            if (!UniqueAggregateFieldVector.contains(AggregateFieldNames[i][0]))
            {
                UniqueAggregateFieldVector.add(AggregateFieldNames[i][0]);
            }
        }
        return UniqueAggregateFieldVector.toArray(new String[UniqueAggregateFieldVector.size()]);
    }

    public boolean hasNumericalFields()
    {
        for (int i = 0; i < FieldColumns.length; i++)
        {
            if (FieldColumns[i].isNumberFormat())
            {
                return true;
            }
        }
        return false;
    }

    public int getAggregateIndex(String _DisplayFieldName)
    {
        int iAggregate = -1;
        if (Type == QueryType.SOSUMMARYQUERY)
        {
            iAggregate = JavaTools.FieldInTable(AggregateFieldNames, _DisplayFieldName);
        }
        return iAggregate;
    }

    public SQLQueryComposer getSQLQueryComposer()
    {
        if (oSQLQueryComposer == null)
        {
            oSQLQueryComposer = new SQLQueryComposer(this);
        }
        return oSQLQueryComposer;
    }
}
