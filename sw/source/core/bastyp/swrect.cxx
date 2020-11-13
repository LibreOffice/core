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

#include <libxml/xmlwriter.h>

#ifdef DBG_UTIL
#include <tools/stream.hxx>
#endif

SwRect::SwRect( const tools::Rectangle &rRect ) :
    m_Point( SwTwips(rRect.Left()), SwTwips(rRect.Top()) )
{
    m_Size.setWidth( SwTwips(rRect.IsWidthEmpty() ? 0 : rRect.Right()  - rRect.Left() + 1));
    m_Size.setHeight(SwTwips(rRect.IsHeightEmpty() ? 0 : rRect.Bottom() - rRect.Top() + 1));
}

SwPoint SwRect::Center() const
{
    return SwPoint( Left() + Width()  / SwTwips(2),
                    Top()  + Height() / SwTwips(2) );
}

SwRect& SwRect::Union( const SwRect& rRect )
{
    if ( Top() > rRect.Top() )
        Top( rRect.Top() );
    if ( Left() > rRect.Left() )
        Left( rRect.Left() );
    SwTwips n = rRect.Right();
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
        SwTwips n = rRect.Right();
        if ( Right() > n )
            Right( n );
        n = rRect.Bottom();
        if ( Bottom() > n )
            Bottom( n );
    }
    else
        // Def.: if intersection is empty, set only SSize to 0
        SSize(SwTwips(0), SwTwips(0));

    return *this;
}

SwRect& SwRect::Intersection_( const SwRect& rOther )
{
    // get smaller right and lower, and greater left and upper edge
    auto left   = std::max( m_Point.X(), rOther.m_Point.X() );
    auto top    = std::max( m_Point.Y(), rOther.m_Point.Y() );
    auto right  = std::min( m_Point.X() + m_Size.Width(), rOther.m_Point.X() + rOther.m_Size.Width() );
    auto bottom = std::min( m_Point.Y() + m_Size.Height(), rOther.m_Point.Y() + rOther.m_Size.Height() );

    *this = SwRect( left, top, right - left, bottom - top );

    return *this;
}

bool SwRect::IsInside( const SwRect& rRect ) const
{
    const SwTwips nRight  = Right();
    const SwTwips nBottom = Bottom();
    const SwTwips nrRight = rRect.Right();
    const SwTwips nrBottom= rRect.Bottom();
    return (Left() <= rRect.Left()) && (rRect.Left()<= nRight)  &&
           (Left() <= nrRight)      && (nrRight     <= nRight)  &&
           (Top()  <= rRect.Top())  && (rRect.Top() <= nBottom) &&
           (Top()  <= nrBottom)     && (nrBottom    <= nBottom);
}

bool SwRect::IsInside( const SwPoint& rPoint ) const
{
    return (Left()  <= rPoint.X()) &&
           (Top()   <= rPoint.Y()) &&
           (Right() >= rPoint.X()) &&
           (Bottom()>= rPoint.Y());
}

// mouse moving of table borders
bool SwRect::IsNear( const SwPoint& rPoint, SwTwips nTolerance ) const
{
    bool bIsNearby = (((Left()   - nTolerance) <= rPoint.X()) &&
                      ((Top()    - nTolerance) <= rPoint.Y()) &&
                      ((Right()  + nTolerance) >= rPoint.X()) &&
                      ((Bottom() + nTolerance) >= rPoint.Y()));
    return IsInside(rPoint) || bIsNearby;
}

bool SwRect::IsOver( const SwRect& rRect ) const
{
    return (Top()   <= rRect.Bottom()) &&
           (Left()  <= rRect.Right())  &&
           (Right() >= rRect.Left())   &&
           (Bottom()>= rRect.Top());
}

void SwRect::Justify()
{
    if ( m_Size.getHeight() < SwTwips(0) )
    {
        m_Point.setY(m_Point.getY() + m_Size.getHeight() + SwTwips(1));
        m_Size.setHeight(-m_Size.getHeight());
    }
    if ( m_Size.getWidth() < SwTwips(0) )
    {
        m_Point.setX(m_Point.getX() + m_Size.getWidth() + SwTwips(1));
        m_Size.setWidth(-m_Size.getWidth());
    }
}

// Similar to the inline methods, but we need the function pointers
void SwRect::Width_( const SwTwips nNew ) { m_Size.setWidth(nNew); }
void SwRect::Height_( const SwTwips nNew ) { m_Size.setHeight(nNew); }
void SwRect::Left_( const SwTwips nLeft ){ m_Size.AdjustWidth(m_Point.getX() - nLeft ); m_Point.setX(nLeft); }
void SwRect::Right_( const SwTwips nRight ){ m_Size.setWidth(nRight - m_Point.getX()); }
void SwRect::Top_( const SwTwips nTop ){ m_Size.AdjustHeight(m_Point.getY() - nTop ); m_Point.setY(nTop); }
void SwRect::Bottom_( const SwTwips nBottom ){ m_Size.setHeight(nBottom - m_Point.getY()); }

SwTwips SwRect::Width_() const{ return m_Size.getWidth(); }
SwTwips SwRect::Height_() const{ return m_Size.getHeight(); }
SwTwips SwRect::Left_() const{ return m_Point.getX(); }
SwTwips SwRect::Right_() const{ return m_Point.getX() + m_Size.getWidth(); }
SwTwips SwRect::Top_() const{ return m_Point.getY(); }
SwTwips SwRect::Bottom_() const{ return m_Point.getY() + m_Size.getHeight(); }

void SwRect::AddWidth( const SwTwips nAdd ) { m_Size.AdjustWidth(nAdd ); }
void SwRect::AddHeight( const SwTwips nAdd ) { m_Size.AdjustHeight(nAdd ); }
void SwRect::AddLeft( const SwTwips nAdd ){ m_Size.AdjustWidth(-nAdd ); m_Point.setX(m_Point.getX() + nAdd); }
void SwRect::SubLeft( const SwTwips nSub ){ m_Size.AdjustWidth(nSub ); m_Point.setX(m_Point.getX() - nSub); }
void SwRect::AddRight( const SwTwips nAdd ){ m_Size.AdjustWidth(nAdd ); }
void SwRect::AddTop( const SwTwips nAdd ){ m_Size.AdjustHeight(-nAdd ); m_Point.setY(m_Point.getY() + nAdd); }
void SwRect::SubTop( const SwTwips nSub ){ m_Size.AdjustHeight(nSub ); m_Point.setY(m_Point.getY() - nSub); }
void SwRect::AddBottom( const SwTwips nAdd ){ m_Size.AdjustHeight(nAdd ); }
void SwRect::SetPosX( const SwTwips nNew ){ m_Point.setX(nNew); }
void SwRect::SetPosY( const SwTwips nNew ){ m_Point.setY(nNew); }

SwSize  SwRect::Size_() const { return SSize(); }
SwSize  SwRect::SwappedSize() const { return SwSize( m_Size.getHeight(), m_Size.getWidth() ); }

SwPoint SwRect::TopLeft() const { return Pos(); }
SwPoint SwRect::TopRight() const { return SwPoint( m_Point.getX() + m_Size.getWidth(), m_Point.getY() ); }
SwPoint SwRect::BottomLeft() const { return SwPoint( m_Point.getX(), m_Point.getY() + m_Size.getHeight() ); }
SwPoint SwRect::BottomRight() const
    { return SwPoint( m_Point.getX() + m_Size.getWidth(), m_Point.getY() + m_Size.getHeight() ); }

SwTwips SwRect::GetLeftDistance( SwTwips nLimit ) const { return m_Point.getX() - nLimit; }
SwTwips SwRect::GetBottomDistance( SwTwips nLim ) const { return nLim - m_Point.getY() - m_Size.getHeight();}
SwTwips SwRect::GetTopDistance( SwTwips nLimit ) const { return m_Point.getY() - nLimit; }
SwTwips SwRect::GetRightDistance( SwTwips nLim ) const { return nLim - m_Point.getX() - m_Size.getWidth(); }

bool SwRect::OverStepLeft( SwTwips nLimit ) const
    { return nLimit > m_Point.getX() && m_Point.getX() + m_Size.getWidth() > nLimit; }
bool SwRect::OverStepBottom( SwTwips nLimit ) const
    { return nLimit > m_Point.getY() && m_Point.getY() + m_Size.getHeight() > nLimit; }
bool SwRect::OverStepTop( SwTwips nLimit ) const
    { return nLimit > m_Point.getY() && m_Point.getY() + m_Size.getHeight() > nLimit; }
bool SwRect::OverStepRight( SwTwips nLimit ) const
    { return nLimit > m_Point.getX() && m_Point.getX() + m_Size.getWidth() > nLimit; }

void SwRect::SetLeftAndWidth( SwTwips nLeft, SwTwips nNew )
{
    m_Point.setX(nLeft);
    m_Size.setWidth(nNew);
}
void SwRect::SetTopAndHeight( SwTwips nTop, SwTwips nNew )
{
    m_Point.setY(nTop);
    m_Size.setHeight(nNew);
}
void SwRect::SetRightAndWidth( SwTwips nRight, SwTwips nNew )
{
    m_Point.setX(nRight - nNew);
    m_Size.setWidth(nNew);
}
void SwRect::SetBottomAndHeight( SwTwips nBottom, SwTwips nNew )
{
    m_Point.setY(nBottom - nNew);
    m_Size.setHeight(nNew);
}
void SwRect::SetUpperLeftCorner(  const SwPoint& rNew )
    { m_Point = rNew; }
void SwRect::SetUpperRightCorner(  const SwPoint& rNew )
    { m_Point = SwPoint(rNew.X() - m_Size.getWidth(), rNew.Y()); }
void SwRect::SetLowerLeftCorner(  const SwPoint& rNew )
    { m_Point = SwPoint(rNew.X(), rNew.Y() - m_Size.getHeight()); }

void SwRect::dumpAsXmlAttributes(xmlTextWriterPtr writer) const
{
    (void)xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("left"), "%li", tools::Long(Left()));
    (void)xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("top"), "%li", tools::Long(Top()));
    (void)xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("width"), "%li", tools::Long(Width()));
    (void)xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("height"), "%li", tools::Long(Height()));
    (void)xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("bottom"), "%li", tools::Long(Bottom()));
    (void)xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("right"), "%li", tools::Long(Right()));
}

#ifdef DBG_UTIL
SvStream& WriteSwRect(SvStream &rStream, const SwRect &rRect)
{
    rStream.WriteChar('[').WriteInt32(tools::Long(rRect.Top())).
            WriteChar('/').WriteInt32(tools::Long(rRect.Left())).
            WriteChar(',').WriteInt32(tools::Long(rRect.Width())).
            WriteChar('x').WriteInt32(tools::Long(rRect.Height())).
            WriteCharPtr("] ");
    return rStream;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
