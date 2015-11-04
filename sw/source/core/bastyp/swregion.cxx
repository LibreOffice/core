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

#include "swrect.hxx"
#include "swregion.hxx"
#include "swtypes.hxx"

SwRegionRects::SwRegionRects( const SwRect &rStartRect, sal_uInt16 nInit ) :
    SwRects(),
    m_aOrigin( rStartRect )
{
    reserve(nInit);
    push_back( m_aOrigin );
}

// If <rDel> is true then this Rect will be overwritten by <rRect> at
// position <nPos>. Otherwise <rRect> is attached at the end.
inline void SwRegionRects::InsertRect( const SwRect &rRect,
                                       const sal_uInt16 nPos, bool &rDel )
{
    if( rDel )
    {
        (*this)[nPos] = rRect;
        rDel = false;
    }
    else
    {
        push_back( rRect );
    }
}

void SwRegionRects::operator+=( const SwRect &rRect )
{
    bool f = false;
    InsertRect( rRect, 0, f );
}

/** Delete all overlaps of the Rects in array with the given <rRect>

    To do so, all existing rectangles have to be either split or deleted.

    @param rRect rectangle with the area that should be deleted
*/
void SwRegionRects::operator-=( const SwRect &rRect )
{
    sal_uInt16 nMax = size();
    for ( sal_uInt16 i = 0; i < nMax; ++i )
    {
        if ( rRect.IsOver( (*this)[i] ) )
        {
            SwRect aTmp( (*this)[i] );
            SwRect aInter( aTmp );
            aInter._Intersection( rRect );

            // The first Rect that should be inserted takes position of i.
            // This avoids one Delete() call.
            bool bDel = true;

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
                erase( begin() + i );
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
    SwRegionRects aInvRegion( m_aOrigin, size()*2+2 );
    for( const_iterator it = begin(); it != end(); ++it )
        aInvRegion -= *it;

    // overwrite all existing
    swap( aInvRegion );
}

inline SwTwips CalcArea( const SwRect &rRect )
{
    return rRect.Width() * rRect.Height();
}

// combine all adjacent rectangles
void SwRegionRects::Compress( bool bFuzzy )
{
    for (size_type i = 0; i < size(); )
    {
        bool bRestart(false);
        for ( size_type j = i+1; j < size(); ++j )
        {
            // If one rectangle contains a second completely than the latter
            // does not need to be stored and can be deleted
            if ( (*this)[i].IsInside( (*this)[j] ) )
            {
                erase( begin() + j );
                --j;
            }
            else if ( (*this)[j].IsInside( (*this)[i] ) )
            {
                (*this)[i] = (*this)[j];
                erase( begin() + j );
                bRestart = true;
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
                SwRect aUnion( (*this)[i] );
                aUnion.Union( (*this)[j] );
                SwRect aInter( (*this)[i] );
                aInter.Intersection( (*this)[j] );
                if ( (::CalcArea( (*this)[i] ) +
                      ::CalcArea( (*this)[j] ) + nFuzzy) >=
                     (::CalcArea( aUnion ) - CalcArea( aInter )) )
                {
                    (*this)[i] = aUnion;
                    erase( begin() + j );
                    bRestart = true;
                    break;
                }
            }
        }
        i = (bRestart) ? 0 : i+1;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
