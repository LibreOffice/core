/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


package com.sun.star.wizards.query;

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.common.Resource;
import com.sun.star.wizards.db.FieldColumn;
import com.sun.star.wizards.db.QueryMetaData;
import com.sun.star.wizards.ui.FilterComponent;

import static com.sun.star.wizards.ui.UIConsts.RID_QUERY;

public class QuerySummary extends QueryMetaData
{

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
            String sFieldNamesFraction = PropertyNames.EMPTY_STRING;
            String sSortingFraction = PropertyNames.EMPTY_STRING;
            String sFilterFraction = PropertyNames.EMPTY_STRING;
            String sAggregateFraction = PropertyNames.EMPTY_STRING;
            String sGroupByFraction = PropertyNames.EMPTY_STRING;
            String sHavingFraction = PropertyNames.EMPTY_STRING;
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
            sSummary = JavaTools.replaceSubString(sSummary, PropertyNames.EMPTY_STRING, "~");
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
            String sconditions = PropertyNames.EMPTY_STRING;
            String sStart = oResource.getResText(_InitResID);
            String BaseString = oResource.getResText(RID_QUERY + 96);
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
        String CurString = PropertyNames.EMPTY_STRING;
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
