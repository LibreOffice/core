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
package com.sun.star.wizards.query;

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.common.Resource;
import com.sun.star.wizards.db.FieldColumn;
import com.sun.star.wizards.db.QueryMetaData;
import com.sun.star.wizards.ui.FilterComponent;

public class QuerySummary extends QueryMetaData
{

    private String sSummary;
    private Resource oResource;
    private final String sSeparator;
    private final String sReturnChar;
    private final String sAnd;
    private final String sOr;

    public QuerySummary(XMultiServiceFactory _xMSF, Resource _oResource)
    {
        super(_xMSF);
        this.oResource = _oResource;
        sAnd = oResource.getResText("RID_QUERY_38");
        sOr = oResource.getResText("RID_QUERY_39");
        sSeparator = oResource.getResText("RID_QUERY_91");
        sReturnChar = String.valueOf((char) 13) + String.valueOf((char) 13);
    }

    public void setSummaryString()
    {
        try
        {
            String sFieldNamesFraction = PropertyNames.EMPTY_STRING;
            String sSortingFraction = PropertyNames.EMPTY_STRING;
            String sFilterFraction = PropertyNames.EMPTY_STRING;
            String sAggregateFraction = PropertyNames.EMPTY_STRING;
            String sGroupByFraction = PropertyNames.EMPTY_STRING;
            String sHavingFraction = PropertyNames.EMPTY_STRING;
            sFieldNamesFraction = combineFieldNameFraction() + sReturnChar;
            sSortingFraction = combinePartString("RID_QUERY_51", getSortFieldNames(), "RID_QUERY_52", "RID_QUERY_93", new String[]
                    {
                        "<FIELDNAME>", "<SORTMODE>"
                    }) + sReturnChar;
            sFilterFraction = combineFilterNameFraction(this.getFilterConditions(), "RID_QUERY_53", "RID_QUERY_54") + sReturnChar;
            if (xDBMetaData.supportsGroupBy())
            {
                sGroupByFraction = combinePartString("RID_QUERY_57", GroupFieldNames, "RID_QUERY_58") + sReturnChar;
                sHavingFraction = combineFilterNameFraction(getGroupByFilterConditions(), "RID_QUERY_59", "RID_QUERY_60");
            }
            // TODO: remove the last return from the string
            sSummary = sFieldNamesFraction + sSortingFraction + sFilterFraction + sAggregateFraction + sGroupByFraction + sHavingFraction;
            sSummary = JavaTools.replaceSubString(sSummary, PropertyNames.EMPTY_STRING, "~");
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public String getSummaryString()
    {
        return sSummary;
    }

    private String combineFilterNameFraction(PropertyValue[][] _filterconditions, String _InitResID, String _AlternativeResID)
    {
        if (_filterconditions != null && _filterconditions.length > 0)
        {
            String sconditions = PropertyNames.EMPTY_STRING;
            String sStart = oResource.getResText(_InitResID);
            String BaseString = oResource.getResText("RID_QUERY_96");
            if (_filterconditions.length == 1)
            {
                PropertyValue[] curfilterconditions = _filterconditions[0];
                for (int i = 0; i < curfilterconditions.length; i++)
                {
                    sconditions += FilterComponent.getDisplayCondition(BaseString, _filterconditions[0][i], this);
                    sconditions = appendClauseSeparator(sconditions, PropertyNames.SPACE + sAnd + PropertyNames.SPACE, i, curfilterconditions.length);
                }
            }
            else
            {

                for (int i = 0; i < _filterconditions.length; i++)
                {
                    sconditions += FilterComponent.getDisplayCondition(BaseString, _filterconditions[i][0], this);
                    sconditions = appendClauseSeparator(sconditions, PropertyNames.SPACE + sOr + PropertyNames.SPACE, i, _filterconditions.length);
                }
            }
            return sStart + sconditions;
        }
        return oResource.getResText(_AlternativeResID);
    }

    private String combineFieldNameFraction()
    {
        String CurString = PropertyNames.EMPTY_STRING;
        String sReturn = oResource.getResText("RID_QUERY_50");
        String BaseString = oResource.getResText("RID_QUERY_92");
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

    private String combinePartString(String _InitResID, String[] _FieldNames, String _AlternativeResID)
    {
        if (_FieldNames != null && _FieldNames.length > 0)
        {
            return ArrayFieldsToString(_InitResID, _FieldNames);
        }
        return oResource.getResText(_AlternativeResID);
    }

    private String ArrayFieldsToString(String _InitResID, String[] _FieldNames)
    {
        StringBuilder sReturn = new StringBuilder(oResource.getResText(_InitResID));
        int FieldCount = _FieldNames.length;
        for (int i = 0; i < FieldCount; i++)
        {
            sReturn.append(this.getFieldColumnByDisplayName(_FieldNames[i]).getFieldTitle());
            if (i < FieldCount - 1)
            {
                sReturn.append(sSeparator);
            }
        }
        return sReturn.toString();
    }

    private String combinePartString(String _InitResID, String[][] _FieldNames, String _AlternativeResID, String _BaseStringID, String[] _ReplaceTags)
    {
        if (_FieldNames != null && _FieldNames.length > 0)
        {
            return ArrayFieldsToString(_InitResID, _FieldNames, _BaseStringID, _ReplaceTags);
        }
        return oResource.getResText(_AlternativeResID);
    }

    private String ArrayFieldsToString(String _InitResID, String[][] _FieldNames, String _BaseStringID, String[] _ReplaceTags)
    {
        String CurString = PropertyNames.EMPTY_STRING;
        StringBuilder sReturn = new StringBuilder(oResource.getResText(_InitResID));
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
                sReturn.append(CurString);
                if (i < FieldCount - 1)
                {
                    sReturn.append(sSeparator);
                }
            }
        }
        return sReturn.toString();
    }
}
