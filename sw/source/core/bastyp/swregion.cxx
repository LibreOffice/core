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

#include <swrect.hxx>
#include <swregion.hxx>
#include <swtypes.hxx>

SwRegionRects::SwRegionRects( const SwRect &rStartRect, sal_uInt16 nInit ) :
    m_aOrigin( rStartRect )
{
    reserve(nInit);
    push_back( m_aOrigin );
}

SwRegionRects::SwRegionRects( sal_uInt16 nInit ) :
    m_aOrigin()
{
    reserve(nInit);
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
    push_back( rRect );
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
        if ( rRect.Overlaps( (*this)[i] ) )
        {
            SwRect aTmp( (*this)[i] );
            SwRect aInter( aTmp );
            aInter.Intersection_( rRect );

            // The first Rect that should be inserted takes position of i.
            // This avoids one Delete() call.
            bool bDel = true;

            // now split; only those rectangles should be left over that are in
            // the "old" but not in the "new" area; hence, not in intersection.
            tools::Long nTmp = aInter.Top() - aTmp.Top();
            if ( 0 < nTmp )
            {
                const tools::Long nOldVal = aTmp.Height();
                aTmp.Height(nTmp);
                InsertRect( aTmp, i, bDel );
                aTmp.Height( nOldVal );
            }

            aTmp.Top( aInter.Top() + aInter.Height() );
            if ( aTmp.Height() > 0 )
                InsertRect( aTmp, i, bDel );

            aTmp.Top( aInter.Top() );
            aTmp.Bottom( aInter.Bottom() );
            nTmp = aInter.Left() - aTmp.Left();
            if ( 0 < nTmp )
            {
                const tools::Long nOldVal = aTmp.Width();
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

static inline SwTwips CalcArea( const SwRect &rRect )
{
    return rRect.Width() * rRect.Height();
}

void SwRegionRects::LimitToOrigin()
{
    for (size_type i = 0; i < size(); ++i )
        (*this)[ i ].Intersection( m_aOrigin );
}

// combine all adjacent rectangles
void SwRegionRects::Compress( CompressType type )
{
    bool bAgain;
    do
    {
        sort( begin(), end(), []( const SwRect& l, const SwRect& r ) { return l.Top() < r.Top(); } );
        bAgain = false;
        bool bRemoved = false;
        for (size_type i = 0; i < size(); ++i )
        {
            if( (*this)[i].IsEmpty())
                continue;
            // Rectangles are sorted by Y axis, so check only pairs of rectangles
            // that are possibly overlapping or adjacent or close enough to be grouped by the fuzzy
            // code below.
            const tools::Long nFuzzy = type == CompressFuzzy ? 1361513 : 0;
            const tools::Long yMax = (*this)[i].Top() + (*this)[i].Height() + nFuzzy
                / std::max<tools::Long>( 1, (*this)[i].Width());
            for(size_type j = i+1; j < size(); ++j)
            {
                if( (*this)[j].IsEmpty())
                    continue;
                if( (*this)[j].Top() > yMax )
                    break;
                // If one rectangle contains a second completely than the latter
                // does not need to be stored and can be deleted
                else if ( (*this)[i].Contains( (*this)[j] ) )
                {
                    (*this)[j].Width(0); // = erase(), see below
                    bRemoved = true;
                }
                else if ( (*this)[j].Contains( (*this)[i] ) )
                {
                    (*this)[i] = (*this)[j];
                    (*this)[j].Width(0);
                    bRemoved = true;
                    bAgain = true;
                }
                else
                {
                    // Merge adjacent rectangles (possibly overlapping), such rectangles can be
                    // detected by their merged areas being equal to the area of the union
                    // (which is obviously the case if they share one side, and using
                    // the nFuzzy extra allows merging also rectangles that do not quite cover
                    // the entire union but it's close enough).

                    // For combining as much as possible (and for having less single
                    // paints), the area of the union can be a little bit larger:
                    // ( 9622 * 141.5 = 1361513 ~= a quarter (1/4) centimeter wider
                    // than the width of an A4 page
                    SwRect aUnion( (*this)[i] );
                    aUnion.Union( (*this)[j] );
                    SwRect aInter( (*this)[i] );
                    aInter.Intersection( (*this)[j] );
                    if ( CalcArea( (*this)[i] ) + CalcArea( (*this)[j] ) - CalcArea( aInter )
                            + nFuzzy >= CalcArea( aUnion ) )
                    {
                        (*this)[i] = aUnion;
                        (*this)[j].Width(0);
                        bRemoved = true;
                        bAgain = true;
                    }
                }
            }
        }
        // Instead of repeated erase() we Width(0) the elements, and now erase
        // all empty elements just once.
        if( bRemoved )
            resize( std::remove_if(begin(), end(), [](const SwRect& rect) { return rect.IsEmpty(); }) - begin());
        // Code paths setting bAgain alter elements of the vector, possibly breaking
        // the Y-axis optimization, so run another pass just to make sure. The adjacent-rects
        // merging code may possibly benefit from a repeated pass also if two pairs of merged
        // rects might get merged again and this pass skipped that.
    } while(bAgain);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
