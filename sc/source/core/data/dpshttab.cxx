/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dpshttab.cxx,v $
 * $Revision: 1.12 $
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



// INCLUDE --------------------------------------------------------------

#include <tools/debug.hxx>
#include <svtools/zforlist.hxx>

#include "dpshttab.hxx"
#include "dptabres.hxx"
#include "document.hxx"
#include "collect.hxx"
#include "cell.hxx"
#include "dpcachetable.hxx"
#include "dpobject.hxx"
#include "globstr.hrc"

#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>

#include <vector>
#include <set>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Sequence;
using ::std::vector;
using ::std::hash_map;
using ::std::hash_set;

// -----------------------------------------------------------------------

class ScSheetDPData_Impl
{
public:
    ScDocument*     pDoc;
    ScRange         aRange;
    ScQueryParam    aQuery;
    BOOL*           pSpecial;           // to flag special handling of query parameters in ValidQuery.
    BOOL            bIgnoreEmptyRows;
    BOOL            bRepeatIfEmpty;
    BOOL*           pDateDim;
    SCROW           nNextRow;       // for iterator, within range

    ScDPCacheTable  aCacheTable;

    ScSheetDPData_Impl(ScDPCollection* p) :
        pSpecial(NULL),
        aCacheTable(p)
    {
    }
};

// -----------------------------------------------------------------------

ScSheetDPData::ScSheetDPData( ScDocument* pD, const ScSheetSourceDesc& rDesc ) :
    ScDPTableData(pD)
{
    pImpl = new ScSheetDPData_Impl(pD->GetDPCollection());
    pImpl->pDoc = pD;
    pImpl->aRange = rDesc.aSourceRange;
    pImpl->aQuery = rDesc.aQueryParam;
    pImpl->bIgnoreEmptyRows = FALSE;
    pImpl->bRepeatIfEmpty = FALSE;
    pImpl->pDateDim = NULL;

    pImpl->nNextRow = pImpl->aRange.aStart.Row() + 1;

    SCSIZE nEntryCount(pImpl->aQuery.GetEntryCount());
    pImpl->pSpecial = new BOOL[nEntryCount];
    for (SCSIZE j = 0; j < nEntryCount; ++j )
    {
        ScQueryEntry& rEntry = pImpl->aQuery.GetEntry(j);
        if (rEntry.bDoQuery)
        {
            pImpl->pSpecial[j] = false;
            if (!rEntry.bQueryByString)
            {
                if (*rEntry.pStr == EMPTY_STRING &&
                   ((rEntry.nVal == SC_EMPTYFIELDS) || (rEntry.nVal == SC_NONEMPTYFIELDS)))
                    pImpl->pSpecial[j] = true;
            }
            else
            {
                sal_uInt32 nIndex = 0;
                rEntry.bQueryByString =
                            !(pD->GetFormatTable()->
                                IsNumberFormat(*rEntry.pStr, nIndex, rEntry.nVal));
            }
        }
    }
}

ScSheetDPData::~ScSheetDPData()
{
    delete[] pImpl->pDateDim;
    delete[] pImpl->pSpecial;
    delete pImpl;
}

void ScSheetDPData::DisposeData()
{
    pImpl->aCacheTable.clear();
}

long ScSheetDPData::GetColumnCount()
{
    CreateCacheTable();
    return pImpl->aCacheTable.getColSize();
}

BOOL lcl_HasQuery( const ScQueryParam& rParam )
{
    return rParam.GetEntryCount() > 0 &&
            rParam.GetEntry(0).bDoQuery;
}

const TypedScStrCollection& ScSheetDPData::GetColumnEntries(long nColumn)
{
    DBG_ASSERT(nColumn>=0 && nColumn < pImpl->aCacheTable.getColSize(), "ScSheetDPData: wrong column");
    CreateCacheTable();
    return pImpl->aCacheTable.getFieldEntries(nColumn);
}

String ScSheetDPData::getDimensionName(long nColumn)
{
    CreateCacheTable();
    if (getIsDataLayoutDimension(nColumn))
    {
        //! different internal and display names?
        //return "Data";
        return ScGlobal::GetRscString(STR_PIVOT_DATA);
    }
    else if (nColumn >= pImpl->aCacheTable.getColSize())
    {
        DBG_ERROR("getDimensionName: invalid dimension");
        return String();
    }
    else
    {
        const String* pStr = pImpl->aCacheTable.getFieldName(nColumn);
        if (pStr)
            return *pStr;
        else return String();
    }
}

BOOL lcl_HasDateFormat( ScDocument* pDoc, const ScRange& rRange )
{
    //! iterate formats in range?

    ScAddress aPos = rRange.aStart;
    aPos.SetRow( aPos.Row() + 1 );      // below title
    ULONG nFormat = pDoc->GetNumberFormat( aPos );
    SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
    return ( pFormatter->GetType(nFormat) & NUMBERFORMAT_DATE ) != 0;
}

BOOL ScSheetDPData::IsDateDimension(long nDim)
{
    CreateCacheTable();
    long nColCount = pImpl->aCacheTable.getColSize();
    if (getIsDataLayoutDimension(nDim))
    {
        return FALSE;
    }
    else if (nDim >= nColCount)
    {
        DBG_ERROR("IsDateDimension: invalid dimension");
        return FALSE;
    }
    else
    {
        if (!pImpl->pDateDim)
        {
            pImpl->pDateDim = new BOOL[nColCount];
            ScRange aTestRange = pImpl->aRange;
            for (long i = 0; i < nColCount; ++i)
            {
                SCCOL nCol = (SCCOL)( pImpl->aRange.aStart.Col() + i );
                aTestRange.aStart.SetCol(nCol);
                aTestRange.aEnd.SetCol(nCol);
                pImpl->pDateDim[i] = lcl_HasDateFormat( pImpl->pDoc, aTestRange );
            }
        }
        return pImpl->pDateDim[nDim];
    }
}

UINT32 ScSheetDPData::GetNumberFormat(long nDim)
{
    CreateCacheTable();
    if (getIsDataLayoutDimension(nDim))
    {
        return 0;
    }
    else if (nDim >= pImpl->aCacheTable.getColSize())
    {
        DBG_ERROR("GetNumberFormat: invalid dimension");
        return 0;
    }
    else
    {
        //  is queried only once per dimension from ScDPOutput -> no need to cache

        ScAddress aPos = pImpl->aRange.aStart;
        aPos.SetCol( sal::static_int_cast<SCCOL>( aPos.Col() + nDim ) );
        aPos.SetRow( aPos.Row() + 1 );      // below title
        return pImpl->pDoc->GetNumberFormat( aPos );
    }
}

BOOL ScSheetDPData::getIsDataLayoutDimension(long nColumn)
{
    CreateCacheTable();
    return (nColumn == pImpl->aCacheTable.getColSize());
}

void ScSheetDPData::SetEmptyFlags( BOOL bIgnoreEmptyRows, BOOL bRepeatIfEmpty )
{
    pImpl->bIgnoreEmptyRows = bIgnoreEmptyRows;
    pImpl->bRepeatIfEmpty   = bRepeatIfEmpty;
}

bool ScSheetDPData::IsRepeatIfEmpty()
{
    return pImpl->bRepeatIfEmpty;
}

void ScSheetDPData::CreateCacheTable()
{
    // Scan and store the data from the source range.
    if (!pImpl->aCacheTable.empty())
        // already cached.
        return;

    pImpl->aCacheTable.fillTable(pImpl->pDoc, pImpl->aRange, pImpl->aQuery, pImpl->pSpecial,
                                 pImpl->bIgnoreEmptyRows);
}

void ScSheetDPData::FilterCacheTable(const vector<ScDPCacheTable::Criterion>& rCriteria, const hash_set<sal_Int32>& rCatDims)
{
    CreateCacheTable();
    pImpl->aCacheTable.filterByPageDimension(
        rCriteria, (IsRepeatIfEmpty() ? rCatDims : hash_set<sal_Int32>()));
}

void ScSheetDPData::GetDrillDownData(const vector<ScDPCacheTable::Criterion>& rCriteria, const hash_set<sal_Int32>& rCatDims, Sequence< Sequence<Any> >& rData)
{
    CreateCacheTable();
    sal_Int32 nRowSize = pImpl->aCacheTable.getRowSize();
    if (!nRowSize)
        return;

    pImpl->aCacheTable.filterTable(
        rCriteria, rData, IsRepeatIfEmpty() ? rCatDims : hash_set<sal_Int32>());
}

void ScSheetDPData::CalcResults(CalcInfo& rInfo, bool bAutoShow)
{
    CreateCacheTable();
    CalcResultsFromCacheTable(pImpl->aCacheTable, rInfo, bAutoShow);
}

const ScDPCacheTable& ScSheetDPData::GetCacheTable() const
{
    return pImpl->aCacheTable;
}

// -----------------------------------------------------------------------







