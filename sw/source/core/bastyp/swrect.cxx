/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: swrect.cxx,v $
 * $Revision: 1.19 $
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


#ifdef DBG_UTIL
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#endif
#include <stdlib.h>
#include "swrect.hxx"
#include <math.h>

/*************************************************************************
|*
|*  SwRect::SwRect()
|*
|*  Ersterstellung      MA 02. Feb. 93
|*  Letzte Aenderung    MA 05. Sep. 93
|*
|*************************************************************************/



SwRect::SwRect( const Rectangle &rRect ) :
    nX( rRect.Left() ),
    nY( rRect.Top() )
{
    nWidth  = rRect.Right() == RECT_EMPTY ? 0 :
                            rRect.Right()  - rRect.Left() +1;
    nHeight = rRect.Bottom() == RECT_EMPTY ? 0 :
                            rRect.Bottom() - rRect.Top() + 1;
}

/*************************************************************************
|*
|*  SwRect::Center()
|*
|*  Ersterstellung      MA 27. Jan. 93
|*  Letzte Aenderung    MA 27. Jan. 93
|*
|*************************************************************************/
Point SwRect::Center() const
{
    return Point( Left() + Width()  / 2,
                  Top()  + Height() / 2 );

/*  Wer ruft schon ein Center auf ein "falsches" Rechteck?
    const long nRight = Right();
    const long nBottom= Bottom();
    return Point( min( Left(), nRight ) + long(abs( (nRight - Left())/2) ),
                  min( Top(),  nBottom) + long(abs( (nBottom - Top())/2)));
*/
}

/*************************************************************************
|*
|*  SwRect::Union()
|*
|*  Ersterstellung      MA 27. Jan. 93
|*  Letzte Aenderung    MA 27. Jan. 93
|*
|*************************************************************************/



SwRect& SwRect::Union( const SwRect& rRect )
{
    if ( Top() > rRect.Top() )
        Top( rRect.Top() );
    if ( Left() > rRect.Left() )
        Left( rRect.Left() );
    long n = rRect.Right();
    if ( Right() < n )
        Right( n );
    n = rRect.Bottom();
    if ( Bottom() < n )
        Bottom( n );
    return *this;
}
/*************************************************************************
|*
|*  SwRect::Intersection(), _Intersection()
|*
|*  Ersterstellung      MA 27. Jan. 93
|*  Letzte Aenderung    MA 05. Sep. 93
|*
|*************************************************************************/



SwRect& SwRect::Intersection( const SwRect& rRect )
{
    //Hat das Teil ueberhaupt Gemeinsamkeiten mit mir?
    if ( IsOver( rRect ) )
    {
        //Bestimmung der kleineren  rechten sowie unteren und
        //           der groesseren linken  sowie oberen Kante.
        if ( Left() < rRect.Left() )
            Left( rRect.Left() );
        if ( Top() < rRect.Top() )
            Top( rRect.Top() );
        long n = rRect.Right();
        if ( Right() > n )
            Right( n );
        n = rRect.Bottom();
        if ( Bottom() > n )
            Bottom( n );
    }
    else
        //Def.: Bei einer leeren Intersection wird nur die SSize genullt.
        nHeight = nWidth = 0;

    return *this;
}



SwRect& SwRect::_Intersection( const SwRect& rRect )
{
    //Bestimmung der kleineren  rechten sowie unteren und
    //           der groesseren linken  sowie oberen Kante.
    if ( Left() < rRect.Left() )
        Left( rRect.Left() );
    if ( Top() < rRect.Top() )
        Top( rRect.Top() );
    long n = rRect.Right();
    if ( Right() > n )
        Right( n );
    n = rRect.Bottom();
    if ( Bottom() > n )
        Bottom( n );

    return *this;
}
/*************************************************************************
|*
|*  SwRect::IsInside()
|*
|*  Ersterstellung      MA 27. Jan. 93
|*  Letzte Aenderung    MA 27. Jan. 93
|*
|*************************************************************************/



BOOL SwRect::IsInside( const SwRect& rRect ) const
{
    const long nRight  = Right();
    const long nBottom = Bottom();
    const long nrRight = rRect.Right();
    const long nrBottom= rRect.Bottom();
    return (Left() <= rRect.Left()) && (rRect.Left()<= nRight)  &&
           (Left() <= nrRight)      && (nrRight     <= nRight)  &&
           (Top()  <= rRect.Top())  && (rRect.Top() <= nBottom) &&
           (Top()  <= nrBottom)     && (nrBottom    <= nBottom);
}



BOOL SwRect::IsInside( const Point& rPoint ) const
{
    return    (Left()  <= rPoint.X())
           && (Top()   <= rPoint.Y())
           && (Right() >= rPoint.X())
           && (Bottom()>= rPoint.Y());
}
/* -----------------------------11.04.00 15:46--------------------------------
    mouse moving of table borders
 ---------------------------------------------------------------------------*/
BOOL SwRect::IsNear( const Point& rPoint, long nTolerance ) const
{
    return    IsInside(rPoint) ||
        (((Left() - nTolerance)  <= rPoint.X())
           && ((Top()  - nTolerance)  <= rPoint.Y())
           && ((Right() + nTolerance) >= rPoint.X())
           && ((Bottom()  + nTolerance)>= rPoint.Y()));
}

/*************************************************************************
|*
|*  SwRect::IsOver()
|*
|*  Ersterstellung      MA 25. Feb. 94
|*  Letzte Aenderung    MA 27. Jun. 96
|*
|*************************************************************************/



BOOL SwRect::IsOver( const SwRect& rRect ) const
{
    return    (Top()   <= rRect.Bottom())
           && (Left()  <= rRect.Right())
           && (Right() >= rRect.Left())
           && (Bottom()>= rRect.Top()) ? TRUE : FALSE;
}

/*************************************************************************
|*
|*  SwRect::Justify()
|*
|*  Ersterstellung      MA 10. Oct. 94
|*  Letzte Aenderung    MA 23. Oct. 96
|*
|*************************************************************************/



void SwRect::Justify()
{
    if ( nHeight < 0 )
    {
        nY = nY + nHeight + 1;
        nHeight = -nHeight;
    }
    if ( nWidth < 0 )
    {
        nX = nX + nWidth + 1;
        nWidth = -nWidth;
    }
}


// Similiar to the inline methods, but we need the function pointers

void SwRect::_Width( const long nNew ) { nWidth = nNew; }
void SwRect::_Height( const long nNew ) { nHeight = nNew; }
void SwRect::_Left( const long nLeft ){ nWidth += nX - nLeft; nX = nLeft; }
void SwRect::_Right( const long nRight ){ nWidth = nRight - nX; }
void SwRect::_Top( const long nTop ){ nHeight += nY - nTop; nY = nTop; }
void SwRect::_Bottom( const long nBottom ){ nHeight = nBottom - nY; }

long SwRect::_Width() const{ return nWidth; }
long SwRect::_Height() const{ return nHeight; }
long SwRect::_Left() const{ return nX; }
long SwRect::_Right() const{ return nX + nWidth; }
long SwRect::_Top() const{ return nY; }
long SwRect::_Bottom() const{ return nY + nHeight; }

void SwRect::AddWidth( const long nAdd ) { nWidth += nAdd; }
void SwRect::AddHeight( const long nAdd ) { nHeight += nAdd; }
void SwRect::SubLeft( const long nSub ){ nWidth += nSub; nX -= nSub; }
void SwRect::AddRight( const long nAdd ){ nWidth += nAdd; }
void SwRect::SubTop( const long nSub ){ nHeight += nSub; nY -= nSub; }
void SwRect::AddBottom( const long nAdd ){ nHeight += nAdd; }
void SwRect::SetPosX( const long nNew ){ nX = nNew; }
void SwRect::SetPosY( const long nNew ){ nY = nNew; }
const Size  SwRect::_Size() const { return SSize(); }
const Size  SwRect::SwappedSize() const { return Size( nHeight, nWidth ); }
const Point SwRect::TopLeft() const { return Pos(); }
const Point SwRect::TopRight() const { return Point( nX + nWidth, nY ); }
const Point SwRect::BottomLeft() const { return Point( nX, nY + nHeight ); }
const Point SwRect::BottomRight() const
    { return Point( nX + nWidth, nY + nHeight ); }
long SwRect::GetLeftDistance( long nLimit ) const { return nX - nLimit; }
long SwRect::GetBottomDistance( long nLim ) const { return nLim - nY - nHeight;}
long SwRect::GetTopDistance( long nLimit ) const { return nY - nLimit; }
long SwRect::GetRightDistance( long nLim ) const { return nLim - nX - nWidth; }
BOOL SwRect::OverStepLeft( long nLimit ) const
    { return nLimit > nX && nX + nWidth > nLimit; }
BOOL SwRect::OverStepBottom( long nLimit ) const
    { return nLimit > nY && nY + nHeight > nLimit; }
BOOL SwRect::OverStepTop( long nLimit ) const
    { return nLimit > nY && nY + nHeight > nLimit; }
BOOL SwRect::OverStepRight( long nLimit ) const
    { return nLimit > nX && nX + nWidth > nLimit; }
void SwRect::SetLeftAndWidth( long nLeft, long nNew )
    { nX = nLeft; nWidth = nNew; }
void SwRect::SetTopAndHeight( long nTop, long nNew )
    { nY = nTop; nHeight = nNew; }
void SwRect::SetRightAndWidth( long nRight, long nNew )
    { nX = nRight - nNew; nWidth = nNew; }
void SwRect::SetBottomAndHeight( long nBottom, long nNew )
    { nY = nBottom - nNew; nHeight = nNew; }
void SwRect::SetUpperLeftCorner(  const Point& rNew )
    { nX = rNew.nA; nY = rNew.nB; }
void SwRect::SetUpperRightCorner(  const Point& rNew )
    { nX = rNew.nA - nWidth; nY = rNew.nB; }
void SwRect::SetLowerLeftCorner(  const Point& rNew )
    { nX = rNew.nA; nY = rNew.nB - nHeight; }

#ifdef DBG_UTIL
/*************************************************************************
 *                  operator<<( ostream&, SwRect&)
 *************************************************************************/



SvStream &operator<<( SvStream &rStream, const SwRect &rRect )
{
    rStream << '[' << rRect.Top()   << '/' << rRect.Left()
            << ',' << rRect.Width() << 'x' << rRect.Height() << "] ";
    return rStream;
}
#endif


