/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "XMLExportDataPilot.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/nmspmap.hxx>
#include <sax/tools/converter.hxx>
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
#include "dputil.hxx"
#include "rangeutl.hxx"
#include "queryentry.hxx"
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

ScXMLExportDataPilot::ScXMLExportDataPilot(ScXMLExport& rTempExport)
    : rExport(rTempExport),
    pDoc( nullptr )
{
}

ScXMLExportDataPilot::~ScXMLExportDataPilot()
{
}

OUString ScXMLExportDataPilot::getDPOperatorXML(
    const ScQueryOp aFilterOperator, const utl::SearchParam::SearchType eSearchType)
{
    switch (aFilterOperator)
    {
        case SC_EQUAL :
        {
            if (eSearchType == utl::SearchParam::SRCH_REGEXP)
                return GetXMLToken(XML_MATCH);
            else
                return OUString("=");
        }
        case SC_NOT_EQUAL :
        {
            if (eSearchType == utl::SearchParam::SRCH_REGEXP)
                return GetXMLToken(XML_NOMATCH);
            else
                return OUString("!=");
        }
        case SC_BOTPERC :
            return GetXMLToken(XML_BOTTOM_PERCENT);
        case SC_BOTVAL :
            return GetXMLToken(XML_BOTTOM_VALUES);
        case SC_GREATER :
            return OUString(">");
        case SC_GREATER_EQUAL :
            return OUString(">=");
        case SC_LESS :
            return OUString("<");
        case SC_LESS_EQUAL :
            return OUString("<=");
        case SC_TOPPERC :
            return GetXMLToken(XML_TOP_PERCENT);
        case SC_TOPVAL :
            return GetXMLToken(XML_TOP_VALUES);
        default:
            OSL_FAIL("This FilterOperator is not supported.");
    }
    return OUString("=");
}

void ScXMLExportDataPilot::WriteDPCondition(const ScQueryEntry& aQueryEntry, bool bIsCaseSensitive,
        utl::SearchParam::SearchType eSearchType)
{
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FIELD_NUMBER, OUString::number(aQueryEntry.nField));
    if (bIsCaseSensitive)
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CASE_SENSITIVE, XML_TRUE);
    const ScQueryEntry::Item& rItem = aQueryEntry.GetQueryItem();
    OUString aQueryStr = rItem.maString.getString();
    if (rItem.meType == ScQueryEntry::ByString)
    {
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_VALUE, aQueryStr);
    }
    else
    {
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATA_TYPE, XML_NUMBER);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_VALUE, aQueryStr);
    }

    if (aQueryEntry.IsQueryByEmpty())
    {
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_OPERATOR, GetXMLToken(XML_EMPTY));
    }
    else if (aQueryEntry.IsQueryByNonEmpty())
    {
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_OPERATOR, GetXMLToken(XML_NOEMPTY));
    }
    else
        rExport.AddAttribute(
            XML_NAMESPACE_TABLE, XML_OPERATOR,
            getDPOperatorXML(aQueryEntry.eOp, eSearchType));

    SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, XML_FILTER_CONDITION, true, true);
}

void ScXMLExportDataPilot::WriteDPFilter(const ScQueryParam& aQueryParam)
{
    SCSIZE nQueryEntryCount = aQueryParam.GetEntryCount();
    if (nQueryEntryCount > 0)
    {
        bool bAnd(false);
        bool bOr(false);
        bool bHasEntries(true);
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
                        bAnd = true;
                    else
                        bOr = true;
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
                OUString sConditionRange;
                ScRangeStringConverter::GetStringFromRange( sConditionRange, aConditionRange, pDoc, ::formula::FormulaGrammar::CONV_OOO );
                if (!sConditionRange.isEmpty())
                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CONDITION_SOURCE_RANGE_ADDRESS, sConditionRange);
            }
            if (!aQueryParam.bDuplicate)
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY_DUPLICATES, XML_FALSE);
            SvXMLElementExport aElemDPF(rExport, XML_NAMESPACE_TABLE, XML_FILTER, true, true);
            rExport.CheckAttrList();
            if (nQueryEntryCount  == 1)
            {
                    WriteDPCondition(aQueryParam.GetEntry(0), aQueryParam.bCaseSens, aQueryParam.eSearchType);
            }
            else if (bOr && !bAnd)
            {
                SvXMLElementExport aElemOr(rExport, XML_NAMESPACE_TABLE, XML_FILTER_OR, true, true);
                for (j = 0; j < nQueryEntryCount; ++j)
                {
                    WriteDPCondition(aQueryParam.GetEntry(j), aQueryParam.bCaseSens, aQueryParam.eSearchType);
                }
            }
            else if (bAnd && !bOr)
            {
                SvXMLElementExport aElemAnd(rExport, XML_NAMESPACE_TABLE, XML_FILTER_AND, true, true);
                for (j = 0; j < nQueryEntryCount; ++j)
                {
                    WriteDPCondition(aQueryParam.GetEntry(j), aQueryParam.bCaseSens, aQueryParam.eSearchType);
                }
            }
            else
            {
                SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, XML_FILTER_OR, true, true);
                ScQueryEntry aPrevFilterField(aQueryParam.GetEntry(0));
                ScQueryConnect aConnection = aQueryParam.GetEntry(1).eConnect;
                bool bOpenAndElement;
                OUString aName(rExport.GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TABLE, GetXMLToken(XML_FILTER_AND)));
                if (aConnection == SC_AND)
                {
                    rExport.StartElement( aName, true );
                    bOpenAndElement = true;
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
                            rExport.StartElement( aName, true );
                            bOpenAndElement = true;
                            WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.eSearchType);
                            aPrevFilterField = aQueryParam.GetEntry(j);
                            if (j == nQueryEntryCount - 1)
                            {
                                WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.eSearchType);
                                rExport.EndElement(aName, true);
                                bOpenAndElement = false;
                            }
                        }
                        else
                        {
                            WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.eSearchType);
                            aPrevFilterField = aQueryParam.GetEntry(j);
                            if (bOpenAndElement)
                            {
                                rExport.EndElement(aName, true);
                                bOpenAndElement = false;
                            }
                            if (j == nQueryEntryCount - 1)
                            {
                                WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.eSearchType);
                            }
                        }
                    }
                    else
                    {
                        WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.eSearchType);
                        aPrevFilterField = aQueryParam.GetEntry(j);
                        if (j == nQueryEntryCount - 1)
                            WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.eSearchType);
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
        OUString sValueStr;
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
        if (!sValueStr.isEmpty())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TYPE, sValueStr);

        if (!pRef->ReferenceField.isEmpty())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FIELD_NAME, pRef->ReferenceField);

        if (pRef->ReferenceItemType == sheet::DataPilotFieldReferenceItemType::NAMED)
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_MEMBER_TYPE, XML_NAMED);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_MEMBER_NAME, pRef->ReferenceItemName);
        }
        else
        {
            sValueStr.clear();
            switch(pRef->ReferenceItemType)
            {
                case sheet::DataPilotFieldReferenceItemType::PREVIOUS :
                sValueStr = GetXMLToken(XML_PREVIOUS);
                break;
                case sheet::DataPilotFieldReferenceItemType::NEXT :
                sValueStr = GetXMLToken(XML_NEXT);
                break;
            }
            if (!sValueStr.isEmpty())
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_MEMBER_TYPE, sValueStr);
        }
        SvXMLElementExport aElemDPFR(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_FIELD_REFERENCE, true, true);
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

        OUString sValueStr;
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
            if (!pSortInfo->Field.isEmpty())
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATA_FIELD, pSortInfo->Field);
            break;
        }
        if (!sValueStr.isEmpty())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_SORT_MODE, sValueStr);
        SvXMLElementExport aElemDPLSI(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_SORT_INFO, true, true);
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

        OUString sValueStr;
        switch (pAutoInfo->ShowItemsMode)
        {
            case sheet::DataPilotFieldShowItemsMode::FROM_TOP:
            sValueStr = GetXMLToken(XML_FROM_TOP);
            break;
            case sheet::DataPilotFieldShowItemsMode::FROM_BOTTOM:
            sValueStr = GetXMLToken(XML_FROM_BOTTOM);
            break;
        }
        if (!sValueStr.isEmpty())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY_MEMBER_MODE, sValueStr);

        OUStringBuffer sBuffer;
        ::sax::Converter::convertNumber(sBuffer, pAutoInfo->ItemCount);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_MEMBER_COUNT, sBuffer.makeStringAndClear());

        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATA_FIELD, pAutoInfo->DataField);

        SvXMLElementExport aElemDPLAI(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_DISPLAY_INFO, true, true);
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

        OUString sValueStr;
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
        if (!sValueStr.isEmpty())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_LAYOUT_MODE, sValueStr);
        SvXMLElementExport aElemDPLLI(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_LAYOUT_INFO, true, true);
    }
}

void ScXMLExportDataPilot::WriteSubTotals(ScDPSaveDimension* pDim)
{
    using sheet::GeneralFunction;

    sal_Int32 nSubTotalCount = pDim->GetSubTotalsCount();
    const OUString* pLayoutName = nullptr;
    if (rExport.getDefaultVersion() > SvtSaveOptions::ODFVER_012)
        // Export display names only for 1.2 extended or later.
        pLayoutName = pDim->GetSubtotalName();

    if (nSubTotalCount > 0)
    {
        SvXMLElementExport aElemSTs(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_SUBTOTALS, true, true);
        rExport.CheckAttrList();
        for (sal_Int32 nSubTotal = 0; nSubTotal < nSubTotalCount; nSubTotal++)
        {
            OUString sFunction;
            GeneralFunction nFunc = static_cast<GeneralFunction>(pDim->GetSubTotalFunc(nSubTotal));
            ScXMLConverter::GetStringFromFunction( sFunction, nFunc);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FUNCTION, sFunction);
            if (pLayoutName && nFunc == sheet::GeneralFunction_AUTO)
                rExport.AddAttribute(XML_NAMESPACE_TABLE_EXT, XML_DISPLAY_NAME, *pLayoutName);
            SvXMLElementExport aElemST(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_SUBTOTAL, true, true);
        }
    }
}

void ScXMLExportDataPilot::WriteMembers(ScDPSaveDimension* pDim)
{
    const ScDPSaveDimension::MemberList &rMembers = pDim->GetMembers();
    if (!rMembers.empty())
    {
        SvXMLElementExport aElemDPMs(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_MEMBERS, true, true);
        rExport.CheckAttrList();
        for (ScDPSaveDimension::MemberList::const_iterator i=rMembers.begin(); i != rMembers.end() ; ++i)
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NAME, OUString((*i)->GetName()));

            if (rExport.getDefaultVersion() > SvtSaveOptions::ODFVER_012)
            {
                // Export display names only for ODF 1.2 extended or later.
                const OUString* pLayoutName = (*i)->GetLayoutName();
                if (pLayoutName)
                    rExport.AddAttribute(XML_NAMESPACE_TABLE_EXT, XML_DISPLAY_NAME, *pLayoutName);
            }

            OUStringBuffer sBuffer;
            ::sax::Converter::convertBool(sBuffer, (*i)->GetIsVisible());
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY, sBuffer.makeStringAndClear());
            ::sax::Converter::convertBool(sBuffer, (*i)->GetShowDetails());
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_SHOW_DETAILS, sBuffer.makeStringAndClear());
            SvXMLElementExport aElemDPM(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_MEMBER, true, true);
            rExport.CheckAttrList();
        }
    }
}

void ScXMLExportDataPilot::WriteLevels(ScDPSaveDimension* pDim)
{
    // #i114202# GetShowEmpty is only valid if HasShowEmpty is true.
    if (pDim->HasShowEmpty())
    {
        OUStringBuffer sBuffer;
        ::sax::Converter::convertBool(sBuffer, pDim->GetShowEmpty());
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_SHOW_EMPTY, sBuffer.makeStringAndClear());
    }
    {
        OUStringBuffer sBuffer;
        ::sax::Converter::convertBool(sBuffer, pDim->GetRepeatItemLabels());
        rExport.AddAttribute(XML_NAMESPACE_CALC_EXT, XML_REPEAT_ITEM_LABELS, sBuffer.makeStringAndClear());
    }
    SvXMLElementExport aElemDPL(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_LEVEL, true, true);

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
    case css::sheet::DataPilotFieldGroupBy::SECONDS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUPED_BY, XML_SECONDS);
        }
        break;
    case css::sheet::DataPilotFieldGroupBy::MINUTES :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUPED_BY, XML_MINUTES);
        }
        break;
    case css::sheet::DataPilotFieldGroupBy::HOURS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUPED_BY, XML_HOURS);
        }
        break;
    case css::sheet::DataPilotFieldGroupBy::DAYS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUPED_BY, XML_DAYS);
        }
        break;
    case css::sheet::DataPilotFieldGroupBy::MONTHS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUPED_BY, XML_MONTHS);
        }
        break;
    case css::sheet::DataPilotFieldGroupBy::QUARTERS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUPED_BY, XML_QUARTERS);
        }
        break;
    case css::sheet::DataPilotFieldGroupBy::YEARS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUPED_BY, XML_YEARS);
        }
        break;
    }
}

void ScXMLExportDataPilot::WriteNumGroupInfo(const ScDPNumGroupInfo& rGroupInfo)
{
    OSL_ENSURE(rGroupInfo.mbEnable, "group dimension should be enabled");
    if (rGroupInfo.mbDateValues)
    {
        if (rGroupInfo.mbAutoStart)
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATE_START, XML_AUTO);
        else
        {
            OUStringBuffer sDate;
            rExport.GetMM100UnitConverter().convertDateTime(sDate, rGroupInfo.mfStart);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATE_START, sDate.makeStringAndClear());
        }
        if (rGroupInfo.mbAutoEnd)
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATE_END, XML_AUTO);
        else
        {
            OUStringBuffer sDate;
            rExport.GetMM100UnitConverter().convertDateTime(sDate, rGroupInfo.mfEnd);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATE_END, sDate.makeStringAndClear());
        }
    }
    else
    {
        if (rGroupInfo.mbAutoStart)
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_START, XML_AUTO);
        else
        {
            OUString sValue( ::rtl::math::doubleToUString( rGroupInfo.mfStart,
                        rtl_math_StringFormat_Automatic,
                        rtl_math_DecimalPlaces_Max, '.', true));
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_START, sValue);
        }
        if (rGroupInfo.mbAutoEnd)
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_END, XML_AUTO);
        else
        {
            OUString sValue( ::rtl::math::doubleToUString( rGroupInfo.mfEnd,
                        rtl_math_StringFormat_Automatic,
                        rtl_math_DecimalPlaces_Max, '.', true));
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_END, sValue);
        }
    }
    OUString sValue( ::rtl::math::doubleToUString( rGroupInfo.mfStep,
                rtl_math_StringFormat_Automatic,
                rtl_math_DecimalPlaces_Max, '.', true));
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_STEP, sValue);
}

void ScXMLExportDataPilot::WriteGroupDimAttributes(const ScDPSaveGroupDimension* pGroupDim)
{
    if (pGroupDim)
    {
        OUString aSrcFieldName = ScDPUtil::getSourceDimensionName(pGroupDim->GetSourceDimName());
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_SOURCE_FIELD_NAME, aSrcFieldName);
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
    const ScDPSaveGroupDimension* pGroupDim = nullptr;
    const ScDPSaveNumGroupDimension* pNumGroupDim = nullptr;
    if (pDimData)
    {
        pGroupDim = pDimData->GetNamedGroupDim(pDim->GetName());
        WriteGroupDimAttributes(pGroupDim);
        pNumGroupDim = pDimData->GetNumGroupDim(pDim->GetName());
        WriteNumGroupDim(pNumGroupDim);

        OSL_ENSURE((!pGroupDim || !pNumGroupDim), "there should be no NumGroup and Group at the same field");
    }
    if (pGroupDim || pNumGroupDim)
    {
        SvXMLElementExport aElemDPGs(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_GROUPS, true, true);
        if (pGroupDim)
        {
            if (!pGroupDim->GetDatePart())
            {
                sal_Int32 nCount = pGroupDim->GetGroupCount();
                for (sal_Int32 i = 0; i < nCount; ++i)
                {
                    const ScDPSaveGroupItem& rGroup = pGroupDim->GetGroupByIndex( i );
                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NAME, rGroup.GetGroupName());
                    SvXMLElementExport aElemDPG(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_GROUP, true, true);
                    sal_Int32 nElemCount = rGroup.GetElementCount();
                    for(sal_Int32 j = 0; j < nElemCount; ++j)
                    {
                        const OUString* pElem = rGroup.GetElementByIndex(j);
                        if (pElem)
                        {
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NAME, *pElem);
                            SvXMLElementExport aElemDPM(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_GROUP_MEMBER, true, true);
                        }
                    }
                }
            }
        }
    }
}

void ScXMLExportDataPilot::WriteDimension(ScDPSaveDimension* pDim, const ScDPDimensionSaveData* pDimData)
{
    OUString aSrcDimName = ScDPUtil::getSourceDimensionName(pDim->GetName());
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_SOURCE_FIELD_NAME, aSrcDimName);
    if (rExport.getDefaultVersion() > SvtSaveOptions::ODFVER_012)
    {
        // Export display names only for ODF 1.2 extended or later.
        const OUString* pLayoutName = pDim->GetLayoutName();
        if (pLayoutName)
            rExport.AddAttribute(XML_NAMESPACE_TABLE_EXT, XML_DISPLAY_NAME, *pLayoutName);
    }

    if (pDim->IsDataLayout())
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_IS_DATA_LAYOUT_FIELD, XML_TRUE);
    OUString sValueStr;
    sheet::DataPilotFieldOrientation eOrientation = (sheet::DataPilotFieldOrientation) pDim->GetOrientation();
    ScXMLConverter::GetStringFromOrientation( sValueStr,
         eOrientation);
    if( !sValueStr.isEmpty() )
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ORIENTATION, sValueStr );
    if (pDim->GetUsedHierarchy() != 1)
    {
        OUStringBuffer sBuffer;
        ::sax::Converter::convertNumber(sBuffer, pDim->GetUsedHierarchy());
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_USED_HIERARCHY, sBuffer.makeStringAndClear());
    }
    ScXMLConverter::GetStringFromFunction( sValueStr,
        (sheet::GeneralFunction) pDim->GetFunction() );
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FUNCTION, sValueStr);

    if (eOrientation == sheet::DataPilotFieldOrientation_PAGE)
    {
        if (rExport.getDefaultVersion() > SvtSaveOptions::ODFVER_012)
        {
            rExport.AddAttribute(XML_NAMESPACE_LO_EXT, XML_IGNORE_SELECTED_PAGE, "true");
        }
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_SELECTED_PAGE, pDim->GetCurrentPage());
    }

    SvXMLElementExport aElemDPF(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_FIELD, true, true);
    WriteLevels(pDim);
    WriteFieldReference(pDim);
    if( pDim->GetOrientation() != sheet::DataPilotFieldOrientation_DATA )
        WriteGroupDimElements(pDim, pDimData);
}

void ScXMLExportDataPilot::WriteDimensions(ScDPSaveData* pDPSave)
{
    const ScDPSaveData::DimsType& rDimensions = pDPSave->GetDimensions();
    for (auto const& iter : rDimensions)
    {
        WriteDimension(iter.get(),
                pDPSave->GetExistingDimensionData());
    }
}

void ScXMLExportDataPilot::WriteGrandTotal(::xmloff::token::XMLTokenEnum eOrient, bool bVisible, const OUString* pGrandTotal)
{
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY, bVisible ? XML_TRUE : XML_FALSE);
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ORIENTATION, eOrient);
    if (pGrandTotal)
        rExport.AddAttribute(XML_NAMESPACE_TABLE_EXT, XML_DISPLAY_NAME, *pGrandTotal);

    SvXMLElementExport aElemGrandTotal(rExport, XML_NAMESPACE_TABLE_EXT, XML_DATA_PILOT_GRAND_TOTAL, true, true);
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

    SvXMLElementExport aElemDPs(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_TABLES, true, true);
    rExport.CheckAttrList();
    for (size_t i = 0; i < nDPCount; ++i)
    {
        ScDPSaveData* pDPSave = (*pDPs)[i].GetSaveData();
        if (!pDPSave)
            continue;

        ScRange aOutRange((*pDPs)[i].GetOutRange());
        OUString sTargetRangeAddress;
        ScRangeStringConverter::GetStringFromRange( sTargetRangeAddress, aOutRange, pDoc, ::formula::FormulaGrammar::CONV_OOO );
        ScDocAttrIterator aAttrItr(pDoc, aOutRange.aStart.Tab(),
            aOutRange.aStart.Col(), aOutRange.aStart.Row(),
            aOutRange.aEnd.Col(), aOutRange.aEnd.Row());
        SCCOL nCol;
        SCROW nRow1, nRow2;
        OUString sOUButtonList;
        const ScPatternAttr* pAttr = aAttrItr.GetNext(nCol, nRow1, nRow2);
        while (pAttr)
        {
            const ScMergeFlagAttr& rItem = static_cast<const ScMergeFlagAttr&>(pAttr->GetItem(ATTR_MERGE_FLAG));
            if (rItem.HasPivotButton())
            {
                for (SCROW nButtonRow = nRow1; nButtonRow <= nRow2; ++nButtonRow)
                {
                    ScAddress aButtonAddr(nCol, nButtonRow, aOutRange.aStart.Tab());
                    ScRangeStringConverter::GetStringFromAddress(
                        sOUButtonList, aButtonAddr, pDoc, ::formula::FormulaGrammar::CONV_OOO, ' ', true );
                }
            }
            pAttr = aAttrItr.GetNext(nCol, nRow1, nRow2);
        }
        OUString sName((*pDPs)[i].GetName());
        OUString sApplicationData((*pDPs)[i].GetTag());
        bool bRowGrand = pDPSave->GetRowGrand();
        bool bColumnGrand = pDPSave->GetColumnGrand();
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
        if ((*pDPs)[i].GetHeaderLayout())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_HEADER_GRID_LAYOUT, XML_TRUE);

        SvXMLElementExport aElemDP(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_TABLE, true, true);

        // grand total elements.

        const OUString* pGrandTotalName = pDPSave->GetGrandTotalName();
        if (pGrandTotalName && rExport.getDefaultVersion() > SvtSaveOptions::ODFVER_012)
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
        if ((*pDPs)[i].IsSheetData())
        {
            const ScSheetSourceDesc* pSheetSource = (*pDPs)[i].GetSheetDesc();

            if (rExport.getDefaultVersion() > SvtSaveOptions::ODFVER_012)
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
            SvXMLElementExport aElemSCR(rExport, XML_NAMESPACE_TABLE, XML_SOURCE_CELL_RANGE, true, true);
            rExport.CheckAttrList();
            WriteDPFilter(pSheetSource->GetQueryParam());
        }
        else if ((*pDPs)[i].IsImportData())
        {
            const ScImportSourceDesc* pImpSource = (*pDPs)[i].GetImportSourceDesc();
            switch (pImpSource->nType)
            {
                case sheet::DataImportMode_NONE : break;
                case sheet::DataImportMode_QUERY :
                {
                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATABASE_NAME, pImpSource->aDBName);
                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_QUERY_NAME, pImpSource->aObject);
                    SvXMLElementExport aElemID(rExport, XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_QUERY, true, true);
                    rExport.CheckAttrList();
                }
                break;
                case sheet::DataImportMode_TABLE :
                {
                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATABASE_NAME, pImpSource->aDBName);
                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATABASE_TABLE_NAME, pImpSource->aObject);
                    SvXMLElementExport aElemID(rExport, XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_TABLE, true, true);
                    rExport.CheckAttrList();
                }
                break;
                case sheet::DataImportMode_SQL :
                {
                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATABASE_NAME, pImpSource->aDBName);
                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_SQL_STATEMENT, pImpSource->aObject);
                    if (!pImpSource->bNative)
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_PARSE_SQL_STATEMENT, XML_TRUE);
                    SvXMLElementExport aElemID(rExport, XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_SQL, true, true);
                    rExport.CheckAttrList();
                }
                break;
            }
        }
        else if ((*pDPs)[i].IsServiceData())
        {
            const ScDPServiceDesc* pServSource = (*pDPs)[i].GetDPServiceDesc();
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NAME, pServSource->aServiceName);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_SOURCE_NAME, pServSource->aParSource);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_OBJECT_NAME, pServSource->aParName);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_USER_NAME, pServSource->aParUser);
            // #i111754# leave out password attribute as long as DataPilotSource doesn't specify the content
            // rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_PASSWORD, OUString(pServSource->aParPass));
            SvXMLElementExport aElemSD(rExport, XML_NAMESPACE_TABLE, XML_SOURCE_SERVICE, true, true);
            rExport.CheckAttrList();
        }
        WriteDimensions(pDPSave);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
