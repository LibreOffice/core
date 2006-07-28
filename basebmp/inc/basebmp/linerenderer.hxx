/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: linerenderer.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: thb $ $Date: 2006-07-28 12:43:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_BASEBMP_LINERENDERER_HXX
#define INCLUDED_BASEBMP_LINERENDERER_HXX

#ifndef _BGFX_POINT_B2IPOINT_HXX
#include <basegfx/point/b2ipoint.hxx>
#endif

#include <vigra/iteratortraits.hxx>


/* Scan-converting lines */

namespace basebmp
{

/** Render line with Bresenham

    This function renders the line given by rPt1 and rPt2 using the
    Bresenham algorithm with the specified color value. Make sure rPt1
    and rPt1 are valid coordinates in the image given by begin and
    end, since no clipping takes place.

    @param aPt1
    Start point of the line

    @param aPt2
    End point of the line

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
void renderLine( const basegfx::B2IPoint&      rPt1,
                 const basegfx::B2IPoint&      rPt2,
                 typename Accessor::value_type color,
                 Iterator                      begin,
                 Accessor                      acc,
                 bool                          bRoundTowardsPt2=false )
{
    // code inspired by Paul Heckbert's Digital Line Drawing
    // (Graphics Gems, Academic Press 1990)
    const sal_Int32 x1 = rPt1.getX();
    const sal_Int32 x2 = rPt2.getX();
    const sal_Int32 y1 = rPt1.getY();
    const sal_Int32 y2 = rPt2.getY();

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

    // TODO(P3): handle horizontal and vertical lines specially
    sal_Int32 xs = x1;
    sal_Int32 ys = y1;
    if( adx >= ady )
    {
        // semi-horizontal line
        sal_Int32 rem = 2*ady - adx - !bRoundTowardsPt2;
        adx *= 2;
        ady *= 2;

        Iterator currIter( begin + vigra::Diff2D(0,ys) );
        typename vigra::IteratorTraits<Iterator>::row_iterator
            rowIter( currIter.rowIterator() + xs );
        while(true)
        {
            acc.set(color, rowIter);

            if( xs == x2 )
                return;

            if( rem >= 0 )
            {
                ys += sy;
                xs += sx;
                currIter.y += sy;
                rowIter = currIter.rowIterator() + xs;
                rem -= adx;
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
        // semi-vertical line
        sal_Int32 rem = 2*adx - ady - !bRoundTowardsPt2;
        adx *= 2;
        ady *= 2;

        Iterator currIter( begin + vigra::Diff2D(xs,0) );
        typename vigra::IteratorTraits<Iterator>::column_iterator
            colIter( currIter.columnIterator() + ys );
        while(true)
        {
            acc.set(color, colIter);

            if( ys == y2 )
                return;

            if( rem >= 0 )
            {
                xs += sx;
                ys += sy;
                currIter.x += sx;
                colIter = currIter.columnIterator() + ys;
                rem -= ady;
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

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_LINERENDERER_HXX */
