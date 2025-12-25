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
    m_Point( rRect.Left(), rRect.Top() ),
    m_Size( rRect.IsWidthEmpty() ? 0 : rRect.Right() - rRect.Left() + 1,
            rRect.IsHeightEmpty() ? 0 : rRect.Bottom() - rRect.Top() + 1 )
{
}

SwRect& SwRect::Union( const SwRect& rRect )
{
    if( rRect.IsEmpty())
        return *this;
    if( IsEmpty())
    {
        *this = rRect;
        return *this;
    }
    if ( Top() > rRect.Top() )
        Top( rRect.Top() );
    if ( Left() > rRect.Left() )
        Left( rRect.Left() );
    if ( tools::Long n = rRect.Right(); Right() < n )
        Right( n );
    if ( tools::Long n = rRect.Bottom(); Bottom() < n )
        Bottom( n );
    return *this;
}

SwRect& SwRect::Intersection( const SwRect& rRect )
{
    // any similarity between me and given element?
    if ( Overlaps( rRect ) )
    {
        // get smaller right and lower, and greater left and upper edge
        if ( Left() < rRect.Left() )
            Left( rRect.Left() );
        if ( Top() < rRect.Top() )
            Top( rRect.Top() );
        tools::Long n = rRect.Right();
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

SwRect& SwRect::Intersection_( const SwRect& rOther )
{
    // get smaller right and lower, and greater left and upper edge
    auto left   = std::max( Left(), rOther.Left() );
    auto top    = std::max( Top(), rOther.Top() );
    auto right  = std::min( Left() + Width(), rOther.Left() + rOther.Width() );
    auto bottom = std::min( Top() + Height(), rOther.Top() + rOther.Height() );

    Chg({ left, top }, { right - left, bottom - top });

    return *this;
}

void SwRect::Justify()
{
    if ( Height() < 0 )
    {
        SetTopAndHeight(Top() + Height() + 1, -Height());
    }
    if ( Width() < 0 )
    {
        SetLeftAndWidth(Left() + Width() + 1, -Width());
    }
}

// Similar to the inline methods, but we need the function pointers
void SwRect::Width_( const tools::Long nNew ) { Width(nNew); }
void SwRect::Height_( const tools::Long nNew ) { Height(nNew); }
void SwRect::Left_( const tools::Long nLeft ) { Left(nLeft); }
void SwRect::Right_( const tools::Long nRight ) { Width(nRight - Left()); }
void SwRect::Top_( const tools::Long nTop ) { Top(nTop); }
void SwRect::Bottom_( const tools::Long nBottom ) { Height(nBottom - Top()); }

tools::Long SwRect::Width_() const{ return Width(); }
tools::Long SwRect::Height_() const{ return Height(); }
tools::Long SwRect::Left_() const{ return Left(); }
tools::Long SwRect::Right_() const{ return Left() + Width(); }
tools::Long SwRect::Top_() const{ return Top(); }
tools::Long SwRect::Bottom_() const{ return Top() + Height(); }

void SwRect::AddWidth( const tools::Long nAdd ) { m_Size.AdjustWidth(nAdd ); }
void SwRect::AddHeight( const tools::Long nAdd ) { m_Size.AdjustHeight(nAdd ); }
void SwRect::AddLeft( const tools::Long nAdd ) { SubLeft(-nAdd); }
void SwRect::SubLeft( const tools::Long nSub ) { AddWidth(nSub); SetPosX(Left() - nSub); }
void SwRect::AddRight( const tools::Long nAdd ) { AddWidth(nAdd); }
void SwRect::AddTop( const tools::Long nAdd ) { SubTop(-nAdd); }
void SwRect::SubTop( const tools::Long nSub ) { AddHeight(nSub); SetPosY(Top() - nSub); }
void SwRect::AddBottom( const tools::Long nAdd ) { AddHeight(nAdd); }
void SwRect::SetPosX( const tools::Long nNew ){ m_Point.setX(nNew); }
void SwRect::SetPosY( const tools::Long nNew ){ m_Point.setY(nNew); }

Size  SwRect::Size_() const { return SSize(); }
Size SwRect::SwappedSize() const { return Size(Height(), Width()); }

tools::Long SwRect::GetLeftDistance(tools::Long nLimit) const { return Left() - nLimit; }
tools::Long SwRect::GetTopDistance(tools::Long nLimit) const { return Top() - nLimit; }
tools::Long SwRect::GetRightDistance(tools::Long nLim) const { return nLim - Left() - Width(); }
tools::Long SwRect::GetBottomDistance(tools::Long nLim) const { return nLim - Top() - Height(); }

bool SwRect::OverStepLeft( tools::Long nLimit ) const
    { return nLimit > Left() && Left() + Width() > nLimit; }
bool SwRect::OverStepRight( tools::Long nLimit ) const { return OverStepLeft(nLimit); }
bool SwRect::OverStepTop( tools::Long nLimit ) const
    { return nLimit > Top() && Top() + Height() > nLimit; }
bool SwRect::OverStepBottom( tools::Long nLimit ) const { return OverStepTop(nLimit); }

void SwRect::SetLeftAndWidth( tools::Long nLeft, tools::Long nNew )
{
    SetPosX(nLeft);
    Width(nNew);
}
void SwRect::SetTopAndHeight( tools::Long nTop, tools::Long nNew )
{
    SetPosY(nTop);
    Height(nNew);
}
void SwRect::SetRightAndWidth( tools::Long nRight, tools::Long nNew )
{
    SetLeftAndWidth(nRight - nNew, nNew);
}
void SwRect::SetBottomAndHeight( tools::Long nBottom, tools::Long nNew )
{
    SetTopAndHeight(nBottom - nNew, nNew);
}
void SwRect::SetUpperLeftCorner(const Point& rNew) { Pos(rNew); }
void SwRect::SetUpperRightCorner(const Point& rNew) { Pos(rNew.X() - Width(), rNew.Y()); }
void SwRect::SetLowerLeftCorner(const Point& rNew) { Pos(rNew.X(), rNew.Y() - Height()); }

void SwRect::dumpAsXmlAttributes(xmlTextWriterPtr writer) const
{
    (void)xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("left"), "%li", Left());
    (void)xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("top"), "%li", Top());
    (void)xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("width"), "%li", Width());
    (void)xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("height"), "%li", Height());
    (void)xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("bottom"), "%li", Bottom());
    (void)xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("right"), "%li", Right());
}

#ifdef DBG_UTIL
SvStream& WriteSwRect(SvStream &rStream, const SwRect &rRect)
{
    rStream.WriteChar('[').WriteInt32(rRect.Top()).
            WriteChar('/').WriteInt32(rRect.Left()).
            WriteChar(',').WriteInt32(rRect.Width()).
            WriteChar('x').WriteInt32(rRect.Height()).
            WriteOString("] ");
    return rStream;
}
#endif

static_assert( std::is_trivially_copyable< SwRect >::value );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
