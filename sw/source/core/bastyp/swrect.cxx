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


#if OSL_DEBUG_LEVEL > 1
#include <tools/stream.hxx>
#endif
#include <stdlib.h>
#include "swrect.hxx"
#include <math.h>

/*************************************************************************
|*
|*  SwRect::SwRect()
|*
|*************************************************************************/



SwRect::SwRect( const Rectangle &rRect ) :
    m_Point( rRect.Left(), rRect.Top() )
{
    m_Size.setWidth(rRect.Right() == RECT_EMPTY ? 0 :
                            rRect.Right()  - rRect.Left() +1);
    m_Size.setHeight(rRect.Bottom() == RECT_EMPTY ? 0 :
                            rRect.Bottom() - rRect.Top() + 1);
}

/*************************************************************************
|*
|*  SwRect::Center()
|*
|*************************************************************************/
Point SwRect::Center() const
{
    return Point( Left() + Width()  / 2,
                  Top()  + Height() / 2 );
}

/*************************************************************************
|*
|*  SwRect::Union()
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
        SSize(0, 0);

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
|*************************************************************************/



sal_Bool SwRect::IsInside( const SwRect& rRect ) const
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



sal_Bool SwRect::IsInside( const Point& rPoint ) const
{
    return    (Left()  <= rPoint.X())
           && (Top()   <= rPoint.Y())
           && (Right() >= rPoint.X())
           && (Bottom()>= rPoint.Y());
}
/* ---------------------------------------------------------------------------
    mouse moving of table borders
 ---------------------------------------------------------------------------*/
sal_Bool SwRect::IsNear( const Point& rPoint, long nTolerance ) const
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
|*************************************************************************/



sal_Bool SwRect::IsOver( const SwRect& rRect ) const
{
    return    (Top()   <= rRect.Bottom())
           && (Left()  <= rRect.Right())
           && (Right() >= rRect.Left())
           && (Bottom()>= rRect.Top()) ? sal_True : sal_False;
}

/*************************************************************************
|*
|*  SwRect::Justify()
|*
|*************************************************************************/



void SwRect::Justify()
{
    if ( m_Size.getHeight() < 0 )
    {
        m_Point.Y() += m_Size.getHeight() + 1;
        m_Size.setHeight(-m_Size.getHeight());
    }
    if ( m_Size.getWidth() < 0 )
    {
        m_Point.X() += m_Size.getWidth() + 1;
        m_Size.setWidth(-m_Size.getWidth());
    }
}


// Similiar to the inline methods, but we need the function pointers

void SwRect::_Width( const long nNew ) { m_Size.setWidth(nNew); }
void SwRect::_Height( const long nNew ) { m_Size.setHeight(nNew); }
void SwRect::_Left( const long nLeft ){ m_Size.Width() += m_Point.getX() - nLeft; m_Point.setX(nLeft); }
void SwRect::_Right( const long nRight ){ m_Size.setWidth(nRight - m_Point.getX()); }
void SwRect::_Top( const long nTop ){ m_Size.Height() += m_Point.getY() - nTop; m_Point.setY(nTop); }
void SwRect::_Bottom( const long nBottom ){ m_Size.setHeight(nBottom - m_Point.getY()); }

long SwRect::_Width() const{ return m_Size.getWidth(); }
long SwRect::_Height() const{ return m_Size.getHeight(); }
long SwRect::_Left() const{ return m_Point.getX(); }
long SwRect::_Right() const{ return m_Point.getX() + m_Size.getWidth(); }
long SwRect::_Top() const{ return m_Point.getY(); }
long SwRect::_Bottom() const{ return m_Point.getY() + m_Size.getHeight(); }

void SwRect::AddWidth( const long nAdd ) { m_Size.Width() += nAdd; }
void SwRect::AddHeight( const long nAdd ) { m_Size.Height() += nAdd; }
void SwRect::SubLeft( const long nSub ){ m_Size.Width() += nSub; m_Point.X() -= nSub; }
void SwRect::AddRight( const long nAdd ){ m_Size.Width() += nAdd; }
void SwRect::SubTop( const long nSub ){ m_Size.Height() += nSub; m_Point.Y() -= nSub; }
void SwRect::AddBottom( const long nAdd ){ m_Size.Height() += nAdd; }
void SwRect::SetPosX( const long nNew ){ m_Point.setX(nNew); }
void SwRect::SetPosY( const long nNew ){ m_Point.setY(nNew); }
const Size  SwRect::_Size() const { return SSize(); }
const Size  SwRect::SwappedSize() const { return Size( m_Size.getHeight(), m_Size.getWidth() ); }
const Point SwRect::TopLeft() const { return Pos(); }
const Point SwRect::TopRight() const { return Point( m_Point.getX() + m_Size.getWidth(), m_Point.getY() ); }
const Point SwRect::BottomLeft() const { return Point( m_Point.getX(), m_Point.getY() + m_Size.getHeight() ); }
const Point SwRect::BottomRight() const
    { return Point( m_Point.getX() + m_Size.getWidth(), m_Point.getY() + m_Size.getHeight() ); }
long SwRect::GetLeftDistance( long nLimit ) const { return m_Point.getX() - nLimit; }
long SwRect::GetBottomDistance( long nLim ) const { return nLim - m_Point.getY() - m_Size.getHeight();}
long SwRect::GetTopDistance( long nLimit ) const { return m_Point.getY() - nLimit; }
long SwRect::GetRightDistance( long nLim ) const { return nLim - m_Point.getX() - m_Size.getWidth(); }
sal_Bool SwRect::OverStepLeft( long nLimit ) const
    { return nLimit > m_Point.getX() && m_Point.getX() + m_Size.getWidth() > nLimit; }
sal_Bool SwRect::OverStepBottom( long nLimit ) const
    { return nLimit > m_Point.getY() && m_Point.getY() + m_Size.getHeight() > nLimit; }
sal_Bool SwRect::OverStepTop( long nLimit ) const
    { return nLimit > m_Point.getY() && m_Point.getY() + m_Size.getHeight() > nLimit; }
sal_Bool SwRect::OverStepRight( long nLimit ) const
    { return nLimit > m_Point.getX() && m_Point.getX() + m_Size.getWidth() > nLimit; }
void SwRect::SetLeftAndWidth( long nLeft, long nNew )
{
    m_Point.setX(nLeft);
    m_Size.setWidth(nNew);
}
void SwRect::SetTopAndHeight( long nTop, long nNew )
{
    m_Point.setY(nTop);
    m_Size.setHeight(nNew);
}
void SwRect::SetRightAndWidth( long nRight, long nNew )
{
    m_Point.setX(nRight - nNew);
    m_Size.setWidth(nNew);
}
void SwRect::SetBottomAndHeight( long nBottom, long nNew )
{
    m_Point.setY(nBottom - nNew);
    m_Size.setHeight(nNew);
}
void SwRect::SetUpperLeftCorner(  const Point& rNew )
    { m_Point = rNew; }
void SwRect::SetUpperRightCorner(  const Point& rNew )
    { m_Point = Point(rNew.nA - m_Size.getWidth(), rNew.nB); }
void SwRect::SetLowerLeftCorner(  const Point& rNew )
    { m_Point = Point(rNew.nA, rNew.nB - m_Size.getHeight()); }

#if OSL_DEBUG_LEVEL > 1
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
