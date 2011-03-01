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



// INCLUDE --------------------------------------------------------------

#include <tools/debug.hxx>
#include <svl/zforlist.hxx>

#include "dpshttab.hxx"
#include "dptabres.hxx"
#include "document.hxx"
#include "collect.hxx"
#include "cell.hxx"
#include "dpcachetable.hxx"
#include "dpobject.hxx"
#include "globstr.hrc"
#include "dpglobal.hxx"
#include "rangenam.hxx"

#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>

#include <vector>
#include <set>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::std::vector;

// -----------------------------------------------------------------------

ScSheetDPData::ScSheetDPData(ScDocument* pD, const ScSheetSourceDesc& rDesc) :
    ScDPTableData(pD),
    aQuery ( rDesc.GetQueryParam() ),
    pSpecial(NULL),
    bIgnoreEmptyRows( FALSE ),
    bRepeatIfEmpty(FALSE),
    mrDesc(rDesc),
    aCacheTable(rDesc.CreateCache())
{
    SCSIZE nEntryCount( aQuery.GetEntryCount());
    pSpecial = new bool[nEntryCount];
    for (SCSIZE j = 0; j < nEntryCount; ++j )
    {
        ScQueryEntry& rEntry = aQuery.GetEntry(j);
        if (rEntry.bDoQuery)
        {
           pSpecial[j] = false;
            if (!rEntry.bQueryByString)
            {
                if (*rEntry.pStr == EMPTY_STRING &&
                   ((rEntry.nVal == SC_EMPTYFIELDS) || (rEntry.nVal == SC_NONEMPTYFIELDS)))
                    pSpecial[j] = true;
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
     delete[] pSpecial;
}

void ScSheetDPData::DisposeData()
{
    aCacheTable.clear();
}

long ScSheetDPData::GetColumnCount()
{
    CreateCacheTable();
    return aCacheTable.getColSize();
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
    else if (nColumn >= aCacheTable.getColSize())
    {
        DBG_ERROR("getDimensionName: invalid dimension");
        return String();
    }
    else
    {
        return  aCacheTable.getFieldName((SCCOL)nColumn);
    }
}

BOOL ScSheetDPData::IsDateDimension(long nDim)
{
    CreateCacheTable();
    long nColCount = aCacheTable.getColSize();
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
        return GetCacheTable().getCache()->IsDateDimension( nDim);
    }
}

ULONG ScSheetDPData::GetNumberFormat(long nDim)
{
    CreateCacheTable();
    if (getIsDataLayoutDimension(nDim))
    {
        return 0;
    }
    else if (nDim >= GetCacheTable().getColSize())
    {
        DBG_ERROR("GetNumberFormat: invalid dimension");
        return 0;
    }
    else
    {
        return GetCacheTable().getCache()->GetNumberFormat( nDim );
    }
}
UINT32  ScDPTableData::GetNumberFormatByIdx( NfIndexTableOffset eIdx )
{
    if( !mpDoc )
        return 0;

    if ( SvNumberFormatter* pFormatter = mpDoc->GetFormatTable() )
        return pFormatter->GetFormatIndex( eIdx, LANGUAGE_SYSTEM );

    return 0;
}

BOOL ScSheetDPData::getIsDataLayoutDimension(long nColumn)
{
    CreateCacheTable();
    return (nColumn ==(long)( aCacheTable.getColSize()));
}

void ScSheetDPData::SetEmptyFlags( BOOL bIgnoreEmptyRowsP, BOOL bRepeatIfEmptyP )
{
    bIgnoreEmptyRows = bIgnoreEmptyRowsP;
    bRepeatIfEmpty   = bRepeatIfEmptyP;
}

bool ScSheetDPData::IsRepeatIfEmpty()
{
    return bRepeatIfEmpty;
}

void ScSheetDPData::CreateCacheTable()
{
    // Scan and store the data from the source range.
    if (!aCacheTable.empty())
        // already cached.
        return;

    if (!aCacheTable.hasCache())
        aCacheTable.setCache(mrDesc.CreateCache());

    aCacheTable.fillTable(aQuery, pSpecial, bIgnoreEmptyRows, bRepeatIfEmpty);
}

void ScSheetDPData::FilterCacheTable(const vector<ScDPCacheTable::Criterion>& rCriteria, const boost::unordered_set<sal_Int32>& rCatDims)
{
    CreateCacheTable();
    aCacheTable.filterByPageDimension(
        rCriteria, (IsRepeatIfEmpty() ? rCatDims : boost::unordered_set<sal_Int32>()));
}

void ScSheetDPData::GetDrillDownData(const vector<ScDPCacheTable::Criterion>& rCriteria, const boost::unordered_set<sal_Int32>& rCatDims, Sequence< Sequence<Any> >& rData)
{
    CreateCacheTable();
    sal_Int32 nRowSize = aCacheTable.getRowSize();
    if (!nRowSize)
        return;

    aCacheTable.filterTable(
        rCriteria, rData, IsRepeatIfEmpty() ? rCatDims : boost::unordered_set<sal_Int32>());
}

void ScSheetDPData::CalcResults(CalcInfo& rInfo, bool bAutoShow)
{
    CreateCacheTable();
    CalcResultsFromCacheTable(aCacheTable, rInfo, bAutoShow);
}

const ScDPCacheTable& ScSheetDPData::GetCacheTable() const
{
    return aCacheTable;
}

ScSheetSourceDesc::ScSheetSourceDesc(ScDocument* pDoc) :
    mpDoc(pDoc) {}

void ScSheetSourceDesc::SetSourceRange(const ScRange& rRange)
{
    maSourceRange = rRange;
    maRangeName = OUString(); // overwrite existing range name if any.
}

const ScRange& ScSheetSourceDesc::GetSourceRange() const
{
    if (maRangeName.getLength())
    {
        // Obtain the source range from the range name first.
        maSourceRange = ScRange();
        ScRangeName* pRangeName = mpDoc->GetRangeName();
        do
        {
            if (!pRangeName)
                break;

            OUString aUpper = ScGlobal::pCharClass->upper(maRangeName);
            USHORT n;
            if (!pRangeName->SearchNameUpper(aUpper, n))
                break;

            // range name found.  Fow now, we only use the first token and
            // ignore the rest.
            ScRangeData* pData = (*pRangeName)[n];
            ScRange aRange;
            if (!pData->IsReference(aRange))
                break;

            maSourceRange = aRange;
        }
        while (false);
    }
    return maSourceRange;
}

void ScSheetSourceDesc::SetRangeName(const OUString& rName)
{
    maRangeName = rName;
}

const OUString& ScSheetSourceDesc::GetRangeName() const
{
    return maRangeName;
}

bool ScSheetSourceDesc::HasRangeName() const
{
    return maRangeName.getLength() > 0;
}

void ScSheetSourceDesc::SetQueryParam(const ScQueryParam& rParam)
{
    maQueryParam = rParam;
}

const ScQueryParam& ScSheetSourceDesc::GetQueryParam() const
{
    return maQueryParam;
}

bool ScSheetSourceDesc::operator== (const ScSheetSourceDesc& rOther) const
{
    return maSourceRange == rOther.maSourceRange &&
        maRangeName == rOther.maRangeName &&
        maQueryParam  == rOther.maQueryParam;
}

ScDPCache* ScSheetSourceDesc::CreateCache() const
{
    if (!mpDoc)
        return NULL;

    ULONG nErrId = CheckSourceRange();
    if (nErrId)
    {
        DBG_ERROR( "Error Create Cache\n" );
        return NULL;
    }

    ScDPCache* pCache = new ScDPCache(mpDoc);
    pCache->InitFromDoc(mpDoc, GetSourceRange());
    return pCache;
}

long ScSheetSourceDesc::GetCacheId() const
{
    return -1;
}

ULONG ScSheetSourceDesc::CheckSourceRange() const
{
    if (!mpDoc)
        return STR_ERR_DATAPILOTSOURCE;

    const ScRange& aSrcRange = GetSourceRange();
    const ScAddress& s = aSrcRange.aStart;
    const ScAddress& e = aSrcRange.aEnd;
    for (SCCOL nCol = aSrcRange.aStart.Col(); nCol <= e.Col(); ++nCol)
    {
        if (mpDoc->IsBlockEmpty(s.Tab(), nCol, s.Row(), nCol, s.Row()))
            return STR_PIVOT_FIRSTROWEMPTYERR;
    }

    if (mpDoc->IsBlockEmpty(s.Tab(), s.Col(), s.Row()+1, e.Col(), e.Row()))
        return STR_PIVOT_ONLYONEROWERR;

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
