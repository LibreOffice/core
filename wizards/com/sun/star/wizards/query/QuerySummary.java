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
package com.sun.star.wizards.query;

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.Resource;
import com.sun.star.wizards.db.FieldColumn;
import com.sun.star.wizards.db.QueryMetaData;
import com.sun.star.wizards.ui.FilterComponent;

public class QuerySummary extends QueryMetaData
{

    final protected int RID_QUERY = 2300;
    final protected int RID_REPORT = 2400;
    String sSummary;
    Resource oResource;
    // XMultiServiceFactory xMSF;
    private String sSeparator;
    private String sReturnChar;
    private String sAnd;
    private String sOr;

    public QuerySummary(XMultiServiceFactory _xMSF, Resource _oResource)
    {
        super(_xMSF);
        this.oResource = _oResource;
        // this.xMSF = _xMSF;
        sAnd = oResource.getResText(RID_QUERY + 38);
        sOr = oResource.getResText(RID_QUERY + 39);
        sSeparator = oResource.getResText(RID_QUERY + 91);
        sReturnChar = String.valueOf((char) 13) + String.valueOf((char) 13);
    }

    public void setSummaryString()
    {
        try
        {
            String sFieldNamesFraction = "";
            String sSortingFraction = "";
            String sFilterFraction = "";
            String sAggregateFraction = "";
            String sGroupByFraction = "";
            String sHavingFraction = "";
            sFieldNamesFraction = combineFieldNameFraction() + sReturnChar;
            sSortingFraction = combinePartString(RID_QUERY + 51, getSortFieldNames(), RID_QUERY + 52, RID_QUERY + 93, new String[]
                    {
                        "<FIELDNAME>", "<SORTMODE>"
                    }) + sReturnChar;
            sFilterFraction = combineFilterNameFraction(this.getFilterConditions(), RID_QUERY + 53, RID_QUERY + 54) + sReturnChar;
            //      if (xDBMetaData.getNumericFunctions().length() > 0)
            //          sAggregateFraction = combinePartString(RID_QUERY + 55, AggregateFieldNames, RID_QUERY + 56, RID_QUERY + 95, new String[]{ "<CALCULATEDFUNCTION>", "<FIELDNAME>"}) + sReturnChar;
            if (xDBMetaData.supportsGroupBy())
            {
                sGroupByFraction = combinePartString(RID_QUERY + 57, GroupFieldNames, RID_QUERY + 58) + sReturnChar;
                sHavingFraction = combineFilterNameFraction(getGroupByFilterConditions(), RID_QUERY + 59, RID_QUERY + 60);
            }
            // TODO: remove the last return from the string
            sSummary = sFieldNamesFraction + sSortingFraction + sFilterFraction + sAggregateFraction + sGroupByFraction + sHavingFraction;
            sSummary = JavaTools.replaceSubString(sSummary, "", "~");
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    public String getSummaryString()
    {
        return sSummary;
    }

    private String combineFilterNameFraction(PropertyValue[][] _filterconditions, int _InitResID, int _AlternativeResID)
    {
        if (_filterconditions != null && _filterconditions.length > 0)
        {
            String sconditions = "";
            String sStart = oResource.getResText(_InitResID);
            String BaseString = oResource.getResText(RID_QUERY + 96);
            if (_filterconditions.length == 1)
            {
                PropertyValue[] curfilterconditions = _filterconditions[0];
                for (int i = 0; i < curfilterconditions.length; i++)
                {
                    sconditions += FilterComponent.getDisplayCondition(BaseString, _filterconditions[0][i], this);
                    sconditions = appendClauseSeparator(sconditions, " " + sAnd + " ", i, curfilterconditions.length);
                }
            }
            else
            {

                for (int i = 0; i < _filterconditions.length; i++)
                {
                    sconditions += FilterComponent.getDisplayCondition(BaseString, _filterconditions[i][0], this);
                    sconditions = appendClauseSeparator(sconditions, " " + sOr + " ", i, _filterconditions.length);
                }
            }
            String sreturn = sStart + sconditions;
            return sreturn;
        }
        return oResource.getResText(_AlternativeResID);
    }

    private String combineFieldNameFraction()
    {
        String CurString = "";
        String sReturn = oResource.getResText(RID_QUERY + 50);
        String BaseString = oResource.getResText(RID_QUERY + 92);
        for (int i = 0; i < FieldColumns.length; i++)
        {
            CurString = BaseString;
            FieldColumn CurDBFieldColumn = super.getFieldColumnByDisplayName(FieldColumns[i].getDisplayFieldName());
            int iAggregate = getAggregateIndex(FieldColumns[i].getFieldName());
            if (iAggregate > -1)
            {
                String sAggregateDisplay = AggregateFieldNames[iAggregate][1] + "(" + AggregateFieldNames[iAggregate][0] + ")";
                CurString = JavaTools.replaceSubString(CurString, sAggregateDisplay, "<FIELDNAME>");
            }
            else
            {
                CurString = JavaTools.replaceSubString(CurString, CurDBFieldColumn.getDisplayFieldName(), "<FIELDNAME>");
            }
            sReturn += JavaTools.replaceSubString(CurString, CurDBFieldColumn.getFieldTitle(), "<FIELDTITLE>");
            sReturn = appendClauseSeparator(sReturn, sSeparator, i, FieldColumns.length);
        }
        return sReturn;
    }

    private String appendClauseSeparator(String _basestring, String _suffix, int _i, int _fieldcount)
    {
        if (_i < _fieldcount - 1)
        {
            _basestring += _suffix;
        }
        return _basestring;
    }
    // TODO: How can you merge the following two methods to a single one in a smarter way??

    public String combinePartString(int _InitResID, String[] _FieldNames, int _AlternativeResID)
    {
        if (_FieldNames != null && _FieldNames.length > 0)
        {
            return ArrayFieldsToString(_InitResID, _FieldNames);
        }
        return oResource.getResText(_AlternativeResID);
    }

    protected String ArrayFieldsToString(int _InitResID, String[] _FieldNames)
    {
        String sReturn = oResource.getResText(_InitResID);
        int FieldCount = _FieldNames.length;
        for (int i = 0; i < FieldCount; i++)
        {
            sReturn += this.getFieldColumnByDisplayName(_FieldNames[i]).getFieldTitle();
            if (i < FieldCount - 1)
            {
                sReturn += sSeparator;
            }
        }
        return (sReturn);
    }

    public String combinePartString(int _InitResID, String[][] _FieldNames, int _AlternativeResID, int _BaseStringID, String[] _ReplaceTags)
    {
        if (_FieldNames != null && _FieldNames.length > 0)
        {
            return ArrayFieldsToString(_InitResID, _FieldNames, _BaseStringID, _ReplaceTags);
        }
        return oResource.getResText(_AlternativeResID);
    }

    public String ArrayFieldsToString(int _InitResID, String[][] _FieldNames, int _BaseStringID, String[] _ReplaceTags)
    {
        String CurString = "";
        String sReturn = oResource.getResText(_InitResID);
        int FieldCount = _FieldNames.length;
        if (FieldCount > 0)
        {
            int DimCount = _FieldNames[0].length;
            String BaseString = oResource.getResText(_BaseStringID);
            for (int i = 0; i < FieldCount; i++)
            {
                for (int a = 0; a < DimCount; a++)
                {
                    if (a == 0)
                    {
                        int iAggregate = getAggregateIndex(this.getFieldColumnByDisplayName(_FieldNames[i][a]).getDisplayFieldName());
                        if (iAggregate > -1)
                        {
                            String sAggregateDisplay = AggregateFieldNames[iAggregate][1] + "(" + AggregateFieldNames[iAggregate][0] + ")";
                            CurString = JavaTools.replaceSubString(BaseString, sAggregateDisplay, _ReplaceTags[a]);
                        }
                        else
                        {
                            CurString = JavaTools.replaceSubString(BaseString, this.getFieldColumnByDisplayName(_FieldNames[i][a]).getFieldTitle(), _ReplaceTags[a]);
                        }
                    }
                    else
                    {
                        CurString = JavaTools.replaceSubString(CurString, _FieldNames[i][a], _ReplaceTags[a]);
                    }
                }
                sReturn += CurString;
                if (i < FieldCount - 1)
                {
                    sReturn += sSeparator;
                }
            }
        }
        return sReturn;
    }
}
