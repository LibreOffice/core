/*************************************************************************
 *
 *  $RCSfile: bmpacc3.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_BMPACC_CXX

#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <bitmap.hxx>
#endif
#ifndef _SV_POLY_HXX
#include <poly.hxx>
#endif
#ifndef _SV_REGION_HXX
#include <region.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <bmpacc.hxx>
#endif

// ---------------------
// - BitmapWriteAccess -
// ---------------------

void BitmapWriteAccess::Erase( const Color& rColor )
{
    const BitmapColor   aOldFillColor( maFillColor );
    const Point         aPoint;
    const Rectangle     aRect( aPoint, maBitmap.GetSizePixel() );

    SetFillColor( rColor );
    FillRect( aRect );
    maFillColor = aOldFillColor;
}

// ------------------------------------------------------------------

void BitmapWriteAccess::DrawLine( const Point& rStart, const Point& rEnd )
{
    long    nX;
    long    nY;

    ImplInitDraw();

    if ( rStart.X() == rEnd.X() )
    {
        // vertikale Line
        const long nEndY = rEnd.Y();

        nX = rStart.X();
        nY = rStart.Y();

        if ( nEndY > nY )
        {
            for (; nY <= nEndY; nY++ )
                SetPixel( nY, nX, maLineColor );
        }
        else
        {
            for (; nY >= nEndY; nY-- )
                SetPixel( nY, nX, maLineColor );
        }
    }
    else if ( rStart.Y() == rEnd.Y() )
    {
        // horizontale Line
        const long nEndX = rEnd.X();

        nX = rStart.X();
        nY = rStart.Y();

        if ( nEndX > nX )
        {
            for (; nX <= nEndX; nX++ )
                SetPixel( nY, nX, maLineColor );
        }
        else
        {
            for (; nX >= nEndX; nX-- )
                SetPixel( nY, nX, maLineColor );
        }
    }
    else
    {
        const long  nDX = labs( rEnd.X() - rStart.X() );
        const long  nDY = labs( rEnd.Y() - rStart.Y() );
        long        nX1;
        long        nY1;
        long        nX2;
        long        nY2;

        if ( nDX >= nDY )
        {
            if ( rStart.X() < rEnd.X() )
            {
                nX1 = rStart.X();
                nY1 = rStart.Y();
                nX2 = rEnd.X();
                nY2 = rEnd.Y();
            }
            else
            {
                nX1 = rEnd.X();
                nY1 = rEnd.Y();
                nX2 = rStart.X();
                nY2 = rStart.Y();
            }

            const long  nDYX = ( nDY - nDX ) << 1;
            const long  nDY2 = nDY << 1;
            long        nD = nDY2 - nDX;
            BOOL        bPos = nY1 < nY2;

            for ( nX = nX1, nY = nY1; nX <= nX2; nX++ )
            {
                SetPixel( nY, nX, maLineColor );

                if ( nD < 0 )
                    nD += nDY2;
                else
                {
                    nD += nDYX;

                    if ( bPos )
                        nY++;
                    else
                        nY--;
                }
            }
        }
        else
        {
            if ( rStart.Y() < rEnd.Y() )
            {
                nX1 = rStart.X();
                nY1 = rStart.Y();
                nX2 = rEnd.X();
                nY2 = rEnd.Y();
            }
            else
            {
                nX1 = rEnd.X();
                nY1 = rEnd.Y();
                nX2 = rStart.X();
                nY2 = rStart.Y();
            }

            const long  nDYX = ( nDX - nDY ) << 1;
            const long  nDY2 = nDX << 1;
            long        nD = nDY2 - nDY;
            BOOL        bPos = nX1 < nX2;

            for ( nX = nX1, nY = nY1; nY <= nY2; nY++ )
            {
                SetPixel( nY, nX, maLineColor );

                if ( nD < 0 )
                    nD += nDY2;
                else
                {
                    nD += nDYX;

                    if ( bPos )
                        nX++;
                    else
                        nX--;
                }
            }
        }
    }
}

// ------------------------------------------------------------------

void BitmapWriteAccess::DrawRect( const Rectangle& rRect )
{
    ImplInitDraw();
    FillRect( rRect );
    DrawLine( rRect.TopLeft(), rRect.TopRight() );
    DrawLine( rRect.TopRight(), rRect.BottomRight() );
    DrawLine( rRect.BottomRight(), rRect.BottomLeft() );
    DrawLine( rRect.BottomLeft(), rRect.TopLeft() );
}

// ------------------------------------------------------------------

void BitmapWriteAccess::FillRect( const Rectangle& rRect )
{
    Point aPoint;
    Rectangle aRect( aPoint, maBitmap.GetSizePixel() );

    aRect.Intersection( rRect );

    if( !aRect.IsEmpty() )
    {
        const long  nStartX = rRect.TopLeft().X();
        const long  nStartY = rRect.TopLeft().Y();
        const long  nEndX = rRect.BottomRight().X();
        const long  nEndY = rRect.BottomRight().Y();

        ImplInitDraw();

        for( long nY = nStartY; nY <= nEndY; nY++ )
            for( long nX = nStartX; nX <= nEndX; nX++ )
                SetPixel( nY, nX, maFillColor );
    }
}

// ------------------------------------------------------------------

void BitmapWriteAccess::DrawPolygon( const Polygon& rPoly )
{
    const USHORT nSize = rPoly.GetSize();

    if( nSize )
    {
        Region      aRegion( rPoly );
        Rectangle   aRect;

        aRegion.Intersect( Rectangle( Point(), Size( Width(), Height() ) ) );

        if( !aRegion.IsEmpty() )
        {
            RegionHandle aRegHandle( aRegion.BeginEnumRects() );

            ImplInitDraw();

            while( aRegion.GetNextEnumRect( aRegHandle, aRect ) )
                for( long nY = aRect.Top(), nEndY = aRect.Bottom(); nY <= nEndY; nY++ )
                    for( long nX = aRect.Left(), nEndX = aRect.Right(); nX <= nEndX; nX++ )
                        SetPixel( nY, nX, maFillColor );

            aRegion.EndEnumRects( aRegHandle );
        }

        if( maLineColor != maFillColor )
        {
            for( USHORT i = 0, nSize1 = nSize - 1; i < nSize1; i++ )
                DrawLine( rPoly[ i ], rPoly[ i + 1 ] );

            if( rPoly[ nSize - 1 ] != rPoly[ 0 ] )
                DrawLine( rPoly[ nSize - 1 ], rPoly[ 0 ] );
        }
    }
}

// ------------------------------------------------------------------

void BitmapWriteAccess::DrawPolyPolygon( const PolyPolygon& rPolyPoly )
{
    const USHORT nCount = rPolyPoly.Count();

    if( nCount )
    {
        Region      aRegion( rPolyPoly );
        Rectangle   aRect;

        aRegion.Intersect( Rectangle( Point(), Size( Width(), Height() ) ) );

        if( !aRegion.IsEmpty() )
        {
            RegionHandle aRegHandle( aRegion.BeginEnumRects() );

            ImplInitDraw();

            while( aRegion.GetNextEnumRect( aRegHandle, aRect ) )
                for( long nY = aRect.Top(), nEndY = aRect.Bottom(); nY <= nEndY; nY++ )
                    for( long nX = aRect.Left(), nEndX = aRect.Right(); nX <= nEndX; nX++ )
                        SetPixel( nY, nX, maFillColor );

            aRegion.EndEnumRects( aRegHandle );
        }

        if( maLineColor != maFillColor )
        {
            for( USHORT n = 0; n < nCount; )
            {
                const Polygon&  rPoly = rPolyPoly[ n++ ];
                const USHORT    nSize = rPoly.GetSize();

                if( nSize )
                {
                    for( USHORT i = 0, nSize1 = nSize - 1; i < nSize1; i++ )
                        DrawLine( rPoly[ i ], rPoly[ i + 1 ] );

                    if( rPoly[ nSize - 1 ] != rPoly[ 0 ] )
                        DrawLine( rPoly[ nSize - 1 ], rPoly[ 0 ] );
                }
            }
        }
    }
}
