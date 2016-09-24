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

#include "tabcol.hxx"
#include <limits.h>

SwTabCols::SwTabCols( sal_uInt16 nSize ) :
    nLeftMin( 0 ),
    nLeft( 0 ),
    nRight( 0 ),
    nRightMax( 0 ),
    bLastRowAllowedToChange( true )
{
    if ( nSize )
        aData.reserve( nSize );
}

SwTabCols::SwTabCols( const SwTabCols& rCpy ) :
    nLeftMin( rCpy.GetLeftMin() ),
    nLeft( rCpy.GetLeft() ),
    nRight( rCpy.GetRight() ),
    nRightMax( rCpy.GetRightMax() ),
    bLastRowAllowedToChange( rCpy.IsLastRowAllowedToChange() ),
    aData( rCpy.GetData() )
{
}

SwTabCols &SwTabCols::operator=( const SwTabCols& rCpy )
{
    nLeftMin = rCpy.GetLeftMin();
    nLeft    = rCpy.GetLeft();
    nRight   = rCpy.GetRight();
    nRightMax= rCpy.GetRightMax();
    bLastRowAllowedToChange = rCpy.IsLastRowAllowedToChange();

    Remove( 0, Count() );
    aData = rCpy.GetData();

    return *this;
}

void SwTabCols::Insert( long nValue, long nMin, long nMax,
                        bool bValue, size_t nPos )
{
    SwTabColsEntry aEntry;
    aEntry.nPos = nValue;
    aEntry.nMin = nMin;
    aEntry.nMax = nMax;
    aEntry.bHidden = bValue;
    aData.insert( aData.begin() + nPos, aEntry );
}

void SwTabCols::Insert( long nValue, bool bValue, size_t nPos )
{
    SwTabColsEntry aEntry;
    aEntry.nPos = nValue;
    aEntry.nMin = 0;
    aEntry.nMax = LONG_MAX;
    aEntry.bHidden = bValue;
    aData.insert( aData.begin() + nPos, aEntry );

#if OSL_DEBUG_LEVEL > 1
    SwTabColsEntries::iterator aPos = aData.begin();
    for ( ; aPos != aData.end(); ++aPos )
    {
        aEntry =(*aPos);
    }
#endif
}

void SwTabCols::Remove( size_t nPos, size_t nCount )
{
    SwTabColsEntries::iterator aStart = aData.begin() + nPos;
    aData.erase( aStart, aStart + nCount );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
