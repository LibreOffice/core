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
#include "precompiled_sw.hxx"



#include "tabcol.hxx"


SwTabCols::SwTabCols( USHORT nSize ) :
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
#if OSL_DEBUG_LEVEL > 1
    for ( USHORT i = 0; i < Count(); ++i )
    {
        SwTabColsEntry aEntry1 = aData[i];
        SwTabColsEntry aEntry2 = rCpy.GetData()[i];
        (void) aEntry1;
        (void) aEntry2;
        OSL_ENSURE( aEntry1.nPos == aEntry2.nPos &&
                aEntry1.nMin == aEntry2.nMin &&
                aEntry1.nMax == aEntry2.nMax &&
                aEntry1.bHidden == aEntry2.bHidden,
                "CopyContructor of SwTabColsEntries did not succeed!" );
    }
#endif
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

BOOL SwTabCols::operator==( const SwTabCols& rCmp ) const
{
    USHORT i;

    if ( !(nLeftMin == rCmp.GetLeftMin() &&
           nLeft    == rCmp.GetLeft()    &&
           nRight   == rCmp.GetRight()   &&
           nRightMax== rCmp.GetRightMax()&&
           bLastRowAllowedToChange== rCmp.IsLastRowAllowedToChange() &&
           Count()== rCmp.Count()) )
        return FALSE;

    for ( i = 0; i < Count(); ++i )
    {
        SwTabColsEntry aEntry1 = aData[i];
        SwTabColsEntry aEntry2 = rCmp.GetData()[i];
        if ( aEntry1.nPos != aEntry2.nPos || aEntry1.bHidden != aEntry2.bHidden  )
            return FALSE;
    }

    return TRUE;
}

void SwTabCols::Insert( long nValue, long nMin, long nMax, BOOL bValue, USHORT nPos )
{
    SwTabColsEntry aEntry;
    aEntry.nPos = nValue;
    aEntry.nMin = nMin;
    aEntry.nMax = nMax;
    aEntry.bHidden = bValue;
    aData.insert( aData.begin() + nPos, aEntry );
}

void SwTabCols::Insert( long nValue, BOOL bValue, USHORT nPos )
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

void SwTabCols::Remove( USHORT nPos, USHORT nAnz )
{
    SwTabColsEntries::iterator aStart = aData.begin() + nPos;
    aData.erase( aStart, aStart + nAnz );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
