/*************************************************************************
*
*  $RCSfile: QuerySummary.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: kz $ $Date: 2004-05-19 12:45:22 $
*
*  The Contents of this file are made available subject to the terms of
*  either of the following licenses
*
*         - GNU Lesser General Public License Version 2.1
*         - Sun Industry Standards Source License Version 1.1
*
*  Sun Microsystems Inc., October, 2000
*
*  GNU Lesser General Public License Version 2.1
*  =============================================
*  Copyright 2000 by Sun Microsystems, Inc.
*  901 San Antonio Road, Palo Alto, CA 94303, USA
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License version 2.1, as published by the Free Software Foundation.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
*  MA  02111-1307  USA
*
*
*  Sun Industry Standards Source License Version 1.1
*  =================================================
*  The contents of this file are subject to the Sun Industry Standards
*  Source License Version 1.1 (the "License"); You may not use this file
*  except in compliance with the License. You may obtain a copy of the
*  License at http://www.openoffice.org/license.html.
*
*  Software provided under this License is provided on an "AS IS" basis,
*  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
*  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
*  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
*  See the License for the specific provisions governing your rights and
*  obligations concerning the Software.
*
*  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
*
*  Copyright: 2000 by Sun Microsystems, Inc.
*
*  All Rights Reserved.
*
*  Contributor(s): _______________________________________
*
*/

package com.sun.star.wizards.query;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.ui.FilterComponent;
import com.sun.star.wizards.ui.*;
import com.sun.star.wizards.db.*;
import com.sun.star.beans.*;

public class QuerySummary extends QueryMetaData {
    final protected int RID_QUERY = 2300;
    final protected int RID_REPORT = 2400;
    String sSummary;
    Resource oResource;
    XMultiServiceFactory xMSF;
    String sSeparator;
    String sReturnChar;

    public QuerySummary(XMultiServiceFactory _xMSF, Resource _oResource) {
        super(_xMSF);
        this.oResource = _oResource;
        this.xMSF = _xMSF;
        sSeparator = oResource.getResText(RID_QUERY + 91);
        sReturnChar = String.valueOf((char) 13) + String.valueOf((char) 13);
    }

    /*  boolean bAssignAliases =        xDBMetaData.supportsColumnAliasing();
        boolean bSupportsGroupByUnrelated = xDBMetaData.supportsGroupByUnrelated();
        boolean bSupportsOrderByUnrelated = xDBMetaData.supportsOrderByUnrelated();
        boolean bSupportsNumericFunctions = xDBMetaData.getNumericFunctions() != "";
                            xDBMetaData.getMaxColumnsInGroupBy();
                            xDBMetaData.getMaxColumnsInOrderBy();
                            xDBMetaData.getMaxColumnsInSelect();
                            xDBMetaData.getMaxCharLiteralLength();  // gef?hrlich, da h?chstwahrscheinlich nicht sauber in jedem Treiber implementiert!!!!!
      *   */

    public void setSummaryString() {
        try {
            String sFieldNamesFraction = "";
            String sSortingFraction = "";
            String sFilterFraction = "";
            String sAggregateFraction = "";
            String sGroupByFraction = "";
            String sHavingFraction = "";
            sFieldNamesFraction = combineFieldNameFraction() + sReturnChar;
            sSortingFraction = combinePartString(RID_QUERY + 51, SortFieldNames, RID_QUERY + 52, RID_QUERY + 93, new String[] { "<FIELDNAME>", "<SORTMODE>" }) + sReturnChar;
            sFilterFraction = combineFilterNameFraction(this.FilterConditions, RID_QUERY + 53, RID_QUERY + 54) + sReturnChar;
            //      if (xDBMetaData.getNumericFunctions().length() > 0)
            //          sAggregateFraction = combinePartString(RID_QUERY + 55, AggregateFieldNames, RID_QUERY + 56, RID_QUERY + 95, new String[]{ "<CALCULATEDFUNCTION>", "<FIELDNAME>"}) + sReturnChar;
            if (xDBMetaData.supportsGroupBy()) {
                sGroupByFraction = combinePartString(RID_QUERY + 57, GroupFieldNames, RID_QUERY + 58) + sReturnChar;
                sHavingFraction = combineFilterNameFraction(this.GroupByFilterConditions, RID_QUERY + 59, RID_QUERY + 60);
            }

            // TODO: remove the last return from the string
            sSummary = sFieldNamesFraction + sSortingFraction + sFilterFraction + sAggregateFraction + sGroupByFraction + sHavingFraction;
            sSummary = JavaTools.replaceSubString(sSummary, "", "~");
        } catch (com.sun.star.uno.Exception exception) {
            exception.printStackTrace(System.out);
        }
    }


    public String getSummaryString() {
        return sSummary;
    }


    private String combineFilterNameFraction(PropertyValue[][] _filterconditions, int _InitResID, int _AlternativeResID) {
        if (_filterconditions != null) {
            if (_filterconditions.length > 0) {
                String sconditions = "";
                String sStart = oResource.getResText(_InitResID);
                String BaseString = oResource.getResText(RID_QUERY + 96);
                //TODO take aliasname instead of displayname
                if (_filterconditions.length == 1) {
                    PropertyValue[] curfilterconditions = _filterconditions[0];
                    for (int i = 0; i < curfilterconditions.length; i++) {
                        sconditions += FilterComponent.getDisplayCondition(BaseString, _filterconditions[0][i], this);
                        sconditions = appendClauseSeparator(sconditions, " and ", i, curfilterconditions.length);
                    }
                } else {
                    for (int i = 0; i < _filterconditions.length; i++) {
                        sconditions += FilterComponent.getDisplayCondition(BaseString, _filterconditions[i][0],this);
                        sconditions = appendClauseSeparator(sconditions, " or ", i, _filterconditions.length);
                    }
                }
                String sreturn = sStart + sconditions;
                return sreturn;
            }
        }
        return oResource.getResText(_AlternativeResID);
    }

    private String combineFieldNameFraction() {
        String CurString = "";
        String sReturn = oResource.getResText(RID_QUERY + 50);
        int FieldCount = FieldNames.length;
        String BaseString = oResource.getResText(RID_QUERY + 92);
        for (int i = 0; i < FieldCount; i++) {
            CurString = BaseString;
            FieldColumn CurDBFieldColumn = super.getFieldColumnByDisplayName(FieldNames[i]);
            int iAggregate = getAggregateIndex(FieldNames[i]);
            if (iAggregate > -1) {
                String sAggregateDisplay = AggregateFieldNames[iAggregate][1] + "(" + AggregateFieldNames[iAggregate][0] + ")";
                CurString = JavaTools.replaceSubString(CurString, sAggregateDisplay, "<FIELDNAME>");
            }
            else
                CurString = JavaTools.replaceSubString(CurString, CurDBFieldColumn.DisplayFieldName, "<FIELDNAME>");
            sReturn += JavaTools.replaceSubString(CurString, CurDBFieldColumn.AliasName, "<FIELDTITLE>");
            sReturn = appendClauseSeparator(sReturn, sSeparator, i, FieldCount);
        }
        return sReturn;
    }


    private String appendClauseSeparator(String _basestring, String _suffix, int _i, int _fieldcount) {
        if (_i < _fieldcount - 1)
            _basestring += _suffix;
        return _basestring;
    }


    // TODO: How can you merge the following two methods to a single one in a smarter way??
    public String combinePartString(int _InitResID, String[] _FieldNames, int _AlternativeResID) {
        if (_FieldNames != null) {
            if (_FieldNames.length > 0) {
                return ArrayFieldsToString(_InitResID, _FieldNames);
            }
        }
        return oResource.getResText(_AlternativeResID);
    }

    protected String ArrayFieldsToString(int _InitResID, String[] _FieldNames) {
        String sReturn = oResource.getResText(_InitResID);
        int FieldCount = _FieldNames.length;
        for (int i = 0; i < FieldCount; i++) {
            sReturn += this.getFieldColumnByDisplayName(_FieldNames[i]).AliasName;
            if (i < FieldCount - 1)
                sReturn += sSeparator;
        }
        return (sReturn);
    }

    public String combinePartString(int _InitResID, String[][] _FieldNames, int _AlternativeResID, int _BaseStringID, String[] _ReplaceTags) {
        if (_FieldNames != null) {
            if (_FieldNames.length > 0)
                return ArrayFieldsToString(_InitResID, _FieldNames, _BaseStringID, _ReplaceTags);
        }
        return oResource.getResText(_AlternativeResID);
    }

    public String ArrayFieldsToString(int _InitResID, String[][] _FieldNames, int _BaseStringID, String[] _ReplaceTags) {
        String CurString = "";
        String sReturn = oResource.getResText(_InitResID);
        int FieldCount = _FieldNames.length;
        if (FieldCount > 0) {
            int DimCount = _FieldNames[0].length;
            String BaseString = oResource.getResText(_BaseStringID);
            for (int i = 0; i < FieldCount; i++) {
                for (int a = 0; a < DimCount; a++) {
                    if (a == 0){
                        int iAggregate = getAggregateIndex(this.getFieldColumnByDisplayName(_FieldNames[i][a]).DisplayFieldName);
                        if (iAggregate > -1){
                            String sAggregateDisplay = AggregateFieldNames[iAggregate][1] + "(" + AggregateFieldNames[iAggregate][0] + ")";
                            CurString = JavaTools.replaceSubString(BaseString, sAggregateDisplay, _ReplaceTags[a]);
                        }
                        else
                            CurString = JavaTools.replaceSubString(BaseString, this.getFieldColumnByDisplayName(_FieldNames[i][a]).AliasName, _ReplaceTags[a]);
                    }
                    else
                        CurString = JavaTools.replaceSubString(CurString, _FieldNames[i][a], _ReplaceTags[a]);
                }
                sReturn += CurString;
                if (i < FieldCount - 1)
                    sReturn += sSeparator;
            }
        }
        return sReturn;
    }
}