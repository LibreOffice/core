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

#include "csvsplits.hxx"

#include <algorithm>

bool ScCsvSplits::Insert( sal_Int32 nPos )
{
    if (nPos < 0)
        return false;

    const iterator aIter = ::std::lower_bound( maVec.begin(), maVec.end(), nPos );

    if (aIter != maVec.end() && *aIter == nPos)
        return false;

    maVec.insert( aIter, nPos );
    return true;
}

bool ScCsvSplits::Remove( sal_Int32 nPos )
{
    sal_uInt32 nIndex = GetIndex( nPos );
    if (nIndex == CSV_VEC_NOTFOUND)
        return false;

    maVec.erase( maVec.begin() + nIndex );
    return true;
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

sal_uInt32 ScCsvSplits::GetIterIndex( const_iterator aIter ) const
{
    return (aIter == maVec.end()) ? CSV_VEC_NOTFOUND : (aIter - maVec.begin());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
