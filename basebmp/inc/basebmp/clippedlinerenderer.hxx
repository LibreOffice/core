/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: clippedlinerenderer.hxx,v $
 * $Revision: 1.10 $
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

#ifndef INCLUDED_BASEBMP_CLIPPEDLINERENDERER_HXX
#define INCLUDED_BASEBMP_CLIPPEDLINERENDERER_HXX

#include <basegfx/tools/rectcliptools.hxx>
#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/range/b2irange.hxx>

#include <vigra/diff2d.hxx>
#include <vigra/iteratortraits.hxx>

namespace basebmp
{

// factored-out bresenham setup code, which is used from two different
// places in renderClippedLine() below. Admittedly messy for the long
// parameter list...
inline bool prepareClip( sal_Int32  a1,
                         sal_Int32  a2,
                         sal_Int32  b1,
                         sal_Int32  da,
                         sal_Int32  db,
                         sal_Int32& o_as,
                         sal_Int32& o_bs,
                         int        sa,
                         int        sb,
                         sal_Int32& io_rem,
                         int&       o_n,
                         sal_uInt32 clipCode1,
                         sal_uInt32 clipCount1,
                         sal_uInt32 clipCode2,
                         sal_uInt32 clipCount2,
                         sal_Int32  aMin,
                         sal_uInt32 aMinFlag,
                         sal_Int32  aMax,
                         sal_uInt32 aMaxFlag,
                         sal_Int32  bMin,
                         sal_uInt32 bMinFlag,
                         sal_Int32  bMax,
                         sal_uInt32 bMaxFlag,
                         bool       bRoundTowardsPt2 )
{
    int ca(0), cb(0);
    if( clipCode1 )
    {
        if( clipCode1 & aMinFlag )
        {
            ca = 2*db*(aMin - a1);
            o_as = aMin;
        }
        else if( clipCode1 & aMaxFlag )
        {
            ca = 2*db*(a1 - aMax);
            o_as = aMax;
        }

        if( clipCode1 & bMinFlag )
        {
            cb = 2*da*(bMin - b1);
            o_bs = bMin;
        }
        else if( clipCode1 & bMaxFlag )
        {
            cb = 2*da*(b1 - bMax);
            o_bs = bMax;
        }

        if( clipCount1 == 2 )
            clipCode1 &= (ca + da < cb + !bRoundTowardsPt2) ? ~(aMinFlag|aMaxFlag) : ~(bMinFlag|bMaxFlag);

        if( clipCode1 & (aMinFlag|aMaxFlag) )
        {
            cb = (ca + da - !bRoundTowardsPt2) / (2*da);

            if( sb >= 0 )
            {
                o_bs = b1 + cb;
                if( o_bs > bMax )
                    return false;
            }
            else
            {
                o_bs = b1 - cb;
                if( o_bs < bMin )
                    return false;
            }

            io_rem += ca - 2*da*cb;
        }
        else
        {
            ca = (cb - da + 2*db - bRoundTowardsPt2) / (2*db);
            if( sa >= 0 )
            {
                o_as = a1 + ca;
                if( o_as > aMax )
                    return false;
            }
            else
            {
                o_as = a1 - ca;
                if( o_as < aMin )
                    return false;
            }

            io_rem += 2*db*ca - cb;
        }
    }
    else
    {
        o_as = a1; o_bs = b1;
    }

    bool bRetVal = false;
    if( clipCode2 )
    {
        if( clipCount2 == 2 )
        {
            ca = 2*db*((clipCode2 & aMinFlag) ? a1 - aMin : aMax - a1);
            cb = 2*da*((clipCode2 & bMinFlag) ? b1 - bMin : bMax - b1);
            clipCode2 &= (cb + da < ca + bRoundTowardsPt2) ? ~(aMinFlag|aMaxFlag) : ~(bMinFlag|bMaxFlag);
        }

        if( clipCode2 & (aMinFlag|aMaxFlag) )
            o_n = (clipCode2 & aMinFlag) ? o_as - aMin : aMax - o_as;
        else
        {
            o_n = (clipCode2 & bMinFlag) ? o_bs - bMin : bMax - o_bs;
            bRetVal = true;
        }
    }
    else
        o_n = (a2 >= o_as) ? a2 - o_as : o_as - a2;

    return bRetVal;
}


/** Render line to image iterators, clip against given rectangle

    This method renders a line from aPt1 to aPt2, clipped against
    rClipRect (the clipping will take place pixel-perfect, i.e. as if
    the original bresenham-rendered line would have been clipped each
    pixel individually. No slight shifts compared to unclipped lines).

    @param aPt1
    Start point of the line

    @param aPt2
    End point of the line

    @param rClipRect
    Rectangle to clip against

    @param color
    Color value to render the line with

    @param begin
    left-top image iterator

    @param end
    right-bottom image iterator

    @param acc
    Image accessor

    @param bRoundTowardsPt2
    Rounding mode to use. Giving false here results in line pixel tend
    towards pt1, i.e. when a pixel exactly hits the middle between two
    pixel, the pixel closer to pt1 will be chosen. Giving true here
    makes renderClippedLine() choose pt2 in those cases.
 */
template< class Iterator, class Accessor >
void renderClippedLine( basegfx::B2IPoint             aPt1,
                        basegfx::B2IPoint             aPt2,
                        const basegfx::B2IRange&      rClipRect,
                        typename Accessor::value_type color,
                        Iterator                      begin,
                        Accessor                      acc,
                        bool                          bRoundTowardsPt2=false )
{
    // Algorithm according to Steven Eker's 'Pixel-perfect line clipping',
    // Graphics Gems V, pp. 314-322
    sal_uInt32 clipCode1 = basegfx::tools::getCohenSutherlandClipFlags(aPt1,
                                                                       rClipRect);
    sal_uInt32 clipCode2 = basegfx::tools::getCohenSutherlandClipFlags(aPt2,
                                                                       rClipRect);

    if( clipCode1 & clipCode2 )
        return; // line fully clipped away

    sal_uInt32 clipCount1 = basegfx::tools::getNumberOfClipPlanes(clipCode1);
    sal_uInt32 clipCount2 = basegfx::tools::getNumberOfClipPlanes(clipCode2);

    if( (clipCode1 != 0 && clipCode2 == 0)
        || (clipCount1 == 2 && clipCount2 == 1) )
    {
        std::swap(clipCount2,clipCount1);
        std::swap(clipCode2,clipCode1);
        std::swap(aPt1,aPt2);
        bRoundTowardsPt2 = !bRoundTowardsPt2;
    }

    const sal_Int32 x1 = aPt1.getX();
    const sal_Int32 x2 = aPt2.getX();
    const sal_Int32 y1 = aPt1.getY();
    const sal_Int32 y2 = aPt2.getY();

    // TODO(E1): This might overflow
    sal_Int32 adx = x2 - x1;
    int sx = 1;
    if( adx < 0 )
    {
        adx *= -1;
        sx = -1;
    }

    // TODO(E1): This might overflow
    sal_Int32 ady = y2 - y1;
    int sy = 1;
    if( ady < 0 )
    {
        ady *= -1;
        sy = -1;
    }

    int n  = 0;
    sal_Int32 xs = x1;
    sal_Int32 ys = y1;
    if( adx >= ady )
    {
        // semi-horizontal line
        sal_Int32 rem = 2*ady - adx - !bRoundTowardsPt2;

        const bool bUseAlternateBresenham(
            prepareClip(x1, x2, y1, adx, ady, xs, ys, sx, sy,
                        rem, n, clipCode1, clipCount1, clipCode2, clipCount2,
                        rClipRect.getMinX(), basegfx::tools::RectClipFlags::LEFT,
                        rClipRect.getMaxX(), basegfx::tools::RectClipFlags::RIGHT,
                        rClipRect.getMinY(), basegfx::tools::RectClipFlags::TOP,
                        rClipRect.getMaxY(), basegfx::tools::RectClipFlags::BOTTOM,
                        bRoundTowardsPt2 ));

        Iterator currIter( begin + vigra::Diff2D(0,ys) );
        typename vigra::IteratorTraits<Iterator>::row_iterator
            rowIter( currIter.rowIterator() + xs );

        adx *= 2;
        ady *= 2;

        if( bUseAlternateBresenham )
        {
            while(true)
            {
                acc.set(color, rowIter);

                if( rem >= 0 )
                {
                    if( --n < 0 )
                        break;

                    ys += sy;
                    xs += sx;
                    rem -= adx;

                    currIter.y += sy;
                    rowIter = currIter.rowIterator() + xs;
                }
                else
                {
                    xs += sx;
                    rowIter += sx;
                }

                rem += ady;
            }
        }
        else
        {
            while(true)
            {
                acc.set(color, rowIter);

                if( --n < 0 )
                    break;

                if( rem >= 0 )
                {
                    ys += sy;
                    xs += sx;
                    rem -= adx;

                    currIter.y += sy;
                    rowIter = currIter.rowIterator() + xs;
                }
                else
                {
                    xs += sx;
                    rowIter += sx;
                }

                rem += ady;
            }
        }
    }
    else
    {
        // semi-vertical line
        sal_Int32 rem = 2*adx - ady - !bRoundTowardsPt2;

        const bool bUseAlternateBresenham(
            prepareClip(y1, y2, x1, ady, adx, ys, xs, sy, sx,
                        rem, n, clipCode1, clipCount1, clipCode2, clipCount2,
                        rClipRect.getMinY(), basegfx::tools::RectClipFlags::TOP,
                        rClipRect.getMaxY(), basegfx::tools::RectClipFlags::BOTTOM,
                        rClipRect.getMinX(), basegfx::tools::RectClipFlags::LEFT,
                        rClipRect.getMaxX(), basegfx::tools::RectClipFlags::RIGHT,
                        bRoundTowardsPt2 ));

        Iterator currIter( begin + vigra::Diff2D(xs,0) );
        typename vigra::IteratorTraits<Iterator>::column_iterator
            colIter( currIter.columnIterator() + ys );

        adx *= 2;
        ady *= 2;

        if( bUseAlternateBresenham )
        {
            while(true)
            {
                acc.set(color, colIter);

                if( rem >= 0 )
                {
                    if( --n < 0 )
                        break;

                    xs += sx;
                    ys += sy;
                    rem -= ady;

                    currIter.x += sx;
                    colIter = currIter.columnIterator() + ys;
                }
                else
                {
                    ys += sy;
                    colIter += sy;
                }

                rem += adx;
            }
        }
        else
        {
            while(true)
            {
                acc.set(color, colIter);

                if( --n < 0 )
                    break;

                if( rem >= 0 )
                {
                    xs += sx;
                    ys += sy;
                    rem -= ady;

                    currIter.x += sx;
                    colIter = currIter.columnIterator() + ys;
                }
                else
                {
                    ys += sy;
                    colIter += sy;
                }

                rem += adx;
            }
        }
    }
}

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_CLIPPEDLINERENDERER_HXX */
