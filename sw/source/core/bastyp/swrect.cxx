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

#ifdef DBG_UTIL
#include <tools/stream.hxx>
#endif

#include <math.h>
#include <stdlib.h>

SwRect::SwRect( const Rectangle &rRect ) :
    m_Point( rRect.Left(), rRect.Top() )
{
    m_Size.setWidth(rRect.Right() == RECT_EMPTY ? 0 :
                            rRect.Right()  - rRect.Left() +1);
    m_Size.setHeight(rRect.Bottom() == RECT_EMPTY ? 0 :
                            rRect.Bottom() - rRect.Top() + 1);
}

Point SwRect::Center() const
{
    return Point( Left() + Width()  / 2,
                  Top()  + Height() / 2 );
}

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

SwRect& SwRect::Intersection( const SwRect& rRect )
{
    // any similarity between me and given element?
    if ( IsOver( rRect ) )
    {
        // get smaller right and lower, and greater left and upper edge
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
        // Def.: if intersection is empty, set only SSize to 0
        SSize(0, 0);

    return *this;
}

SwRect& SwRect::_Intersection( const SwRect& rRect )
{
    // get smaller right and lower, and greater left and upper edge
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
    return (Left()  <= rPoint.X()) &&
           (Top()   <= rPoint.Y()) &&
           (Right() >= rPoint.X()) &&
           (Bottom()>= rPoint.Y());
}

// mouse moving of table borders
sal_Bool SwRect::IsNear( const Point& rPoint, long nTolerance ) const
{
    bool bIsNearby = (((Left()   - nTolerance) <= rPoint.X()) &&
                      ((Top()    - nTolerance) <= rPoint.Y()) &&
                      ((Right()  + nTolerance) >= rPoint.X()) &&
                      ((Bottom() + nTolerance) >= rPoint.Y()));
    return IsInside(rPoint) || bIsNearby;
}

sal_Bool SwRect::IsOver( const SwRect& rRect ) const
{
    return (Top()   <= rRect.Bottom()) &&
           (Left()  <= rRect.Right())  &&
           (Right() >= rRect.Left())   &&
           (Bottom()>= rRect.Top());
}

void SwRect::Justify()
{
    if ( m_Size.getHeight() < 0 )
    {
        m_Point.setY(m_Point.getY() + m_Size.getHeight() + 1);
        m_Size.setHeight(-m_Size.getHeight());
    }
    if ( m_Size.getWidth() < 0 )
    {
        m_Point.setX(m_Point.getX() + m_Size.getWidth() + 1);
        m_Size.setWidth(-m_Size.getWidth());
    }
}

// Similar to the inline methods, but we need the function pointers
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
void SwRect::SubLeft( const long nSub ){ m_Size.Width() += nSub; m_Point.setX(m_Point.getX() - nSub); }
void SwRect::AddRight( const long nAdd ){ m_Size.Width() += nAdd; }
void SwRect::SubTop( const long nSub ){ m_Size.Height() += nSub; m_Point.setY(m_Point.getY() - nSub); }
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
    { m_Point = Point(rNew.A() - m_Size.getWidth(), rNew.B()); }
void SwRect::SetLowerLeftCorner(  const Point& rNew )
    { m_Point = Point(rNew.A(), rNew.B() - m_Size.getHeight()); }

#ifdef DBG_UTIL
SvStream &operator<<( SvStream &rStream, const SwRect &rRect )
{
    rStream << '[' << static_cast<sal_Int32>(rRect.Top())
            << '/' << static_cast<sal_Int32>(rRect.Left())
            << ',' << static_cast<sal_Int32>(rRect.Width())
            << 'x' << static_cast<sal_Int32>(rRect.Height())
            << "] ";
    return rStream;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
