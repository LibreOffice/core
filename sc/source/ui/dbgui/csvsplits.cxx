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


// ============================================================================
#include "csvsplits.hxx"

#include <algorithm>


// ============================================================================

bool ScCsvSplits::Insert( sal_Int32 nPos )
{
    bool bValid = (nPos >= 0);
    if( bValid )
    {
        iterator aIter = ::std::lower_bound( maVec.begin(), maVec.end(), nPos );
        bValid = (aIter == maVec.end()) || (*aIter != nPos);
        if( bValid )
            aIter = maVec.insert( aIter, nPos );
    }
    return bValid;
}

bool ScCsvSplits::Remove( sal_Int32 nPos )
{
    sal_uInt32 nIndex = GetIndex( nPos );
    bool bValid = (nIndex != CSV_VEC_NOTFOUND);
    if( bValid )
        maVec.erase( maVec.begin() + nIndex );
    return bValid;
}

void ScCsvSplits::RemoveRange( sal_Int32 nPosStart, sal_Int32 nPosEnd )
{
    sal_uInt32 nStartIx = LowerBound( nPosStart );
    sal_uInt32 nEndIx = UpperBound( nPosEnd );
    if( (nStartIx != CSV_VEC_NOTFOUND) && (nEndIx != CSV_VEC_NOTFOUND) && (nStartIx <= nEndIx) )
        maVec.erase( maVec.begin() + nStartIx, maVec.begin() + nEndIx + 1 );
}

void ScCsvSplits::Clear()
{
    maVec.clear();
}

bool ScCsvSplits::HasSplit( sal_Int32 nPos ) const
{
    return GetIndex( nPos ) != CSV_VEC_NOTFOUND;
}


// ----------------------------------------------------------------------------

sal_uInt32 ScCsvSplits::GetIndex( sal_Int32 nPos ) const
{
    const_iterator aIter = ::std::lower_bound( maVec.begin(), maVec.end(), nPos );
    return GetIterIndex( ((aIter != maVec.end()) && (*aIter == nPos)) ? aIter : maVec.end() );
}

sal_uInt32 ScCsvSplits::LowerBound( sal_Int32 nPos ) const
{
    return GetIterIndex( ::std::lower_bound( maVec.begin(), maVec.end(), nPos ) );
}

sal_uInt32 ScCsvSplits::UpperBound( sal_Int32 nPos ) const
{
    sal_uInt32 nIndex = LowerBound( nPos );
    if( nIndex == CSV_VEC_NOTFOUND )
        return Count() ? (Count() - 1) : CSV_VEC_NOTFOUND;
    if( GetPos( nIndex ) == nPos )
        return nIndex;
    return nIndex ? (nIndex - 1) : CSV_VEC_NOTFOUND;
}

sal_Int32 ScCsvSplits::GetPos( sal_uInt32 nIndex ) const
{
    return (nIndex < Count()) ? maVec[ nIndex ] : CSV_POS_INVALID;
}


// ----------------------------------------------------------------------------

sal_uInt32 ScCsvSplits::GetIterIndex( const_iterator aIter ) const
{
    return (aIter == maVec.end()) ? CSV_VEC_NOTFOUND : (aIter - maVec.begin());
}


// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
