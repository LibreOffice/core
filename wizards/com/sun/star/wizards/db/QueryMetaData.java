/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: QueryMetaData.java,v $
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

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.beans.PropertyValue;

import java.util.*;
import com.sun.star.lang.Locale;
import com.sun.star.wizards.common.*;

public class QueryMetaData extends CommandMetaData
{

    FieldColumn CurFieldColumn;
    public String Command;
    // Vector CommandNamesV;
    private PropertyValue[][] m_aFilterConditions; /* = new PropertyValue[][] {}; */

    public PropertyValue[][] GroupByFilterConditions = new PropertyValue[][]
    {
    };
    public String[] UniqueAggregateFieldNames = new String[]
    {
    };
    public int Type = QueryType.SODETAILQUERY;

    public static interface QueryType
    {

        final static int SOSUMMARYQUERY = 0;
        final static int SODETAILQUERY = 1;
    }

    public QueryMetaData(XMultiServiceFactory xMSF, Locale CharLocale, NumberFormatter oNumberFormatter)
    {
        super(xMSF, CharLocale, oNumberFormatter);
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
//  public void addFieldColumn(String _FieldName, String _sCommandName){
//      FieldColumn oFieldColumn = getFieldColumn(_FieldName, _sCommandName);
//      if (oFieldColumn == null){
//          FieldColumn[] LocFieldColumns = new FieldColumn[FieldColumns.length + 1];
//          System.arraycopy(FieldColumns, 0, LocFieldColumns, 0, FieldColumns.length);
//          LocFieldColumns[FieldColumns.length] = new FieldColumn(this, _FieldName, _sCommandName);
//          FieldColumns = LocFieldColumns;
//      }
//    }
    public void addSeveralFieldColumns(String[] _FieldNames, String _sCommandName)
    {
        Vector oToBeAddedFieldColumns = new Vector();
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
                LocFieldColumns[nOldFieldCount + i] = (FieldColumn) oToBeAddedFieldColumns.elementAt(i);
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
        Vector oRemainingFieldColumns = new Vector();
        int a = 0;
        for (int n = 0; n < FieldColumns.length; n++)
        {
            String sDisplayFieldName = FieldColumns[n].getDisplayFieldName();
            if (!(JavaTools.FieldInList(_DisplayFieldNames, sDisplayFieldName) > -1))
            {
                oRemainingFieldColumns.add(FieldColumns[n]);
            }
        }
        FieldColumns = new FieldColumn[oRemainingFieldColumns.size()];
        oRemainingFieldColumns.toArray(FieldColumns);
    }

    public void removeFieldColumn(String _sFieldName, String _sCommandName)
    {
        FieldColumn oFieldColumn = getFieldColumn(_sFieldName, _sCommandName);
        int a = 0;
        if (oFieldColumn != null)
        {
            FieldColumn[] LocFieldColumns = new FieldColumn[FieldColumns.length - 1];
            for (int i = 0; i < FieldColumns.length; i++)
            {
                if (!FieldColumns[i].getFieldName().equals(_sFieldName))
                {
                    if (!FieldColumns[i].getCommandName().equals(_sCommandName))
                    {
                        LocFieldColumns[a] = FieldColumns[i];
                        a++;
                    }
                }
            }
            FieldColumns = LocFieldColumns;
        }
    }

    public String[] getIncludedCommandNames()
    {
        // FieldColumn CurQueryField;
        Vector CommandNamesV = new Vector(1);
        // String CurCommandName;
        for (int i = 0; i < FieldColumns.length; i++)
        {
            final FieldColumn CurQueryField = FieldColumns[i];
            final String CurCommandName = CurQueryField.getCommandName();
            if (!CommandNamesV.contains(CurCommandName))
            {
                CommandNamesV.addElement(CurCommandName);
            }
        }
        String[] sIncludedCommandNames = new String[CommandNamesV.size()];
        CommandNamesV.toArray(sIncludedCommandNames);
        return sIncludedCommandNames;
    }

    public static String[] getIncludedCommandNames(String[] _FieldNames)
    {
        Vector CommandNames = new Vector(1);
        for (int i = 0; i < _FieldNames.length; i++)
        {
            String CurCommandName = "";
            String[] MetaList = JavaTools.ArrayoutofString(_FieldNames[i], ".");
            if (MetaList.length > 1)
            {
                for (int a = 0; a < MetaList.length - 1; a++)
                {
                    CurCommandName += MetaList[a];
                }
                if (!CommandNames.contains(CurCommandName))
                {
                    CommandNames.addElement(CurCommandName);
                }
            }
        }
        String[] sIncludedCommandNames = new String[CommandNames.size()];
        CommandNames.toArray(sIncludedCommandNames);
        return sIncludedCommandNames;
    }

    public String[] getFieldNamesOfCommand(String _sCommandName)
    {
        CommandObject oTable = getTableByName(_sCommandName);
        return oTable.getColumns().getElementNames();
    }

    public void initializeFieldTitleSet(boolean _bAppendMode)
    {
        try
        {
            // this.getIncludedCommandNames(); // fills the var CommandNamesV indirectly :-(
            if (FieldTitleSet == null)
            {
                FieldTitleSet = new HashMap();
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
            exception.printStackTrace(System.out);
        }
    }

    public String[] getUniqueAggregateFieldNames()
    {
        Vector UniqueAggregateFieldVector = new Vector(0);
        for (int i = 0; i < AggregateFieldNames.length; i++)
        {
            if (!UniqueAggregateFieldVector.contains(AggregateFieldNames[i][0]))
            {
                UniqueAggregateFieldVector.add(AggregateFieldNames[i][0]);
            }
        }
        UniqueAggregateFieldNames = new String[UniqueAggregateFieldVector.size()];
        UniqueAggregateFieldVector.toArray(UniqueAggregateFieldNames);
        return UniqueAggregateFieldNames;
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
}
