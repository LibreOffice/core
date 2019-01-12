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

#include <tabcol.hxx>
#include <limits.h>

SwTabCols::SwTabCols( sal_uInt16 nSize ) :
    m_nLeftMin( 0 ),
    m_nLeft( 0 ),
    m_nRight( 0 ),
    m_nRightMax( 0 ),
    m_bLastRowAllowedToChange( true )
{
    if ( nSize )
        m_aData.reserve( nSize );
}

SwTabCols::SwTabCols( const SwTabCols& rCpy ) :
    m_nLeftMin( rCpy.GetLeftMin() ),
    m_nLeft( rCpy.GetLeft() ),
    m_nRight( rCpy.GetRight() ),
    m_nRightMax( rCpy.GetRightMax() ),
    m_bLastRowAllowedToChange( rCpy.IsLastRowAllowedToChange() ),
    m_aData( rCpy.GetData() )
{
}

SwTabCols &SwTabCols::operator=( const SwTabCols& rCpy )
{
    m_nLeftMin = rCpy.GetLeftMin();
    m_nLeft    = rCpy.GetLeft();
    m_nRight   = rCpy.GetRight();
    m_nRightMax= rCpy.GetRightMax();
    m_bLastRowAllowedToChange = rCpy.IsLastRowAllowedToChange();

    Remove( 0, Count() );
    m_aData = rCpy.GetData();

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
    m_aData.insert( m_aData.begin() + nPos, aEntry );
}

void SwTabCols::Insert( long nValue, bool bValue, size_t nPos )
{
    SwTabColsEntry aEntry;
    aEntry.nPos = nValue;
    aEntry.nMin = 0;
    aEntry.nMax = LONG_MAX;
    aEntry.bHidden = bValue;
    m_aData.insert( m_aData.begin() + nPos, aEntry );

#if OSL_DEBUG_LEVEL > 1
    for ( const auto& rPos : m_aData )
    {
        aEntry = rPos;
    }
#endif
}

void SwTabCols::Remove( size_t nPos, size_t nCount )
{
    SwTabColsEntries::iterator aStart = m_aData.begin() + nPos;
    m_aData.erase( aStart, aStart + nCount );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
