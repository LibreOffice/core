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

#include "dptabdat.hxx"

#include "global.hxx"
#include "dpcachetable.hxx"
#include "dptabres.hxx"
#include "document.hxx"
#include "dpobject.hxx"

#include <stdio.h>
#include <rtl/math.hxx>
#include <tools/date.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <unotools/collatorwrapper.hxx>

#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::std::vector;

// ---------------------------------------------------------------------------

ScDPTableData::CalcInfo::CalcInfo() :
    bRepeatIfEmpty(false)
{
}

// ---------------------------------------------------------------------------

ScDPTableData::ScDPTableData(ScDocument* pDoc) :
    mpDoc(pDoc)
{
    nLastDateVal = nLastHier = nLastLevel = nLastRet = -1;      // invalid

    //! reset before new calculation (in case the base date is changed)
}

ScDPTableData::~ScDPTableData()
{
}

rtl::OUString ScDPTableData::GetFormattedString(long nDim, const ScDPItemData& rItem) const
{
    const ScDPCache* pCache = GetCacheTable().getCache();
    return pCache->GetFormattedString(nDim, rItem);
}

long ScDPTableData::GetDatePart( long nDateVal, long nHierarchy, long nLevel )
{
    if ( nDateVal == nLastDateVal && nHierarchy == nLastHier && nLevel == nLastLevel )
        return nLastRet;

    Date aDate( 30,12,1899 );                   //! get from source data (and cache here)
    aDate += nDateVal;

    long nRet = 0;
    switch (nHierarchy)
    {
        case SC_DAPI_HIERARCHY_QUARTER:
            switch (nLevel)
            {
                case 0: nRet = aDate.GetYear();                 break;
                case 1: nRet = (aDate.GetMonth()-1) / 3 + 1;    break;
                case 2: nRet = aDate.GetMonth();                break;
                case 3: nRet = aDate.GetDay();                  break;
                default:
                    OSL_FAIL("GetDatePart: wrong level");
            }
            break;
        case SC_DAPI_HIERARCHY_WEEK:
            switch (nLevel)
            {
                //! use settings for different definitions
                case 0: nRet = aDate.GetYear();                 break;      //!...
                case 1: nRet = aDate.GetWeekOfYear();           break;
                case 2: nRet = (long)aDate.GetDayOfWeek();      break;
                default:
                    OSL_FAIL("GetDatePart: wrong level");
            }
            break;
        default:
            OSL_FAIL("GetDatePart: wrong hierarchy");
    }

    nLastDateVal = nDateVal;
    nLastHier    = nHierarchy;
    nLastLevel   = nLevel;
    nLastRet     = nRet;

    return nRet;
}

bool ScDPTableData::IsRepeatIfEmpty()
{
    return false;
}

sal_uLong ScDPTableData::GetNumberFormat(long)
{
    return 0;           // default format
}

sal_Bool ScDPTableData::IsBaseForGroup(long) const
{
    return false;       // always false
}

long ScDPTableData::GetGroupBase(long) const
{
    return -1;          // always none
}

sal_Bool ScDPTableData::IsNumOrDateGroup(long) const
{
    return false;       // always false
}

sal_Bool ScDPTableData::IsInGroup( const ScDPItemData&, long,
                               const ScDPItemData&, long ) const
{
    OSL_FAIL("IsInGroup shouldn't be called for non-group data");
    return false;
}

sal_Bool ScDPTableData::HasCommonElement( const ScDPItemData&, long,
                                      const ScDPItemData&, long ) const
{
    OSL_FAIL("HasCommonElement shouldn't be called for non-group data");
    return false;
}
void ScDPTableData::FillRowDataFromCacheTable(sal_Int32 nRow, const ScDPFilteredCache& rCacheTable,
                                        const CalcInfo& rInfo, CalcRowData& rData)
{
    // column dimensions
    GetItemData(rCacheTable, nRow, rInfo.aColLevelDims, rData.aColData);

    // row dimensions
    GetItemData(rCacheTable, nRow, rInfo.aRowLevelDims, rData.aRowData);

    // page dimensions
    GetItemData(rCacheTable, nRow, rInfo.aPageDims, rData.aPageData);

    long nCacheColumnCount = rCacheTable.getCache()->GetColumnCount();
    sal_Int32 n = rInfo.aDataSrcCols.size();
    for (sal_Int32 i = 0; i < n; ++i)
    {
        long nDim = rInfo.aDataSrcCols[i];
        rData.aValues.push_back( ScDPValueData() );
        // #i111435# GetItemData needs dimension indexes including groups,
        // so the index must be checked here (groups aren't useful as data fields).
        if ( nDim < nCacheColumnCount )
        {
            ScDPValueData& rVal = rData.aValues.back();
            rCacheTable.getValue( rVal, static_cast<SCCOL>(nDim), static_cast<SCROW>(nRow), false);
        }
    }
}

void ScDPTableData::ProcessRowData(CalcInfo& rInfo, CalcRowData& rData, bool bAutoShow)
{
    if (!bAutoShow)
    {
            LateInitParams  aColParams(rInfo.aColDims, rInfo.aColLevels, false);
            LateInitParams  aRowParams(rInfo.aRowDims, rInfo.aRowLevels, true);
            // root always init child
            aColParams.SetInitChild(true);
            aColParams.SetInitAllChildren( false);
            aRowParams.SetInitChild(true);
            aRowParams.SetInitAllChildren( false);

            rInfo.pColRoot->LateInitFrom(aColParams, rData.aColData, 0, *rInfo.pInitState);
            rInfo.pRowRoot->LateInitFrom(aRowParams, rData.aRowData, 0, *rInfo.pInitState);
    }

    if ( ( !rInfo.pColRoot->GetChildDimension() || rInfo.pColRoot->GetChildDimension()->IsValidEntry(rData.aColData) ) &&
         ( !rInfo.pRowRoot->GetChildDimension() || rInfo.pRowRoot->GetChildDimension()->IsValidEntry(rData.aRowData) ) )
    {
        //! single process method with ColMembers, RowMembers and data !!!
        if (rInfo.pColRoot->GetChildDimension())
        {
            vector<SCROW> aEmptyData;
            rInfo.pColRoot->GetChildDimension()->ProcessData(rData.aColData, NULL, aEmptyData, rData.aValues);
        }

        rInfo.pRowRoot->ProcessData(rData.aRowData, rInfo.pColRoot->GetChildDimension(),
                                    rData.aColData, rData.aValues);
    }
}

void ScDPTableData::CalcResultsFromCacheTable(const ScDPFilteredCache& rCacheTable, CalcInfo& rInfo, bool bAutoShow)
{
    sal_Int32 nRowSize = rCacheTable.getRowSize();
    for (sal_Int32 nRow = 0; nRow < nRowSize; ++nRow)
    {
        sal_Int32 nLastRow;
        if (!rCacheTable.isRowActive(nRow, &nLastRow))
        {
            nRow = nLastRow;
            continue;
        }

        CalcRowData aData;
        FillRowDataFromCacheTable(nRow, rCacheTable, rInfo, aData);
        ProcessRowData(rInfo, aData, bAutoShow);
    }
}

void ScDPTableData::GetItemData(const ScDPFilteredCache& rCacheTable, sal_Int32 nRow,
                                const vector<long>& rDims, vector<SCROW>& rItemData)
{
    sal_Int32 nDimSize = rDims.size();
    for (sal_Int32 i = 0; i < nDimSize; ++i)
    {
        long nDim = rDims[i];

        if (getIsDataLayoutDimension(nDim))
        {
            rItemData.push_back( -1 );
            continue;
        }

        nDim = GetSourceDim( nDim );
        if ( nDim >= rCacheTable.getCache()->GetColumnCount() )
           continue;

        SCROW nId= rCacheTable.getCache()->GetItemDataId( static_cast<SCCOL>(nDim), static_cast<SCROW>(nRow), IsRepeatIfEmpty());
        rItemData.push_back( nId );
    }
}

// -----------------------------------------------------------------------

long ScDPTableData::GetMembersCount( long nDim )
{
    if ( nDim > MAXCOL )
        return 0;
    return GetCacheTable().getFieldEntries( nDim ).size();
}

const ScDPItemData* ScDPTableData::GetMemberByIndex( long nDim, long nIndex )
{
    if ( nIndex >= GetMembersCount( nDim ) )
        return NULL;

    const ::std::vector<SCROW>& nMembers = GetCacheTable().getFieldEntries( nDim );

    return GetCacheTable().getCache()->GetItemDataById( (SCCOL) nDim, (SCROW)nMembers[nIndex] );
}

const ScDPItemData* ScDPTableData::GetMemberById( long nDim, long nId)
{

    return GetCacheTable().getCache()->GetItemDataById(nDim, static_cast<SCROW>(nId));
}

SCROW   ScDPTableData::GetIdOfItemData( long  nDim, const ScDPItemData& rData )
{
        return GetCacheTable().getCache()->GetIdByItemData((SCCOL) nDim, rData );
 }

const std::vector< SCROW >& ScDPTableData::GetColumnEntries( long nColumn )
{
    return GetCacheTable().getFieldEntries( nColumn );
}

long ScDPTableData::GetSourceDim( long nDim )
{
    return nDim;

}

long ScDPTableData::Compare( long nDim, long nDataId1, long nDataId2)
{
    if ( getIsDataLayoutDimension(nDim) )
        return 0;

    long n1 = GetCacheTable().getOrder(nDim, nDataId1);
    long n2 = GetCacheTable().getOrder(nDim, nDataId2);
    if ( n1 > n2 )
        return 1;
    else if ( n1 == n2 )
        return 0;
    else
        return -1;
}

#if DEBUG_PIVOT_TABLE
void ScDPTableData::Dump() const
{
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
