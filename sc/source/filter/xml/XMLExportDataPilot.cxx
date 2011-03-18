/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------
#include "XMLExportDataPilot.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/nmspmap.hxx>
#include <rtl/math.hxx>
#include "xmlexprt.hxx"
#include "XMLConverter.hxx"
#include "document.hxx"
#include "dpobject.hxx"
#include "dociter.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "scitems.hxx"
#include "dpsave.hxx"
#include "dpshttab.hxx"
#include "dpsdbtab.hxx"
#include "dpdimsave.hxx"
#include "dpgroup.hxx"
#include "rangeutl.hxx"
#include <com/sun/star/sheet/DataImportMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldReference.hpp>
#include <com/sun/star/sheet/DataPilotFieldReferenceType.hpp>
#include <com/sun/star/sheet/DataPilotFieldReferenceItemType.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldAutoShowInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldShowItemsMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>

using namespace com::sun::star;
using namespace xmloff::token;
using ::rtl::OUString;

ScXMLExportDataPilot::ScXMLExportDataPilot(ScXMLExport& rTempExport)
    : rExport(rTempExport),
    pDoc( NULL )
{
}

ScXMLExportDataPilot::~ScXMLExportDataPilot()
{
}

rtl::OUString ScXMLExportDataPilot::getDPOperatorXML(const ScQueryOp aFilterOperator, const sal_Bool bUseRegularExpressions,
    const sal_Bool bIsString, const double dVal, const String& sVal) const
{
    switch (aFilterOperator)
    {
        case SC_EQUAL :
        {
            rtl::OUString sReturn;
            if (bUseRegularExpressions)
                sReturn = GetXMLToken(XML_MATCH);
            else
                sReturn = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("="));

            if (!bIsString && sVal == EMPTY_STRING)
            {
                if (dVal == SC_EMPTYFIELDS)
                    sReturn = GetXMLToken(XML_EMPTY);
                else if (dVal == SC_NONEMPTYFIELDS)
                    sReturn = GetXMLToken(XML_NOEMPTY);
            }

            return sReturn;
        }
        case SC_NOT_EQUAL :
        {
            if (bUseRegularExpressions)
                return GetXMLToken(XML_NOMATCH);
            else
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("!="));
        }
        case SC_BOTPERC :
            return GetXMLToken(XML_BOTTOM_PERCENT);
        case SC_BOTVAL :
            return GetXMLToken(XML_BOTTOM_VALUES);
        case SC_GREATER :
            return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(">"));
        case SC_GREATER_EQUAL :
            return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(">="));
        case SC_LESS :
            return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<"));
        case SC_LESS_EQUAL :
            return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<="));
        case SC_TOPPERC :
            return GetXMLToken(XML_TOP_PERCENT);
        case SC_TOPVAL :
            return GetXMLToken(XML_TOP_VALUES);
        default:
            OSL_FAIL("This FilterOperator is not supported.");
    }
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("="));
}

void ScXMLExportDataPilot::WriteDPCondition(const ScQueryEntry& aQueryEntry, sal_Bool bIsCaseSensitive, sal_Bool bUseRegularExpressions)
{
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FIELD_NUMBER, rtl::OUString::valueOf(sal_Int32(aQueryEntry.nField)));
    if (bIsCaseSensitive)
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CASE_SENSITIVE, XML_TRUE);
    if (aQueryEntry.bQueryByString)
    {
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_VALUE, *aQueryEntry.pStr);
    }
    else
    {
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATA_TYPE, XML_NUMBER);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_VALUE, rtl::OUString(*aQueryEntry.pStr));
    }
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_OPERATOR, getDPOperatorXML(aQueryEntry.eOp, bUseRegularExpressions,
        aQueryEntry.bQueryByString, aQueryEntry.nVal, *aQueryEntry.pStr));
    SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, XML_FILTER_CONDITION, sal_True, sal_True);
}

void ScXMLExportDataPilot::WriteDPFilter(const ScQueryParam& aQueryParam)
{
    SCSIZE nQueryEntryCount = aQueryParam.GetEntryCount();
    if (nQueryEntryCount > 0)
    {
        sal_Bool bAnd(false);
        sal_Bool bOr(false);
        sal_Bool bHasEntries(sal_True);
        SCSIZE nEntries(0);
        SCSIZE j;

        for ( j = 0; (j < nQueryEntryCount) && bHasEntries; ++j)
        {
            ScQueryEntry aEntry = aQueryParam.GetEntry(j);
            if (aEntry.bDoQuery)
            {
                if (nEntries > 0)
                {
                    if (aEntry.eConnect == SC_AND)
                        bAnd = sal_True;
                    else
                        bOr = sal_True;
                }
                ++nEntries;
            }
            else
                bHasEntries = false;
        }
        nQueryEntryCount = nEntries;
        if (nQueryEntryCount)
        {
            if(!((aQueryParam.nCol1 == aQueryParam.nCol2) && (aQueryParam.nRow1 == aQueryParam.nRow2) &&
                        (static_cast<SCCOLROW>(aQueryParam.nCol1) == static_cast<SCCOLROW>(aQueryParam.nRow1)) &&
                        (aQueryParam.nCol1 == 0) && (aQueryParam.nTab == SCTAB_MAX)))
            {
                ScRange aConditionRange(aQueryParam.nCol1, aQueryParam.nRow1, aQueryParam.nTab,
                    aQueryParam.nCol2, aQueryParam.nRow2, aQueryParam.nTab);
                rtl::OUString sConditionRange;
                ScRangeStringConverter::GetStringFromRange( sConditionRange, aConditionRange, pDoc, ::formula::FormulaGrammar::CONV_OOO );
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CONDITION_SOURCE_RANGE_ADDRESS, sConditionRange);
            }
            if (!aQueryParam.bDuplicate)
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY_DUPLICATES, XML_FALSE);
            SvXMLElementExport aElemDPF(rExport, XML_NAMESPACE_TABLE, XML_FILTER, sal_True, sal_True);
            rExport.CheckAttrList();
            if (nQueryEntryCount  == 1)
            {
                    WriteDPCondition(aQueryParam.GetEntry(0), aQueryParam.bCaseSens, aQueryParam.bRegExp);
            }
            else if (bOr && !bAnd)
            {
                SvXMLElementExport aElemOr(rExport, XML_NAMESPACE_TABLE, XML_FILTER_OR, sal_True, sal_True);
                for (j = 0; j < nQueryEntryCount; ++j)
                {
                    WriteDPCondition(aQueryParam.GetEntry(j), aQueryParam.bCaseSens, aQueryParam.bRegExp);
                }
            }
            else if (bAnd && !bOr)
            {
                SvXMLElementExport aElemAnd(rExport, XML_NAMESPACE_TABLE, XML_FILTER_AND, sal_True, sal_True);
                for (j = 0; j < nQueryEntryCount; ++j)
                {
                    WriteDPCondition(aQueryParam.GetEntry(j), aQueryParam.bCaseSens, aQueryParam.bRegExp);
                }
            }
            else
            {
                SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, XML_FILTER_OR, sal_True, sal_True);
                ScQueryEntry aPrevFilterField(aQueryParam.GetEntry(0));
                ScQueryConnect aConnection = aQueryParam.GetEntry(1).eConnect;
                sal_Bool bOpenAndElement;
                rtl::OUString aName(rExport.GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TABLE, GetXMLToken(XML_FILTER_AND)));
                if (aConnection == SC_AND)
                {
                    rExport.StartElement( aName, sal_True );
                    bOpenAndElement = sal_True;
                }
                else
                    bOpenAndElement = false;
                for (j = 1; j < nQueryEntryCount; ++j)
                {
                    if (aConnection != aQueryParam.GetEntry(j).eConnect)
                    {
                        aConnection = aQueryParam.GetEntry(j).eConnect;
                        if (aQueryParam.GetEntry(j).eConnect == SC_AND)
                        {
                            rExport.StartElement( aName, sal_True );
                            bOpenAndElement = sal_True;
                            WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                            aPrevFilterField = aQueryParam.GetEntry(j);
                            if (j == nQueryEntryCount - 1)
                            {
                                WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                                rExport.EndElement(aName, sal_True);
                                bOpenAndElement = false;
                            }
                        }
                        else
                        {
                            WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                            aPrevFilterField = aQueryParam.GetEntry(j);
                            if (bOpenAndElement)
                            {
                                rExport.EndElement(aName, sal_True);
                                bOpenAndElement = false;
                            }
                            if (j == nQueryEntryCount - 1)
                            {
                                WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                            }
                        }
                    }
                    else
                    {
                        WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                        aPrevFilterField = aQueryParam.GetEntry(j);
                        if (j == nQueryEntryCount - 1)
                            WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                    }
                }
            }
        }
    }
}

void ScXMLExportDataPilot::WriteFieldReference(ScDPSaveDimension* pDim)
{
    const sheet::DataPilotFieldReference* pRef = pDim->GetReferenceValue();
    if (pRef)
    {
        rtl::OUString sValueStr;
        switch (pRef->ReferenceType)
        {
            case sheet::DataPilotFieldReferenceType::NONE :
                sValueStr = GetXMLToken(XML_NONE);
                break;
            case sheet::DataPilotFieldReferenceType::ITEM_DIFFERENCE :
                sValueStr = GetXMLToken(XML_MEMBER_DIFFERENCE);
                break;
            case sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE :
                sValueStr = GetXMLToken(XML_MEMBER_PERCENTAGE);
                break;
            case sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE :
                sValueStr = GetXMLToken(XML_MEMBER_PERCENTAGE_DIFFERENCE);
                break;
            case sheet::DataPilotFieldReferenceType::RUNNING_TOTAL :
                sValueStr = GetXMLToken(XML_RUNNING_TOTAL);
                break;
            case sheet::DataPilotFieldReferenceType::ROW_PERCENTAGE :
                sValueStr = GetXMLToken(XML_ROW_PERCENTAGE);
                break;
            case sheet::DataPilotFieldReferenceType::COLUMN_PERCENTAGE :
                sValueStr = GetXMLToken(XML_COLUMN_PERCENTAGE);
                break;
            case sheet::DataPilotFieldReferenceType::TOTAL_PERCENTAGE :
                sValueStr = GetXMLToken(XML_TOTAL_PERCENTAGE);
                break;
            case sheet::DataPilotFieldReferenceType::INDEX :
                sValueStr = GetXMLToken(XML_INDEX);
                break;
        }
        if (sValueStr.getLength())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TYPE, sValueStr);

        if (pRef->ReferenceField.getLength())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FIELD_NAME, pRef->ReferenceField);

        if (pRef->ReferenceItemType == sheet::DataPilotFieldReferenceItemType::NAMED)
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_MEMBER_TYPE, XML_NAMED);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_MEMBER_NAME, pRef->ReferenceItemName);
        }
        else
        {
            sValueStr = rtl::OUString();
            switch(pRef->ReferenceItemType)
            {
                case sheet::DataPilotFieldReferenceItemType::PREVIOUS :
                sValueStr = GetXMLToken(XML_PREVIOUS);
                break;
                case sheet::DataPilotFieldReferenceItemType::NEXT :
                sValueStr = GetXMLToken(XML_NEXT);
                break;
            }
            if (sValueStr.getLength())
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_MEMBER_TYPE, sValueStr);
        }
        SvXMLElementExport aElemDPFR(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_FIELD_REFERENCE, sal_True, sal_True);
    }
    rExport.CheckAttrList();
}

void ScXMLExportDataPilot::WriteSortInfo(ScDPSaveDimension* pDim)
{
    const sheet::DataPilotFieldSortInfo* pSortInfo = pDim->GetSortInfo();
    if (pSortInfo)
    {
        if (pSortInfo->IsAscending)
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ORDER, XML_ASCENDING);
        else
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ORDER, XML_DESCENDING);

        rtl::OUString sValueStr;
        switch (pSortInfo->Mode)
        {
            case sheet::DataPilotFieldSortMode::NONE:
            sValueStr = GetXMLToken(XML_NONE);
            break;
            case sheet::DataPilotFieldSortMode::MANUAL:
            sValueStr = GetXMLToken(XML_MANUAL);
            break;
            case sheet::DataPilotFieldSortMode::NAME:
            sValueStr = GetXMLToken(XML_NAME);
            break;
            case sheet::DataPilotFieldSortMode::DATA:
            sValueStr = GetXMLToken(XML_DATA);
            if (pSortInfo->Field.getLength())
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATA_FIELD, pSortInfo->Field);
            break;
        }
        if (sValueStr.getLength())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_SORT_MODE, sValueStr);
        SvXMLElementExport aElemDPLSI(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_SORT_INFO, sal_True, sal_True);
    }
}

void ScXMLExportDataPilot::WriteAutoShowInfo(ScDPSaveDimension* pDim)
{
    const sheet::DataPilotFieldAutoShowInfo* pAutoInfo = pDim->GetAutoShowInfo();
    if (pAutoInfo)
    {
        if (pAutoInfo->IsEnabled)
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ENABLED, XML_TRUE);
        else
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ENABLED, XML_FALSE);

        rtl::OUString sValueStr;
        switch (pAutoInfo->ShowItemsMode)
        {
            case sheet::DataPilotFieldShowItemsMode::FROM_TOP:
            sValueStr = GetXMLToken(XML_FROM_TOP);
            break;
            case sheet::DataPilotFieldShowItemsMode::FROM_BOTTOM:
            sValueStr = GetXMLToken(XML_FROM_BOTTOM);
            break;
        }
        if (sValueStr.getLength())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY_MEMBER_MODE, sValueStr);

        rtl::OUStringBuffer sBuffer;
        SvXMLUnitConverter::convertNumber(sBuffer, pAutoInfo->ItemCount);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_MEMBER_COUNT, sBuffer.makeStringAndClear());

        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATA_FIELD, pAutoInfo->DataField);

        SvXMLElementExport aElemDPLAI(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_DISPLAY_INFO, sal_True, sal_True);
    }
}

void ScXMLExportDataPilot::WriteLayoutInfo(ScDPSaveDimension* pDim)
{
    const sheet::DataPilotFieldLayoutInfo* pLayoutInfo = pDim->GetLayoutInfo();
    if (pLayoutInfo)
    {
        if (pLayoutInfo->AddEmptyLines)
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ADD_EMPTY_LINES, XML_TRUE);
        else
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ADD_EMPTY_LINES, XML_FALSE);

        rtl::OUString sValueStr;
        switch (pLayoutInfo->LayoutMode)
        {
            case sheet::DataPilotFieldLayoutMode::TABULAR_LAYOUT:
            sValueStr = GetXMLToken(XML_TABULAR_LAYOUT);
            break;
            case sheet::DataPilotFieldLayoutMode::OUTLINE_SUBTOTALS_TOP:
            sValueStr = GetXMLToken(XML_OUTLINE_SUBTOTALS_TOP);
            break;
            case sheet::DataPilotFieldLayoutMode::OUTLINE_SUBTOTALS_BOTTOM:
            sValueStr = GetXMLToken(XML_OUTLINE_SUBTOTALS_BOTTOM);
            break;
        }
        if (sValueStr.getLength())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_LAYOUT_MODE, sValueStr);
        SvXMLElementExport aElemDPLLI(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_LAYOUT_INFO, sal_True, sal_True);
    }
}

void ScXMLExportDataPilot::WriteSubTotals(ScDPSaveDimension* pDim)
{
    using sheet::GeneralFunction;

    sal_Int32 nSubTotalCount = pDim->GetSubTotalsCount();
    const OUString* pLayoutName = NULL;
    if (rExport.getDefaultVersion() == SvtSaveOptions::ODFVER_LATEST)
        // Export display names only for 1.2 extended or later.
        pLayoutName = pDim->GetSubtotalName();

    if (nSubTotalCount > 0)
    {
        SvXMLElementExport aElemSTs(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_SUBTOTALS, sal_True, sal_True);
        rExport.CheckAttrList();
        for (sal_Int32 nSubTotal = 0; nSubTotal < nSubTotalCount; nSubTotal++)
        {
            rtl::OUString sFunction;
            GeneralFunction nFunc = static_cast<GeneralFunction>(pDim->GetSubTotalFunc(nSubTotal));
            ScXMLConverter::GetStringFromFunction( sFunction, nFunc);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FUNCTION, sFunction);
            if (pLayoutName && nFunc == sheet::GeneralFunction_AUTO)
                rExport.AddAttribute(XML_NAMESPACE_TABLE_EXT, XML_DISPLAY_NAME, *pLayoutName);
            SvXMLElementExport aElemST(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_SUBTOTAL, sal_True, sal_True);
        }
    }
}

void ScXMLExportDataPilot::WriteMembers(ScDPSaveDimension* pDim)
{
    const ScDPSaveDimension::MemberList &rMembers = pDim->GetMembers();
    if (rMembers.begin() != rMembers.end())
    {
        SvXMLElementExport aElemDPMs(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_MEMBERS, sal_True, sal_True);
        rExport.CheckAttrList();
        for (ScDPSaveDimension::MemberList::const_iterator i=rMembers.begin(); i != rMembers.end() ; ++i)
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NAME, rtl::OUString((*i)->GetName()));

            if (rExport.getDefaultVersion() == SvtSaveOptions::ODFVER_LATEST)
            {
                // Export display names only for ODF 1.2 extended or later.
                const OUString* pLayoutName = (*i)->GetLayoutName();
                if (pLayoutName)
                    rExport.AddAttribute(XML_NAMESPACE_TABLE_EXT, XML_DISPLAY_NAME, *pLayoutName);
            }

            rtl::OUStringBuffer sBuffer;
            SvXMLUnitConverter::convertBool(sBuffer, (*i)->GetIsVisible());
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY, sBuffer.makeStringAndClear());
            SvXMLUnitConverter::convertBool(sBuffer, (*i)->GetShowDetails());
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_SHOW_DETAILS, sBuffer.makeStringAndClear());
            SvXMLElementExport aElemDPM(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_MEMBER, sal_True, sal_True);
            rExport.CheckAttrList();
        }
    }
}

void ScXMLExportDataPilot::WriteLevels(ScDPSaveDimension* pDim)
{
    // #i114202# GetShowEmpty is only valid if HasShowEmpty is true.
    if (pDim->HasShowEmpty())
    {
        rtl::OUStringBuffer sBuffer;
        SvXMLUnitConverter::convertBool(sBuffer, pDim->GetShowEmpty());
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_SHOW_EMPTY, sBuffer.makeStringAndClear());
    }
    SvXMLElementExport aElemDPL(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_LEVEL, sal_True, sal_True);

    WriteSubTotals(pDim);
    WriteMembers(pDim);
    WriteAutoShowInfo(pDim);
    WriteSortInfo(pDim);
    WriteLayoutInfo(pDim);
    rExport.CheckAttrList();
}

void ScXMLExportDataPilot::WriteDatePart(sal_Int32 nPart)
{
    switch(nPart)
    {
    case com::sun::star::sheet::DataPilotFieldGroupBy::SECONDS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUPED_BY, XML_SECONDS);
        }
        break;
    case com::sun::star::sheet::DataPilotFieldGroupBy::MINUTES :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUPED_BY, XML_MINUTES);
        }
        break;
    case com::sun::star::sheet::DataPilotFieldGroupBy::HOURS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUPED_BY, XML_HOURS);
        }
        break;
    case com::sun::star::sheet::DataPilotFieldGroupBy::DAYS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUPED_BY, XML_DAYS);
        }
        break;
    case com::sun::star::sheet::DataPilotFieldGroupBy::MONTHS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUPED_BY, XML_MONTHS);
        }
        break;
    case com::sun::star::sheet::DataPilotFieldGroupBy::QUARTERS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUPED_BY, XML_QUARTERS);
        }
        break;
    case com::sun::star::sheet::DataPilotFieldGroupBy::YEARS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUPED_BY, XML_YEARS);
        }
        break;
    }
}

void ScXMLExportDataPilot::WriteNumGroupInfo(const ScDPNumGroupInfo& rGroupInfo)
{
    DBG_ASSERT(rGroupInfo.Enable, "group dimension should be enabled");
    if (rGroupInfo.DateValues)
    {
        if (rGroupInfo.AutoStart)
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATE_START, XML_AUTO);
        else
        {
            rtl::OUStringBuffer sDate;
            rExport.GetMM100UnitConverter().convertDateTime(sDate, rGroupInfo.Start);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATE_START, sDate.makeStringAndClear());
        }
        if (rGroupInfo.AutoEnd)
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATE_END, XML_AUTO);
        else
        {
            rtl::OUStringBuffer sDate;
            rExport.GetMM100UnitConverter().convertDateTime(sDate, rGroupInfo.End);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATE_END, sDate.makeStringAndClear());
        }
    }
    else
    {
        if (rGroupInfo.AutoStart)
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_START, XML_AUTO);
        else
        {
            rtl::OUString sValue( ::rtl::math::doubleToUString( rGroupInfo.Start,
                        rtl_math_StringFormat_Automatic,
                        rtl_math_DecimalPlaces_Max, '.', sal_True));
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_START, sValue);
        }
        if (rGroupInfo.AutoEnd)
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_END, XML_AUTO);
        else
        {
            rtl::OUStringBuffer sDate;
            rtl::OUString sValue( ::rtl::math::doubleToUString( rGroupInfo.End,
                        rtl_math_StringFormat_Automatic,
                        rtl_math_DecimalPlaces_Max, '.', sal_True));
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_END, sValue);
        }
    }
    rtl::OUString sValue( ::rtl::math::doubleToUString( rGroupInfo.Step,
                rtl_math_StringFormat_Automatic,
                rtl_math_DecimalPlaces_Max, '.', sal_True));
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_STEP, sValue);
}

void ScXMLExportDataPilot::WriteGroupDimAttributes(const ScDPSaveGroupDimension* pGroupDim)
{
    if (pGroupDim)
    {
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_IS_GROUP_FIELD, XML_TRUE);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_SOURCE_FIELD_NAME, pGroupDim->GetSourceDimName());
        if (pGroupDim->GetDatePart())
        {
            WriteDatePart(pGroupDim->GetDatePart());
            WriteNumGroupInfo(pGroupDim->GetDateInfo());
        }
    }
}

void ScXMLExportDataPilot::WriteNumGroupDim(const ScDPSaveNumGroupDimension* pNumGroupDim)
{
    if (pNumGroupDim)
    {
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_IS_GROUP_FIELD, XML_TRUE);
        if (pNumGroupDim->GetDatePart())
        {
            WriteDatePart(pNumGroupDim->GetDatePart());
            WriteNumGroupInfo(pNumGroupDim->GetDateInfo());
        }
        else
        {
            WriteNumGroupInfo(pNumGroupDim->GetInfo());
        }
    }
}

void ScXMLExportDataPilot::WriteGroupDimElements(ScDPSaveDimension* pDim, const ScDPDimensionSaveData* pDimData)
{
    const ScDPSaveGroupDimension* pGroupDim = NULL;
    const ScDPSaveNumGroupDimension* pNumGroupDim = NULL;
    if (pDimData)
    {
        pGroupDim = pDimData->GetNamedGroupDim(pDim->GetName());
        WriteGroupDimAttributes(pGroupDim);
        pNumGroupDim = pDimData->GetNumGroupDim(pDim->GetName());
        WriteNumGroupDim(pNumGroupDim);

        DBG_ASSERT((!pGroupDim || !pNumGroupDim), "there should be no NumGroup and Group at the same field");
    }
    if (pGroupDim || pNumGroupDim)
    {
        SvXMLElementExport aElemDPGs(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_GROUPS, sal_True, sal_True);
        if (pGroupDim)
        {
            if (!pGroupDim->GetDatePart())
            {
                sal_Int32 nCount = pGroupDim->GetGroupCount();
                for (sal_Int32 i = 0; i < nCount; ++i)
                {
                    const ScDPSaveGroupItem* pGroup = pGroupDim->GetGroupByIndex( i );
                    if (pGroup)
                    {
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NAME, pGroup->GetGroupName());
                        SvXMLElementExport aElemDPG(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_GROUP, sal_True, sal_True);
                        sal_Int32 nElemCount = pGroup->GetElementCount();
                        for(sal_Int32 j = 0; j < nElemCount; ++j)
                        {
                            const String* pElem = pGroup->GetElementByIndex( j );
                            if (pElem)
                            {
                                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NAME, *pElem);
                                SvXMLElementExport aElemDPM(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_MEMBER, sal_True, sal_True);
                            }
                        }
                    }
                }
            }
        }
    }
}

void ScXMLExportDataPilot::WriteDimension(ScDPSaveDimension* pDim, const ScDPDimensionSaveData* pDimData)
{
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_SOURCE_FIELD_NAME, rtl::OUString(pDim->GetName()));
    if (rExport.getDefaultVersion() == SvtSaveOptions::ODFVER_LATEST)
    {
        // Export display names only for ODF 1.2 extended or later.
        const OUString* pLayoutName = pDim->GetLayoutName();
        if (pLayoutName)
            rExport.AddAttribute(XML_NAMESPACE_TABLE_EXT, XML_DISPLAY_NAME, *pLayoutName);
    }

    if (pDim->IsDataLayout())
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_IS_DATA_LAYOUT_FIELD, XML_TRUE);
    rtl::OUString sValueStr;
    ScXMLConverter::GetStringFromOrientation( sValueStr,
        (sheet::DataPilotFieldOrientation) pDim->GetOrientation() );
    if( sValueStr.getLength() )
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ORIENTATION, sValueStr );
    if (pDim->GetOrientation() == sheet::DataPilotFieldOrientation_PAGE)
        if (pDim->HasCurrentPage())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_SELECTED_PAGE, pDim->GetCurrentPage());
    if (pDim->GetUsedHierarchy() != 1)
    {
        rtl::OUStringBuffer sBuffer;
        SvXMLUnitConverter::convertNumber(sBuffer, pDim->GetUsedHierarchy());
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_USED_HIERARCHY, sBuffer.makeStringAndClear());
    }
    ScXMLConverter::GetStringFromFunction( sValueStr,
        (sheet::GeneralFunction) pDim->GetFunction() );
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FUNCTION, sValueStr);

    SvXMLElementExport aElemDPF(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_FIELD, sal_True, sal_True);
    WriteFieldReference(pDim);
    WriteLevels(pDim);
    if( pDim->GetOrientation() != sheet::DataPilotFieldOrientation_DATA )
        WriteGroupDimElements(pDim, pDimData);
}

void ScXMLExportDataPilot::WriteDimensions(ScDPSaveData* pDPSave)
{
    const boost::ptr_vector<ScDPSaveDimension> &rDimensions = pDPSave->GetDimensions();
    boost::ptr_vector<ScDPSaveDimension>::const_iterator iter;
    for (iter = rDimensions.begin(); iter != rDimensions.end(); ++iter)
        WriteDimension(const_cast<ScDPSaveDimension*>(&(*iter)), pDPSave->GetExistingDimensionData());
}

void ScXMLExportDataPilot::WriteGrandTotal(::xmloff::token::XMLTokenEnum eOrient, bool bVisible, const OUString* pGrandTotal)
{
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY, bVisible ? XML_TRUE : XML_FALSE);
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ORIENTATION, eOrient);
    if (pGrandTotal)
        rExport.AddAttribute(XML_NAMESPACE_TABLE_EXT, XML_DISPLAY_NAME, *pGrandTotal);

    SvXMLElementExport aElemGrandTotal(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_GRAND_TOTAL, sal_True, sal_True);
}

void ScXMLExportDataPilot::WriteDataPilots(const uno::Reference <sheet::XSpreadsheetDocument>& /* xSpreadDoc */)
{
    pDoc = rExport.GetDocument();
    if (!pDoc)
        return;

    ScDPCollection* pDPs = pDoc->GetDPCollection();
    if (!pDPs)
        return;

    size_t nDPCount = pDPs->GetCount();
    if (!nDPCount)
        return;

    SvXMLElementExport aElemDPs(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_TABLES, sal_True, sal_True);
    rExport.CheckAttrList();
    for (size_t i = 0; i < nDPCount; ++i)
    {
        ScDPSaveData* pDPSave = (*pDPs)[i]->GetSaveData();
        if (!pDPSave)
            continue;

        ScRange aOutRange((*pDPs)[i]->GetOutRange());
        rtl::OUString sTargetRangeAddress;
        ScRangeStringConverter::GetStringFromRange( sTargetRangeAddress, aOutRange, pDoc, ::formula::FormulaGrammar::CONV_OOO );
        ScDocAttrIterator aAttrItr(pDoc, aOutRange.aStart.Tab(),
            aOutRange.aStart.Col(), aOutRange.aStart.Row(),
            aOutRange.aEnd.Col(), aOutRange.aEnd.Row());
        SCCOL nCol;
        SCROW nRow1, nRow2;
        rtl::OUString sOUButtonList;
        const ScPatternAttr* pAttr = aAttrItr.GetNext(nCol, nRow1, nRow2);
        while (pAttr)
        {
            ScMergeFlagAttr& rItem = (ScMergeFlagAttr&)pAttr->GetItem(ATTR_MERGE_FLAG);
            if (rItem.HasButton())
            {
                for (SCROW nButtonRow = nRow1; nButtonRow <= nRow2; ++nButtonRow)
                {
                    ScAddress aButtonAddr(nCol, nButtonRow, aOutRange.aStart.Tab());
                    ScRangeStringConverter::GetStringFromAddress(
                        sOUButtonList, aButtonAddr, pDoc, ::formula::FormulaGrammar::CONV_OOO, ' ', sal_True );
                }
            }
            pAttr = aAttrItr.GetNext(nCol, nRow1, nRow2);
        }
        rtl::OUString sName((*pDPs)[i]->GetName());
        rtl::OUString sApplicationData((*pDPs)[i]->GetTag());
        sal_Bool bRowGrand = pDPSave->GetRowGrand();
        sal_Bool bColumnGrand = pDPSave->GetColumnGrand();
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NAME, sName);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_APPLICATION_DATA, sApplicationData);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS, sTargetRangeAddress);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_BUTTONS, sOUButtonList);
        if (!(bRowGrand && bColumnGrand))
        {
            if (bRowGrand)
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GRAND_TOTAL, XML_ROW);
            else if (bColumnGrand)
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GRAND_TOTAL, XML_COLUMN);
            else
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GRAND_TOTAL, XML_NONE);
        }
        if (pDPSave->GetIgnoreEmptyRows())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_IGNORE_EMPTY_ROWS, XML_TRUE);
        if (pDPSave->GetRepeatIfEmpty())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_IDENTIFY_CATEGORIES, XML_TRUE);
        if (!pDPSave->GetFilterButton())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_SHOW_FILTER_BUTTON, XML_FALSE);
        if (!pDPSave->GetDrillDown())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DRILL_DOWN_ON_DOUBLE_CLICK, XML_FALSE);
        if ((*pDPs)[i]->GetHeaderLayout())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_HEADER_GRID_LAYOUT, XML_TRUE);

        SvXMLElementExport aElemDP(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_TABLE, sal_True, sal_True);

        // grand total elements.

        const OUString* pGrandTotalName = pDPSave->GetGrandTotalName();
        if (pGrandTotalName && rExport.getDefaultVersion() == SvtSaveOptions::ODFVER_LATEST)
        {
            // Use the new data-pilot-grand-total element.
            if (bRowGrand && bColumnGrand)
            {
                WriteGrandTotal(XML_BOTH, true, pGrandTotalName);
            }
            else
            {
                WriteGrandTotal(XML_ROW, bRowGrand, pGrandTotalName);
                WriteGrandTotal(XML_COLUMN, bColumnGrand, pGrandTotalName);
            }
        }

        rExport.CheckAttrList();
        if ((*pDPs)[i]->IsSheetData())
        {
            const ScSheetSourceDesc* pSheetSource = (*pDPs)[i]->GetSheetDesc();

            if (rExport.getDefaultVersion() == SvtSaveOptions::ODFVER_LATEST)
            {
                if (pSheetSource->HasRangeName())
                    rExport.AddAttribute(
                        XML_NAMESPACE_TABLE, XML_NAME, pSheetSource->GetRangeName());
            }

            OUString sCellRangeAddress;
            ScRangeStringConverter::GetStringFromRange(
                sCellRangeAddress, pSheetSource->GetSourceRange(), pDoc,
                ::formula::FormulaGrammar::CONV_OOO);

            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS, sCellRangeAddress);
            SvXMLElementExport aElemSCR(rExport, XML_NAMESPACE_TABLE, XML_SOURCE_CELL_RANGE, sal_True, sal_True);
            rExport.CheckAttrList();
            WriteDPFilter(pSheetSource->GetQueryParam());
        }
        else if ((*pDPs)[i]->IsImportData())
        {
            const ScImportSourceDesc* pImpSource = (*pDPs)[i]->GetImportSourceDesc();
            switch (pImpSource->nType)
            {
                case sheet::DataImportMode_NONE : break;
                case sheet::DataImportMode_QUERY :
                {
                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATABASE_NAME, rtl::OUString(pImpSource->aDBName));
                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_QUERY_NAME, rtl::OUString(pImpSource->aObject));
                    SvXMLElementExport aElemID(rExport, XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_QUERY, sal_True, sal_True);
                    rExport.CheckAttrList();
                }
                break;
                case sheet::DataImportMode_TABLE :
                {
                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATABASE_NAME, rtl::OUString(pImpSource->aDBName));
                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TABLE_NAME, rtl::OUString(pImpSource->aObject));
                    SvXMLElementExport aElemID(rExport, XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_TABLE, sal_True, sal_True);
                    rExport.CheckAttrList();
                }
                break;
                case sheet::DataImportMode_SQL :
                {
                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATABASE_NAME, rtl::OUString(pImpSource->aDBName));
                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_SQL_STATEMENT, rtl::OUString(pImpSource->aObject));
                    if (!pImpSource->bNative)
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_PARSE_SQL_STATEMENT, XML_TRUE);
                    SvXMLElementExport aElemID(rExport, XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_SQL, sal_True, sal_True);
                    rExport.CheckAttrList();
                }
                break;
            }
        }
        else if ((*pDPs)[i]->IsServiceData())
        {
            const ScDPServiceDesc* pServSource = (*pDPs)[i]->GetDPServiceDesc();
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NAME, rtl::OUString(pServSource->aServiceName));
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_SOURCE_NAME, rtl::OUString(pServSource->aParSource));
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_OBJECT_NAME, rtl::OUString(pServSource->aParName));
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_USER_NAME, rtl::OUString(pServSource->aParUser));
            // #i111754# leave out password attribute as long as DataPilotSource doesn't specify the content
            // rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_PASSWORD, rtl::OUString(pServSource->aParPass));
            SvXMLElementExport aElemSD(rExport, XML_NAMESPACE_TABLE, XML_SOURCE_SERVICE, sal_True, sal_True);
            rExport.CheckAttrList();
        }
        WriteDimensions(pDPSave);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
