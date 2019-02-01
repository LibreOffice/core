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

#include <sal/config.h>

#include <algorithm>
#include <sstream>
#include <o3tl/safeint.hxx>
#include <tools/gen.hxx>
#include <tools/stream.hxx>

SvStream& ReadPair( SvStream& rIStream, Pair& rPair )
{
    sal_Int32 nTmpA(0), nTmpB(0);
    rIStream.ReadInt32( nTmpA ).ReadInt32( nTmpB );
    rPair.nA = nTmpA;
    rPair.nB = nTmpB;

    return rIStream;
}

SvStream& WritePair( SvStream& rOStream, const Pair& rPair )
{
    rOStream.WriteInt32( rPair.nA ).WriteInt32( rPair.nB );

    return rOStream;
}

OString Pair::toString() const
{
    std::stringstream ss;
    // Note that this is not just used for debugging output but the
    // format is parsed by external code (passed in callbacks to
    // LibreOfficeKit clients). So don't change.
    ss << A() << ", " << B();
    return ss.str().c_str();
}

void tools::Rectangle::SetSize( const Size& rSize )
{
    if ( rSize.Width() < 0 )
        nRight  = nLeft + rSize.Width() +1;
    else if ( rSize.Width() > 0 )
        nRight  = nLeft + rSize.Width() -1;
    else
        nRight = RECT_EMPTY;

    if ( rSize.Height() < 0 )
        nBottom  = nTop + rSize.Height() +1;
    else if ( rSize.Height() > 0 )
        nBottom  = nTop + rSize.Height() -1;
    else
        nBottom = RECT_EMPTY;
}

void tools::Rectangle::SaturatingSetSize(const Size& rSize)
{
    if (rSize.Width() < 0)
        nRight = o3tl::saturating_add(nLeft, (rSize.Width() + 1));
    else if ( rSize.Width() > 0 )
        nRight = o3tl::saturating_add(nLeft, (rSize.Width() - 1));
    else
        nRight = RECT_EMPTY;

    if ( rSize.Height() < 0 )
        nBottom = o3tl::saturating_add(nTop, (rSize.Height() + 1));
    else if ( rSize.Height() > 0 )
        nBottom = o3tl::saturating_add(nTop, (rSize.Height() - 1));
    else
        nBottom = RECT_EMPTY;
}

void tools::Rectangle::SaturatingSetX(long x)
{
    nRight = o3tl::saturating_add(nRight, x - nLeft);
    nLeft = x;
}

void tools::Rectangle::SaturatingSetY(long y)
{
    nBottom = o3tl::saturating_add(nBottom, y - nTop);
    nTop = y;
}

tools::Rectangle& tools::Rectangle::Union( const tools::Rectangle& rRect )
{
    if ( rRect.IsEmpty() )
        return *this;

    if ( IsEmpty() )
        *this = rRect;
    else
    {
        nLeft  =  std::min( std::min( nLeft, rRect.nLeft ), std::min( nRight, rRect.nRight )   );
        nRight  = std::max( std::max( nLeft, rRect.nLeft ), std::max( nRight, rRect.nRight )   );
        nTop    = std::min( std::min( nTop, rRect.nTop ),   std::min( nBottom, rRect.nBottom ) );
        nBottom = std::max( std::max( nTop, rRect.nTop ),   std::max( nBottom, rRect.nBottom ) );
    }

    return *this;
}

tools::Rectangle& tools::Rectangle::Intersection( const tools::Rectangle& rRect )
{
    if ( IsEmpty() )
        return *this;
    if ( rRect.IsEmpty() )
    {
        *this = tools::Rectangle();
        return *this;
    }

    // Justify rectangle
    tools::Rectangle aTmpRect( rRect );
    Justify();
    aTmpRect.Justify();

    // Perform intersection
    nLeft  = std::max( nLeft, aTmpRect.nLeft );
    nRight = std::min( nRight, aTmpRect.nRight );
    nTop   = std::max( nTop, aTmpRect.nTop );
    nBottom= std::min( nBottom, aTmpRect.nBottom );

    // Determine if intersection is empty
    if ( nRight < nLeft || nBottom < nTop )
        *this = tools::Rectangle();

    return *this;
}

void tools::Rectangle::Justify()
{
    if ( (nRight < nLeft) && (nRight != RECT_EMPTY) )
    {
        std::swap(nLeft, nRight);
    }

    if ( (nBottom < nTop) && (nBottom != RECT_EMPTY) )
    {
        std::swap(nBottom, nTop);
    }
}

bool tools::Rectangle::IsInside( const Point& rPoint ) const
{
    if ( IsEmpty() )
        return false;

    if ( nLeft <= nRight )
    {
        if ( (rPoint.X() < nLeft) || (rPoint.X() > nRight) )
            return false;
    }
    else
    {
        if ( (rPoint.X() > nLeft) || (rPoint.X() < nRight) )
            return false;
    }
    if ( nTop <= nBottom )
    {
        if ( (rPoint.Y() < nTop) || (rPoint.Y() > nBottom) )
            return false;
    }
    else
    {
        if ( (rPoint.Y() > nTop) || (rPoint.Y() < nBottom) )
            return false;
    }
    return true;
}

bool tools::Rectangle::IsInside( const tools::Rectangle& rRect ) const
{
    return IsInside( rRect.TopLeft() ) && IsInside( rRect.BottomRight() );
}

bool tools::Rectangle::IsOver( const tools::Rectangle& rRect ) const
{
    // If there's no intersection, they don't overlap
    return !GetIntersection( rRect ).IsEmpty();
}

namespace tools
{
SvStream& ReadRectangle( SvStream& rIStream, tools::Rectangle& rRect )
{
    sal_Int32 nTmpL(0), nTmpT(0), nTmpR(0), nTmpB(0);

    rIStream.ReadInt32( nTmpL ).ReadInt32( nTmpT ).ReadInt32( nTmpR ).ReadInt32( nTmpB );

    rRect.nLeft = nTmpL;
    rRect.nTop = nTmpT;
    rRect.nRight = nTmpR;
    rRect.nBottom = nTmpB;

    return rIStream;
}

SvStream& WriteRectangle( SvStream& rOStream, const tools::Rectangle& rRect )
{
    rOStream.WriteInt32( rRect.nLeft )
            .WriteInt32( rRect.nTop )
            .WriteInt32( rRect.nRight )
            .WriteInt32( rRect.nBottom );

    return rOStream;
}
}

OString tools::Rectangle::toString() const
{
    std::stringstream ss;
    // Note that this is not just used for debugging output but the
    // format is parsed by external code (passed in callbacks to
    // LibreOfficeKit clients). So don't change.
    ss << getX() << ", " << getY() << ", " << getWidth() << ", " << getHeight();
    return ss.str().c_str();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
