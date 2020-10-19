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
#include <sal/log.hxx>

#include <algorithm>
#include <cassert>
#include <sstream>
#include <o3tl/safeint.hxx>
#include <tools/gen.hxx>
#include <tools/stream.hxx>

OString Pair::toString() const
{
    std::stringstream ss;
    // Note that this is not just used for debugging output but the
    // format is parsed by external code (passed in callbacks to
    // LibreOfficeKit clients). So don't change.
    ss << A() << ", " << B();
    return ss.str().c_str();
}

tools::Rectangle tools::Rectangle::Justify( const Point& rLT, const Point& rRB )
{
    tools::Long nLeft   = std::min(rLT.X(), rRB.X());
    tools::Long nTop    = std::min(rLT.Y(), rRB.Y());
    tools::Long nRight  = std::max(rLT.X(), rRB.X());
    tools::Long nBottom = std::max(rLT.Y(), rRB.Y());
    return Rectangle( nLeft, nTop, nRight, nBottom );
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

void tools::Rectangle::SaturatingSetX(tools::Long x)
{
    if (nRight != RECT_EMPTY)
        nRight = o3tl::saturating_add(nRight, x - nLeft);
    nLeft = x;
}

void tools::Rectangle::SaturatingSetY(tools::Long y)
{
    if (nBottom != RECT_EMPTY)
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

OString tools::Rectangle::toString() const
{
    std::stringstream ss;
    // Note that this is not just used for debugging output but the
    // format is parsed by external code (passed in callbacks to
    // LibreOfficeKit clients). So don't change.
    ss << getX() << ", " << getY() << ", " << getWidth() << ", " << getHeight();
    return ss.str().c_str();
}

void tools::Rectangle::expand(tools::Long nExpandBy)
{
    nLeft   -= nExpandBy;
    nTop    -= nExpandBy;
    if (nRight == RECT_EMPTY)
        nRight = nLeft + nExpandBy - 1;
    else
        nRight += nExpandBy;
    if (nBottom == RECT_EMPTY)
        nBottom = nTop + nExpandBy - 1;
    else
        nBottom += nExpandBy;
}

void tools::Rectangle::shrink(tools::Long nShrinkBy)
{
    nLeft   += nShrinkBy;
    nTop    += nShrinkBy;
    if (nRight != RECT_EMPTY)
        nRight -= nShrinkBy;
    if (nBottom != RECT_EMPTY)
        nBottom -= nShrinkBy;
}

tools::Long tools::Rectangle::AdjustRight(tools::Long nHorzMoveDelta)
{
    if (nRight == RECT_EMPTY)
        nRight = nLeft + nHorzMoveDelta - 1;
    else
        nRight += nHorzMoveDelta;
    return nRight;
}

tools::Long tools::Rectangle::AdjustBottom( tools::Long nVertMoveDelta )
{
    if (nBottom == RECT_EMPTY)
        nBottom = nTop + nVertMoveDelta - 1;
    else
        nBottom += nVertMoveDelta;
    return nBottom;
}

void tools::Rectangle::setX( tools::Long x )
{
    if (nRight != RECT_EMPTY)
        nRight += x - nLeft;
    nLeft = x;
}

void tools::Rectangle::setY( tools::Long y )
{
    if (nBottom != RECT_EMPTY)
        nBottom += y - nTop;
    nTop  = y;
}

tools::Long tools::Rectangle::Right() const
{
    return nRight == RECT_EMPTY ? nLeft : nRight;
}

tools::Long tools::Rectangle::Bottom() const
{
    return nBottom == RECT_EMPTY ? nTop : nBottom;
}

/// Returns the difference between right and left, assuming the range includes one end, but not the other.
tools::Long tools::Rectangle::getWidth() const
{
    return nRight == RECT_EMPTY ? 0 : nRight - nLeft;
}

/// Returns the difference between bottom and top, assuming the range includes one end, but not the other.
tools::Long tools::Rectangle::getHeight() const
{
    return nBottom == RECT_EMPTY ? 0 : nBottom - nTop;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
