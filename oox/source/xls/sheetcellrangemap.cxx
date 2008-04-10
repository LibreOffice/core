/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sheetcellrangemap.cxx,v $
 * $Revision: 1.3 $
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

#include "oox/xls/sheetcellrangemap.hxx"

#define DEBUG_OOX_CELLRANGE_MAP 0

#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/CellAddress.hpp>

#if DEBUG_OOX_CELLRANGE_MAP
#include <stdio.h>
#endif

using ::com::sun::star::table::CellAddress;
using ::com::sun::star::table::CellRangeAddress;

namespace oox {
namespace xls {

SheetCellRangeMap::SheetCellRangeMap()
{
}

SheetCellRangeMap::~SheetCellRangeMap() throw()
{
}

void SheetCellRangeMap::addCellRange( const CellRangeAddress& aRangeAddr )
{
    size_t nAreaId = maAreas.size();

    // First, find the sheet ID.
    SheetMapType::iterator posSheet = maSheetMap.find(aRangeAddr.Sheet);
    if ( posSheet == maSheetMap.end() )
    {
        maSheetMap.insert( SheetMapType::value_type(aRangeAddr.Sheet, SheetSet()) );
        posSheet = maSheetMap.find(aRangeAddr.Sheet);
        OSL_ENSURE( posSheet != maSheetMap.end(), "SheetCellRangeMap::addCellRange: insertion failure" );
    }
    SheetSet& rSheet = posSheet->second;

    addRange(rSheet.maColRanges, aRangeAddr.StartColumn, aRangeAddr.EndColumn, nAreaId);
    addRange(rSheet.maRowRanges, aRangeAddr.StartRow, aRangeAddr.EndRow, nAreaId);

#if DEBUG_OOX_CELLRANGE_MAP
    fprintf(stdout, "SheetCellRangeMap::addCellRange: adding (sheet: %d) (col: %ld - %ld) (row: %ld - %ld) (area: %d)\n",
            aRangeAddr.Sheet, aRangeAddr.StartColumn, aRangeAddr.EndColumn, aRangeAddr.StartRow, aRangeAddr.EndRow, nAreaId);fflush(stdout);
#endif

    maAreas.push_back(aRangeAddr);
}

bool SheetCellRangeMap::isOverlapping( const CellAddress& aCellAddr ) const
{
    if ( maAreas.empty() )
        return false;

    SheetMapType::const_iterator pos = maSheetMap.find(aCellAddr.Sheet);
    if ( pos == maSheetMap.end() )
        // There is no cell range registered for this sheet.
        return false;

    const SheetSet& rSheet = pos->second;
    return searchColumns( rSheet, aCellAddr );
}

void SheetCellRangeMap::addRange( StartEndMapType& rRangeMap, sal_Int32 nStart, sal_Int32 nEnd, size_t nAreaId )
{
    StartEndMapType::iterator posStart = rRangeMap.find(nStart);
    if ( posStart == rRangeMap.end() )
    {
        EndAreaIdMapType aMap;
        rRangeMap.insert( StartEndMapType::value_type(nStart, aMap) );
        posStart = rRangeMap.find(nStart);
        OSL_ENSURE( posStart != rRangeMap.end(), "TableBuffer::addRangeToSet: insertion failure" );
    }
    EndAreaIdMapType& rEndMap = posStart->second;

    EndAreaIdMapType::iterator posEnd = rEndMap.find(nEnd);
    if ( posEnd == rEndMap.end() )
    {
        AreaIdSetType aSet;
        rEndMap.insert( EndAreaIdMapType::value_type(nEnd, aSet) );
        posEnd = rEndMap.find(nEnd);
        OSL_ENSURE( posEnd != rEndMap.end(), "TableBuffer::addRangeToSet: insertion failure" );
    }

    AreaIdSetType& rSet = posEnd->second;
    rSet.push_back(nAreaId);
}

bool SheetCellRangeMap::expandSearch( const EndAreaIdMapType& rEndMap, const CellAddress& rCellAddr, bool bColumn ) const
{
    sal_Int32 nId = bColumn ? rCellAddr.Column : rCellAddr.Row;

    EndAreaIdMapType::const_reverse_iterator itr, itrBeg = rEndMap.rbegin(), itrEnd = rEndMap.rend();
    for ( itr = itrBeg; itr != itrEnd; ++itr )
    {
        if ( itr->first >= nId )
        {
            // The point is in-range.
            const AreaIdSetType& rSet = itr->second;
            AreaIdSetType::const_iterator itr2 = rSet.begin(), itr2End = rSet.end();
            for ( ; itr2 != itr2End; ++itr2 )
            {
                OSL_ENSURE( maAreas.size() > *itr2, "SheetCellRangeMap::expandSearch: size mismatch" );
                const CellRangeAddress& rRange = maAreas[*itr2];
                if ( bColumn && rCellAddr.Row >= rRange.StartRow && rCellAddr.Row <= rRange.EndRow )
                    return true;
                if ( !bColumn && rCellAddr.Column >= rRange.StartColumn && rCellAddr.Column <= rRange.EndColumn )
                    return true;
            }
        }
        else if ( itr->first < nId )
            // No more enclosing ranges.
            return false;
    }
    return false;
}

bool SheetCellRangeMap::searchColumns( const SheetSet& rSheet, const CellAddress& aCellAddr ) const
{
    StartEndMapType::const_iterator itr, itrBeg = rSheet.maColRanges.begin(), itrEnd = rSheet.maColRanges.end();
    for ( itr = itrBeg; itr != itrEnd; ++itr )
    {
        if ( itr->first <= aCellAddr.Column )
        {
            if ( expandSearch(itr->second, aCellAddr, true) )
                return true;
        }
        else if ( itr->first > aCellAddr.Column )
            return false;
    }
    return false;
}

} // namespace xls
} // namespace oox

