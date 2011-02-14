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

#include <stdio.h>
#include <rtl/math.hxx>
#include <tools/debug.hxx>
#include <tools/date.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <unotools/collatorwrapper.hxx>

#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>

#include "dptabdat.hxx"
#include "global.hxx"
#include "dpcachetable.hxx"
#include "dptabres.hxx"
#include "document.hxx"
#include "dpobject.hxx"

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

ScDPTableData::ScDPTableData(ScDocument* pDoc, long nCacheId ) :
    mnCacheId( nCacheId ),
    mpDoc ( pDoc )
{
    nLastDateVal = nLastHier = nLastLevel = nLastRet = -1;      // invalid

    //! reset before new calculation (in case the base date is changed)
}

ScDPTableData::~ScDPTableData()
{
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
                    DBG_ERROR("GetDatePart: wrong level");
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
                    DBG_ERROR("GetDatePart: wrong level");
            }
            break;
        default:
            DBG_ERROR("GetDatePart: wrong hierarchy");
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
    return sal_False;       // always false
}

long ScDPTableData::GetGroupBase(long) const
{
    return -1;          // always none
}

sal_Bool ScDPTableData::IsNumOrDateGroup(long) const
{
    return sal_False;       // always false
}

sal_Bool ScDPTableData::IsInGroup( const ScDPItemData&, long,
                               const ScDPItemData&, long ) const
{
    DBG_ERROR("IsInGroup shouldn't be called for non-group data");
    return sal_False;
}

sal_Bool ScDPTableData::HasCommonElement( const ScDPItemData&, long,
                                      const ScDPItemData&, long ) const
{
    DBG_ERROR("HasCommonElement shouldn't be called for non-group data");
    return sal_False;
}
void ScDPTableData::FillRowDataFromCacheTable(sal_Int32 nRow, const ScDPCacheTable& rCacheTable,
                                        const CalcInfo& rInfo, CalcRowData& rData)
{
    // column dimensions
    GetItemData(rCacheTable, nRow, rInfo.aColLevelDims, rData.aColData);

    // row dimensions
    GetItemData(rCacheTable, nRow, rInfo.aRowLevelDims, rData.aRowData);

    // page dimensions
    GetItemData(rCacheTable, nRow, rInfo.aPageDims, rData.aPageData);

    long nCacheColumnCount = rCacheTable.GetCache()->GetColumnCount();
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
        // Wang Xu Ming -- 2009-6-16
        // DataPilot Migration
    if (!bAutoShow)
    {
            LateInitParams  aColParams( rInfo.aColDims, rInfo.aColLevels, sal_False );
            LateInitParams  aRowParams ( rInfo.aRowDims, rInfo.aRowLevels, sal_True );
            // root always init child
            aColParams.SetInitChild( sal_True );
            aColParams.SetInitAllChildren( sal_False);
            aRowParams.SetInitChild( sal_True );
            aRowParams.SetInitAllChildren( sal_False);

            rInfo.pColRoot->LateInitFrom( aColParams, rData.aColData,0, *rInfo.pInitState);
            rInfo.pRowRoot->LateInitFrom( aRowParams, rData.aRowData, 0, *rInfo.pInitState);
    }
        // End Comments

    if ( ( !rInfo.pColRoot->GetChildDimension() || rInfo.pColRoot->GetChildDimension()->IsValidEntry(rData.aColData) ) &&
         ( !rInfo.pRowRoot->GetChildDimension() || rInfo.pRowRoot->GetChildDimension()->IsValidEntry(rData.aRowData) ) )
    {
        //! single process method with ColMembers, RowMembers and data !!!
        if (rInfo.pColRoot->GetChildDimension())
        {
// Wang Xu Ming -- 2009-6-10
// DataPilot Migration
            vector</*ScDPItemData*/ SCROW > aEmptyData;
            rInfo.pColRoot->GetChildDimension()->ProcessData(rData.aColData, NULL, aEmptyData, rData.aValues);
// End Comments
        }

        rInfo.pRowRoot->ProcessData(rData.aRowData, rInfo.pColRoot->GetChildDimension(),
                                    rData.aColData, rData.aValues);
    }
}

void ScDPTableData::CalcResultsFromCacheTable(const ScDPCacheTable& rCacheTable, CalcInfo& rInfo, bool bAutoShow)
{
    sal_Int32 nRowSize = rCacheTable.getRowSize();
    for (sal_Int32 nRow = 0; nRow < nRowSize; ++nRow)
    {
        if (!rCacheTable.isRowActive(nRow))
            continue;

        CalcRowData aData;
        FillRowDataFromCacheTable(nRow, rCacheTable, rInfo, aData);
        ProcessRowData(rInfo, aData, bAutoShow);
    }
}

// Wang Xu Ming -- 2009-6-10
// DataPilot Migration
void ScDPTableData::GetItemData(const ScDPCacheTable& rCacheTable, sal_Int32 nRow,
                                const vector<long>& rDims, vector< SCROW/*ScDPItemData*/>& rItemData)
// End Comments
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
        if ( nDim >= rCacheTable.GetCache()->GetColumnCount() )
           continue;

        SCROW nId= rCacheTable.GetCache()->GetItemDataId( static_cast<SCCOL>(nDim), static_cast<SCROW>(nRow), IsRepeatIfEmpty());
        rItemData.push_back( nId );

    }
}

// -----------------------------------------------------------------------

// Wang Xu Ming -- 2009-6-8
// DataPilot Migration
long ScDPTableData::GetMembersCount( long nDim )
{
    if ( nDim > MAXCOL )
        return 0;
    return GetCacheTable().getFieldEntries( nDim ).size();
}

long ScDPTableData::GetCacheId() const
{
    return mnCacheId;
}

const ScDPItemData* ScDPTableData::GetMemberByIndex( long nDim, long nIndex )
{
    if ( nIndex >= GetMembersCount( nDim ) )
        return NULL;

    const ::std::vector<SCROW>& nMembers = GetCacheTable().getFieldEntries( nDim );

    return GetCacheTable().GetCache()->GetItemDataById( (SCCOL) nDim, (SCROW)nMembers[nIndex] );
}

const ScDPItemData* ScDPTableData::GetMemberById( long nDim, long nId)
{

    return GetCacheTable().GetCache()->GetItemDataById( (SCCOL) nDim, (SCROW)nId);
}

SCROW   ScDPTableData::GetIdOfItemData( long  nDim, const ScDPItemData& rData )
{
        return GetCacheTable().GetCache()->GetIdByItemData((SCCOL) nDim, rData );
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

    long n1 = GetCacheTable().GetCache()->GetOrder( nDim, nDataId1);
    long n2 = GetCacheTable().GetCache()->GetOrder( nDim, nDataId2);
    if ( n1 > n2 )
        return 1;
    else if ( n1 == n2 )
        return 0;
    else
        return -1;
}
// End Comments
// -----------------------------------------------------------------------
