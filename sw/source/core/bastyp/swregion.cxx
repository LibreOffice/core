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

#include "swtypes.hxx"
#include "swrect.hxx"
#include "swregion.hxx"

SV_IMPL_VARARR( SwRects, SwRect );

SwRegionRects::SwRegionRects( const SwRect &rStartRect, sal_uInt16 nInit ) :
    SwRects( (sal_uInt8)nInit ),
    aOrigin( rStartRect )
{
    Insert( aOrigin, 0 );
}

// If <rDel> is sal_True then this Rect will be overwritten by <rRect> at
// position <nPos>. Otherwise <rRect> is attached at the end.
inline void SwRegionRects::InsertRect( const SwRect &rRect,
                                       const sal_uInt16 nPos, sal_Bool &rDel )
{
    if( rDel )
    {
        pData = (SwRect*)pData; // looks weird but seems to help gcc ->i78417
        *(pData+nPos) = rRect;
        rDel = sal_False;
    }
    else
    {
        Insert( rRect, Count() );
    }
}

/** Delete all overlaps of the Rects in array with the given <rRect>

    To do so, all existing rectangles have to be either split or deleted.

    @param rRect rectangle with the area that should be deleted
*/
void SwRegionRects::operator-=( const SwRect &rRect )
{
    sal_uInt16 nMax = Count();
    for ( sal_uInt16 i = 0; i < nMax; ++i )
    {
        if ( rRect.IsOver( *(pData+i) ) )
        {
            SwRect aTmp( *(pData+i) );
            SwRect aInter( aTmp );
            aInter._Intersection( rRect );

            // The first Rect that should be inserted takes position of i.
            // This avoids one Delete() call.
            sal_Bool bDel = sal_True;

            // now split; only those rectangles should be left over that are in
            // the "old" but not in the "new" area; hence, not in intersection.
            long nTmp;
            if ( 0 < (nTmp = aInter.Top() - aTmp.Top()) )
            {
                const long nOldVal = aTmp.Height();
                aTmp.Height(nTmp);
                InsertRect( aTmp, i, bDel );
                aTmp.Height( nOldVal );
            }

            aTmp.Top( aInter.Top() + aInter.Height() );
            if ( aTmp.Height() > 0 )
                InsertRect( aTmp, i, bDel );

            aTmp.Top( aInter.Top() );
            aTmp.Bottom( aInter.Bottom() );
            if ( 0 < (nTmp = aInter.Left() - aTmp.Left()) )
            {
                const long nOldVal = aTmp.Width();
                aTmp.Width( nTmp );
                InsertRect( aTmp, i, bDel );
                aTmp.Width( nOldVal );
            }

            aTmp.Left( aInter.Left() + aInter.Width() ); //+1?
            if ( aTmp.Width() > 0 )
                InsertRect( aTmp, i, bDel );

            if( bDel )
            {
                Remove( i );
                --i;     // so that we don't forget any
                --nMax;  // so that we don't check too much
            }
        }
    }
}

/** invert current rectangle

    Change the shape, such that holes with be areas and areas are holes now.

    Note: If no rects were removed, then the shape is identical to the original
          shape. As a result, it will be a NULL-SRectangle after inverting.
*/
void SwRegionRects::Invert()
{
    // not very elegant and fast, but efficient:
    // Create a new region and remove all areas that are left over. Afterwards
    // copy all values.

    // To avoid unnecessary memory requirements, create a "useful" initial size:
    // Number of rectangles in this area * 2 + 2 for the special case of a
    // single hole (so four Rects in the inverse case).
    SwRegionRects aInvRegion( aOrigin, Count()*2+2 );
    const SwRect *pDat = GetData();
    for( sal_uInt16 i = 0; i < Count(); ++pDat, ++i )
        aInvRegion -= *pDat;

    sal_uInt16 nCpy = Count(), nDel = 0;
    if( aInvRegion.Count() < Count() )
    {
        nDel = Count() - aInvRegion.Count();
        nCpy = aInvRegion.Count();
    }
    // overwrite all existing
    memcpy( pData, aInvRegion.GetData(), nCpy * sizeof( SwRect ));

    if( nCpy < aInvRegion.Count() )
        Insert( &aInvRegion, nCpy, nCpy );
    else if( nDel )
        Remove( nCpy, nDel );
}

inline SwTwips CalcArea( const SwRect &rRect )
{
    return rRect.Width() * rRect.Height();
}

// combine all adjacent rectangles
void SwRegionRects::Compress( sal_Bool bFuzzy )
{
    for ( int i = 0; i < Count(); ++i )
    {
        for ( int j = i+1; j < Count(); ++j )
        {
            // If one rectangle contains a second completely than the latter
            // does not need to be stored and can be deleted
            if ( (*(pData + i)).IsInside( *(pData + j) ) )
            {
                Remove( static_cast<sal_uInt16>(j), 1 );
                --j;
            }
            else if ( (*(pData + j)).IsInside( *(pData + i) ) )
            {
                *(pData + i) = *(pData + j);
                Remove( static_cast<sal_uInt16>(j), 1 );
                i = -1;
                break;
            }
            else
            {
                // If two rectangles have the same area of their union minus the
                // intersection then one of them can be deleted.
                // For combining as much as possible (and for having less single
                // paints), the area of the union can be a little bit larger:
                // ( 9622 * 141.5 = 1361513 ~= a quarter (1/4) centimeter wider
                // than the width of a A4 page
                const long nFuzzy = bFuzzy ? 1361513 : 0;
                SwRect aUnion( *(pData + i) );
                aUnion.Union( *(pData + j) );
                SwRect aInter( *(pData + i) );
                aInter.Intersection( *(pData + j));
                if ( (::CalcArea( *(pData + i) ) +
                      ::CalcArea( *(pData + j) ) + nFuzzy) >=
                     (::CalcArea( aUnion ) - CalcArea( aInter )) )
                {
                    *(pData + i) = aUnion;
                    Remove( static_cast<sal_uInt16>(j), 1 );
                    i = -1;
                    break;
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
