/*************************************************************************
 *
 *  $RCSfile: XMLExportDataPilot.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 11:34:14 $
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
 *
 ************************************************************************/

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#ifndef _SC_XMLEXPORTDATAPILOT_HXX
#include "XMLExportDataPilot.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef SC_XMLEXPRT_HXX
#include "xmlexprt.hxx"
#endif
#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_DPOBJECT_HXX
#include "dpobject.hxx"
#endif
#ifndef SC_DOCITER_HXX
#include "dociter.hxx"
#endif
#ifndef SC_SCATTR_HXX
#include "attrib.hxx"
#endif
#ifndef SC_SCPATATR_HXX
#include "patattr.hxx"
#endif
#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif
#ifndef SC_DPSAVE_HXX
#include "dpsave.hxx"
#endif
#ifndef SC_DPSHTTAB_HXX
#include "dpshttab.hxx"
#endif
#ifndef SC_DPSDBTAB_HXX
#include "dpsdbtab.hxx"
#endif
#ifndef SC_DPDIMSAVE_HXX
#include "dpdimsave.hxx"
#endif
#ifndef SC_DPGROUP_HXX
#include "dpgroup.hxx"
#endif

#ifndef _COM_SUN_STAR_SHEET_DATAIMPORTMODE_HPP_
#include <com/sun/star/sheet/DataImportMode.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDREFERENCE_HPP_
#include <com/sun/star/sheet/DataPilotFieldReference.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDREFERENCETYPE_HPP_
#include <com/sun/star/sheet/DataPilotFieldReferenceType.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDREFERENCEITEMTYPE_HPP_
#include <com/sun/star/sheet/DataPilotFieldReferenceItemType.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDSORTINFO_HPP_
#include <com/sun/star/sheet/DataPilotFieldSortInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDAUTOSHOWINFO_HPP_
#include <com/sun/star/sheet/DataPilotFieldAutoShowInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDLAYOUTINFO_HPP_
#include <com/sun/star/sheet/DataPilotFieldLayoutInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDSHOWITEMSMODE_HPP_
#include <com/sun/star/sheet/DataPilotFieldShowItemsMode.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDSORTMODE_HPP_
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDLAYOUTMODE_HPP_
#include <com/sun/star/sheet/DataPilotFieldLayoutMode.hpp>
#endif

using namespace com::sun::star;
using namespace xmloff::token;

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
        break;
        case SC_NOT_EQUAL :
        {
            if (bUseRegularExpressions)
                return GetXMLToken(XML_NOMATCH);
            else
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("!="));
        }
        break;
        case SC_BOTPERC :
            return GetXMLToken(XML_BOTTOM_PERCENT);
            break;
        case SC_BOTVAL :
            return GetXMLToken(XML_BOTTOM_VALUES);
            break;
        case SC_GREATER :
            return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(">"));
            break;
        case SC_GREATER_EQUAL :
            return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(">="));
            break;
        case SC_LESS :
            return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<"));
            break;
        case SC_LESS_EQUAL :
            return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<="));
            break;
        case SC_TOPPERC :
            return GetXMLToken(XML_TOP_PERCENT);
            break;
        case SC_TOPVAL :
            return GetXMLToken(XML_TOP_VALUES);
            break;
        default:
            DBG_ERROR("This FilterOperator is not supported.");
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
        sal_Bool bAnd(sal_False);
        sal_Bool bOr(sal_False);
        sal_Bool bHasEntries(sal_True);
        SCSIZE nEntries(0);
        SCSIZE j;

        for ( j = 0; (j < nQueryEntryCount) && bHasEntries; j++)
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
                nEntries++;
            }
            else
                bHasEntries = sal_False;
        }
        nQueryEntryCount = nEntries;
        if (nQueryEntryCount)
        {
            // There is never a target range in a data pilot.
/*          if (!aQueryParam.bInplace)
            {
                ScAddress aTargetAddress(aQueryParam.nDestCol, aQueryParam.nDestRow, aQueryParam.nDestTab);
                rtl::OUString sAddress;
                ScXMLConverter::GetStringFromAddress( sAddress, aTargetAddress, pDoc );
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS, sAddress);
            }*/
            if(!((aQueryParam.nCol1 == aQueryParam.nCol2) && (aQueryParam.nRow1 == aQueryParam.nRow2) &&
                        (static_cast<SCCOLROW>(aQueryParam.nCol1) == static_cast<SCCOLROW>(aQueryParam.nRow1)) &&
                        (aQueryParam.nCol1 == 0) && (aQueryParam.nTab == SCTAB_MAX)))
            {
                ScRange aConditionRange(aQueryParam.nCol1, aQueryParam.nRow1, aQueryParam.nTab,
                    aQueryParam.nCol2, aQueryParam.nRow2, aQueryParam.nTab);
                rtl::OUString sConditionRange;
                ScXMLConverter::GetStringFromRange( sConditionRange, aConditionRange, pDoc );
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
                for (j = 0; j < nQueryEntryCount; j++)
                {
                    WriteDPCondition(aQueryParam.GetEntry(j), aQueryParam.bCaseSens, aQueryParam.bRegExp);
                }
            }
            else if (bAnd && !bOr)
            {
                SvXMLElementExport aElemAnd(rExport, XML_NAMESPACE_TABLE, XML_FILTER_AND, sal_True, sal_True);
                for (j = 0; j < nQueryEntryCount; j++)
                {
                    WriteDPCondition(aQueryParam.GetEntry(j), aQueryParam.bCaseSens, aQueryParam.bRegExp);
                }
            }
            else
            {
                SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, XML_FILTER_OR, sal_True, sal_True);
                ScQueryEntry aPrevFilterField = aQueryParam.GetEntry(0);
                ScQueryConnect aConnection = aQueryParam.GetEntry(1).eConnect;
                sal_Bool bOpenAndElement;
                rtl::OUString aName = rExport.GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TABLE, GetXMLToken(XML_FILTER_AND));
                if (aConnection == SC_AND)
                {
                    rExport.StartElement( aName, sal_True );
                    bOpenAndElement = sal_True;
                }
                else
                    bOpenAndElement = sal_False;
                for (j = 1; j < nQueryEntryCount; j++)
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
                                bOpenAndElement = sal_False;
                            }
                        }
                        else
                        {
                            WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                            aPrevFilterField = aQueryParam.GetEntry(j);
                            if (bOpenAndElement)
                            {
                                rExport.EndElement(aName, sal_True);
                                bOpenAndElement = sal_False;
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

        if (pAutoInfo->DataField.getLength())
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
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ADD_EMPTY_LINE, XML_TRUE);
        else
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ADD_EMPTY_LINE, XML_FALSE);

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
    sal_Int32 nSubTotalCount = pDim->GetSubTotalsCount();
    if (nSubTotalCount > 0)
    {
        SvXMLElementExport aElemSTs(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_SUBTOTALS, sal_True, sal_True);
        rExport.CheckAttrList();
        for (sal_Int32 nSubTotal = 0; nSubTotal < nSubTotalCount; nSubTotal++)
        {
            rtl::OUString sFunction;
            ScXMLConverter::GetStringFromFunction( sFunction, (sheet::GeneralFunction)pDim->GetSubTotalFunc(nSubTotal) );
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FUNCTION, sFunction);
            SvXMLElementExport aElemST(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_SUBTOTAL, sal_True, sal_True);
        }
    }
}

void ScXMLExportDataPilot::WriteMembers(ScDPSaveDimension* pDim)
{
    List aMembers = pDim->GetMembers();
    sal_Int32 nMemberCount = aMembers.Count();
    if (nMemberCount > 0)
    {
        SvXMLElementExport aElemDPMs(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_MEMBERS, sal_True, sal_True);
        rExport.CheckAttrList();
        for (sal_Int32 nMember = 0; nMember < nMemberCount; nMember++)
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NAME, rtl::OUString(((ScDPSaveMember*)aMembers.GetObject(nMember))->GetName()));
            rtl::OUStringBuffer sBuffer;
            SvXMLUnitConverter::convertBool(sBuffer, ((ScDPSaveMember*)aMembers.GetObject(nMember))->GetIsVisible());
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY, sBuffer.makeStringAndClear());
            SvXMLUnitConverter::convertBool(sBuffer, ((ScDPSaveMember*)aMembers.GetObject(nMember))->GetShowDetails());
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY_DETAILS, sBuffer.makeStringAndClear());
            SvXMLElementExport aElemDPM(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_MEMBER, sal_True, sal_True);
            rExport.CheckAttrList();
        }
    }
}

void ScXMLExportDataPilot::WriteLevels(ScDPSaveDimension* pDim)
{
    rtl::OUStringBuffer sBuffer;
    SvXMLUnitConverter::convertBool(sBuffer, pDim->GetShowEmpty());
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY_EMPTY, sBuffer.makeStringAndClear());
    SvXMLElementExport aElemDPL(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_LEVEL, sal_True, sal_True);
    WriteSortInfo(pDim);
    WriteAutoShowInfo(pDim);
    WriteLayoutInfo(pDim);
    rExport.CheckAttrList();

    WriteSubTotals(pDim);
    WriteMembers(pDim);
}

void ScXMLExportDataPilot::WriteDatePart(sal_Int32 nPart)
{
    switch(nPart)
    {
    case SC_DP_DATE_SECONDS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUP_PART, XML_SECONDS);
        }
        break;
    case SC_DP_DATE_MINUTES :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUP_PART, XML_MINUTES);
        }
        break;
    case SC_DP_DATE_HOURS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUP_PART, XML_HOURS);
        }
        break;
    case SC_DP_DATE_DAYS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUP_PART, XML_DAYS);
        }
        break;
    case SC_DP_DATE_MONTHS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUP_PART, XML_MONTHS);
        }
        break;
    case SC_DP_DATE_QUARTERS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUP_PART, XML_QUARTERS);
        }
        break;
    case SC_DP_DATE_YEARS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUP_PART, XML_YEARS);
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
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUP_SOURCE_FIELD_NAME, pGroupDim->GetSourceDimName());
        if (pGroupDim->GetDatePart())
        {
            WriteDatePart(pGroupDim->GetDatePart());
            WriteNumGroupInfo(pGroupDim->GetDateInfo());
        }
    }
}

void ScXMLExportDataPilot::WriteGroupDimElements(const ScDPSaveGroupDimension* pGroupDim)
{
    if (pGroupDim)
    {
        if (!pGroupDim->GetDatePart())
        {
            SvXMLElementExport aElemDPGs(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_GROUPS, sal_True, sal_True);
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
                            SvXMLElementExport aElemDPGs(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_MEMBER, sal_True, sal_True);
                        }
                    }
                }
            }
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

void ScXMLExportDataPilot::WriteDimension(ScDPSaveDimension* pDim, const ScDPDimensionSaveData* pDimData)
{
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_SOURCE_FIELD_NAME, rtl::OUString(pDim->GetName()));
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
    const ScDPSaveGroupDimension* pGroupDim = NULL;
    const ScDPSaveNumGroupDimension* pNumGroupDim = NULL;
    if (pDimData)
    {
        pGroupDim = pDimData->GetNamedGroupDim(pDim->GetName());
        WriteGroupDimAttributes(pGroupDim);
        pNumGroupDim = pDimData->GetNumGroupDim(pDim->GetName());
        WriteNumGroupDim(pNumGroupDim);
    }
    SvXMLElementExport aElemDPF(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_FIELD, sal_True, sal_True);
    WriteFieldReference(pDim);
    WriteLevels(pDim);
    WriteGroupDimElements(pGroupDim);
}

void ScXMLExportDataPilot::WriteDimensions(ScDPSaveData* pDPSave)
{
    List aDimensions = pDPSave->GetDimensions();
    sal_Int32 nDimCount = aDimensions.Count();
    for (sal_Int32 nDim = 0; nDim < nDimCount; nDim++)
    {
        WriteDimension((ScDPSaveDimension*)aDimensions.GetObject(nDim), pDPSave->GetExistingDimensionData());
    }
}

void ScXMLExportDataPilot::WriteDataPilots(const uno::Reference <sheet::XSpreadsheetDocument>& xSpreadDoc)
{
    pDoc = rExport.GetDocument();
    if (pDoc)
    {
        ScDPCollection* pDPs = pDoc->GetDPCollection();
        if (pDPs)
        {
            sal_Int16 nDPCount = pDPs->GetCount();
            if (nDPCount > 0)
            {
                SvXMLElementExport aElemDPs(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_TABLES, sal_True, sal_True);
                rExport.CheckAttrList();
                for (sal_Int16 i = 0; i < nDPCount; i++)
                {
                    ScDPSaveData* pDPSave = (*pDPs)[i]->GetSaveData();
                    if (pDPSave)
                    {
                        ScRange aOutRange = (*pDPs)[i]->GetOutRange();
                        rtl::OUString sTargetRangeAddress;
                        ScXMLConverter::GetStringFromRange( sTargetRangeAddress, aOutRange, pDoc );
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
                                for (SCROW nButtonRow = nRow1; nButtonRow <= nRow2; nButtonRow++)
                                {
                                    ScAddress aButtonAddr(nCol, nButtonRow, aOutRange.aStart.Tab());
                                    ScXMLConverter::GetStringFromAddress(
                                        sOUButtonList, aButtonAddr, pDoc, sal_True );
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
                        SvXMLElementExport aElemDP(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_TABLE, sal_True, sal_True);
                        rExport.CheckAttrList();
                        if ((*pDPs)[i]->IsSheetData())
                        {
                            const ScSheetSourceDesc* pSheetSource = (*pDPs)[i]->GetSheetDesc();
                            rtl::OUString sCellRangeAddress;
                            ScXMLConverter::GetStringFromRange( sCellRangeAddress, pSheetSource->aSourceRange, pDoc );
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS, sCellRangeAddress);
                            SvXMLElementExport aElemSCR(rExport, XML_NAMESPACE_TABLE, XML_SOURCE_CELL_RANGE, sal_True, sal_True);
                            rExport.CheckAttrList();
                            WriteDPFilter(pSheetSource->aQueryParam);
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
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_USERNAME, rtl::OUString(pServSource->aParUser));
                            // How to write the Passwort? We must know, whether the passwort shoulb be written encrypted and how or not
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_PASSWORT, rtl::OUString(pServSource->aParPass));
                            SvXMLElementExport aElemSD(rExport, XML_NAMESPACE_TABLE, XML_SOURCE_SERVICE, sal_True, sal_True);
                            rExport.CheckAttrList();
                        }
                        WriteDimensions(pDPSave);
                    }
                }
            }
        }
    }
}
